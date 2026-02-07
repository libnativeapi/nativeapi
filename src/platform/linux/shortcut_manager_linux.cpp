#include <algorithm>
#include <atomic>
#include <cctype>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "../../shortcut_manager.h"

namespace nativeapi {
namespace {

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return value;
}

std::vector<std::string> SplitAccelerator(const std::string& accelerator) {
  std::vector<std::string> parts;
  std::string current;
  for (char ch : accelerator) {
    if (ch == '+') {
      if (!current.empty()) {
        parts.push_back(current);
        current.clear();
      }
    } else if (!std::isspace(static_cast<unsigned char>(ch))) {
      current.push_back(ch);
    }
  }
  if (!current.empty()) {
    parts.push_back(current);
  }
  return parts;
}

bool ParseAcceleratorTokens(const std::string& accelerator,
                            std::vector<std::string>& modifiers,
                            std::string& key_token) {
  modifiers.clear();
  key_token.clear();

  auto parts = SplitAccelerator(accelerator);
  if (parts.empty()) {
    return false;
  }

  for (auto& part : parts) {
    std::string token = ToLower(part);
    if (token == "ctrl" || token == "control" || token == "alt" || token == "shift" ||
        token == "cmd" || token == "command" || token == "super" || token == "meta" ||
        token == "cmdorctrl") {
      modifiers.push_back(token);
    } else {
      if (!key_token.empty()) {
        return false;
      }
      key_token = token;
    }
  }

  return !key_token.empty();
}

KeySym KeySymFromToken(const std::string& token) {
  if (token.size() == 1) {
    char ch = token[0];
    if (std::isalpha(static_cast<unsigned char>(ch))) {
      return XStringToKeysym(std::string(1, static_cast<char>(std::toupper(ch))).c_str());
    }
    if (std::isdigit(static_cast<unsigned char>(ch))) {
      return XStringToKeysym(std::string(1, ch).c_str());
    }
  }

  if (token.rfind("f", 0) == 0) {
    int fnum = std::stoi(token.substr(1));
    if (fnum >= 1 && fnum <= 24) {
      return XK_F1 + (fnum - 1);
    }
  }

  if (token == "space") return XK_space;
  if (token == "tab") return XK_Tab;
  if (token == "enter" || token == "return") return XK_Return;
  if (token == "escape" || token == "esc") return XK_Escape;
  if (token == "backspace") return XK_BackSpace;
  if (token == "delete") return XK_Delete;
  if (token == "insert") return XK_Insert;
  if (token == "home") return XK_Home;
  if (token == "end") return XK_End;
  if (token == "pageup") return XK_Page_Up;
  if (token == "pagedown") return XK_Page_Down;
  if (token == "up") return XK_Up;
  if (token == "down") return XK_Down;
  if (token == "left") return XK_Left;
  if (token == "right") return XK_Right;
  if (token == "plus") return XK_plus;
  if (token == "equal") return XK_equal;
  if (token == "minus") return XK_minus;

  return NoSymbol;
}

bool ParseAcceleratorLinux(const std::string& accelerator,
                           unsigned int& modifiers,
                           KeyCode& keycode,
                           Display* display) {
  modifiers = 0;
  keycode = 0;

  std::vector<std::string> modifier_tokens;
  std::string key_token;
  if (!ParseAcceleratorTokens(accelerator, modifier_tokens, key_token)) {
    return false;
  }

  for (const auto& token : modifier_tokens) {
    if (token == "ctrl" || token == "control" || token == "cmdorctrl") {
      modifiers |= ControlMask;
    } else if (token == "alt") {
      modifiers |= Mod1Mask;
    } else if (token == "shift") {
      modifiers |= ShiftMask;
    } else if (token == "cmd" || token == "command" || token == "super" || token == "meta") {
      modifiers |= Mod4Mask;
    }
  }

  KeySym keysym = KeySymFromToken(key_token);
  if (keysym == NoSymbol) {
    return false;
  }

  keycode = XKeysymToKeycode(display, keysym);
  return keycode != 0;
}

}  // namespace

class ShortcutManagerImpl final : public ShortcutManager::Impl {
 public:
  explicit ShortcutManagerImpl(ShortcutManager* manager) : manager_(manager) {
    XInitThreads();
    display_ = XOpenDisplay(nullptr);
    if (display_) {
      root_ = DefaultRootWindow(display_);
      exit_atom_ = XInternAtom(display_, "NATIVEAPI_SHORTCUT_EXIT", False);
    }
  }

  ~ShortcutManagerImpl() override {
    StopThread();
    if (display_) {
      XCloseDisplay(display_);
      display_ = nullptr;
    }
  }

  bool IsSupported() override { return display_ != nullptr; }

  bool RegisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    if (!display_) {
      return false;
    }

    unsigned int modifiers = 0;
    KeyCode keycode = 0;
    if (!ParseAcceleratorLinux(shortcut->GetAccelerator(), modifiers, keycode, display_)) {
      return false;
    }

    GrabKeyWithModifiers(keycode, modifiers);

    {
      std::lock_guard<std::mutex> lock(mutex_);
      GrabInfo info{keycode, modifiers};
      grabs_[shortcut->GetId()] = info;
      combo_to_shortcut_[ComposeKey(modifiers, keycode)] = shortcut->GetId();
    }

    EnsureThread();
    return true;
  }

  bool UnregisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    if (!display_) {
      return false;
    }

    GrabInfo info;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = grabs_.find(shortcut->GetId());
      if (it == grabs_.end()) {
        return false;
      }
      info = it->second;
      grabs_.erase(it);
      combo_to_shortcut_.erase(ComposeKey(info.modifiers, info.keycode));
    }

    UngrabKeyWithModifiers(info.keycode, info.modifiers);
    return true;
  }

  void SetupEventMonitoring() override { EnsureThread(); }

  void CleanupEventMonitoring() override {
    // Keep thread running while shortcuts may still be registered.
  }

 private:
  struct GrabInfo {
    KeyCode keycode;
    unsigned int modifiers;
  };

  void GrabKeyWithModifiers(KeyCode keycode, unsigned int modifiers) {
    const unsigned int extra_masks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    for (unsigned int mask : extra_masks) {
      XGrabKey(display_, keycode, modifiers | mask, root_, True, GrabModeAsync, GrabModeAsync);
    }
    XSync(display_, False);
  }

  void UngrabKeyWithModifiers(KeyCode keycode, unsigned int modifiers) {
    const unsigned int extra_masks[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    for (unsigned int mask : extra_masks) {
      XUngrabKey(display_, keycode, modifiers | mask, root_);
    }
    XSync(display_, False);
  }

  uint32_t ComposeKey(unsigned int modifiers, KeyCode keycode) const {
    return (static_cast<uint32_t>(modifiers & 0xFFFF) << 16) | (keycode & 0xFFFF);
  }

  void EnsureThread() {
    if (running_.load() || !display_) {
      return;
    }

    running_.store(true);
    event_thread_ = std::thread([this]() { ThreadMain(); });
  }

  void StopThread() {
    if (!running_.load()) {
      return;
    }

    running_.store(false);
    SendExitMessage();
    if (event_thread_.joinable()) {
      event_thread_.join();
    }
  }

  void SendExitMessage() {
    if (!display_) {
      return;
    }

    XClientMessageEvent client_message = {};
    client_message.type = ClientMessage;
    client_message.message_type = exit_atom_;
    client_message.window = root_;
    client_message.format = 32;
    XSendEvent(display_, root_, False, 0, reinterpret_cast<XEvent*>(&client_message));
    XFlush(display_);
  }

  void ThreadMain() {
    if (!display_) {
      return;
    }

    XSelectInput(display_, root_, KeyPressMask);

    while (running_.load()) {
      XEvent event;
      XNextEvent(display_, &event);

      if (!running_.load()) {
        break;
      }

      if (event.type == ClientMessage) {
        if (event.xclient.message_type == exit_atom_) {
          break;
        }
      }

      if (event.type != KeyPress) {
        continue;
      }

      unsigned int normalized_mods = event.xkey.state & ~(LockMask | Mod2Mask);
      uint32_t combo = ComposeKey(normalized_mods, event.xkey.keycode);

      ShortcutId shortcut_id = 0;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = combo_to_shortcut_.find(combo);
        if (it == combo_to_shortcut_.end()) {
          continue;
        }
        shortcut_id = it->second;
      }

      auto shortcut = manager_->Get(shortcut_id);
      if (!shortcut) {
        continue;
      }

      if (!manager_->IsEnabled() || !shortcut->IsEnabled()) {
        continue;
      }

      manager_->EmitShortcutActivated(shortcut_id, shortcut->GetAccelerator());
      shortcut->Invoke();
    }
  }

  ShortcutManager* manager_;
  Display* display_ = nullptr;
  Window root_ = 0;
  Atom exit_atom_ = None;

  std::mutex mutex_;
  std::unordered_map<ShortcutId, GrabInfo> grabs_;
  std::unordered_map<uint32_t, ShortcutId> combo_to_shortcut_;

  std::atomic<bool> running_{false};
  std::thread event_thread_;
};

ShortcutManager::ShortcutManager()
    : pimpl_(std::make_unique<ShortcutManagerImpl>(this)), next_shortcut_id_(1), enabled_(true) {}

ShortcutManager::~ShortcutManager() {
  UnregisterAll();
}

}  // namespace nativeapi
