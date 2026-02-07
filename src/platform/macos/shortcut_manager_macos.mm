#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

#import <Carbon/Carbon.h>

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

bool ParseAcceleratorMac(const std::string& accelerator, UInt32& modifiers, UInt32& keycode) {
  modifiers = 0;
  keycode = 0;

  std::vector<std::string> modifier_tokens;
  std::string key_token;
  if (!ParseAcceleratorTokens(accelerator, modifier_tokens, key_token)) {
    return false;
  }

  for (const auto& token : modifier_tokens) {
    if (token == "ctrl" || token == "control") {
      modifiers |= controlKey;
    } else if (token == "alt") {
      modifiers |= optionKey;
    } else if (token == "shift") {
      modifiers |= shiftKey;
    } else if (token == "cmd" || token == "command" || token == "meta") {
      modifiers |= cmdKey;
    } else if (token == "super") {
      modifiers |= cmdKey;
    } else if (token == "cmdorctrl") {
      modifiers |= cmdKey;
    }
  }

  if (key_token.size() == 1) {
    char ch = key_token[0];
    if (std::isalpha(static_cast<unsigned char>(ch))) {
      switch (std::toupper(ch)) {
        case 'A': keycode = kVK_ANSI_A; return true;
        case 'B': keycode = kVK_ANSI_B; return true;
        case 'C': keycode = kVK_ANSI_C; return true;
        case 'D': keycode = kVK_ANSI_D; return true;
        case 'E': keycode = kVK_ANSI_E; return true;
        case 'F': keycode = kVK_ANSI_F; return true;
        case 'G': keycode = kVK_ANSI_G; return true;
        case 'H': keycode = kVK_ANSI_H; return true;
        case 'I': keycode = kVK_ANSI_I; return true;
        case 'J': keycode = kVK_ANSI_J; return true;
        case 'K': keycode = kVK_ANSI_K; return true;
        case 'L': keycode = kVK_ANSI_L; return true;
        case 'M': keycode = kVK_ANSI_M; return true;
        case 'N': keycode = kVK_ANSI_N; return true;
        case 'O': keycode = kVK_ANSI_O; return true;
        case 'P': keycode = kVK_ANSI_P; return true;
        case 'Q': keycode = kVK_ANSI_Q; return true;
        case 'R': keycode = kVK_ANSI_R; return true;
        case 'S': keycode = kVK_ANSI_S; return true;
        case 'T': keycode = kVK_ANSI_T; return true;
        case 'U': keycode = kVK_ANSI_U; return true;
        case 'V': keycode = kVK_ANSI_V; return true;
        case 'W': keycode = kVK_ANSI_W; return true;
        case 'X': keycode = kVK_ANSI_X; return true;
        case 'Y': keycode = kVK_ANSI_Y; return true;
        case 'Z': keycode = kVK_ANSI_Z; return true;
      }
    }
    if (std::isdigit(static_cast<unsigned char>(ch))) {
      switch (ch) {
        case '0': keycode = kVK_ANSI_0; return true;
        case '1': keycode = kVK_ANSI_1; return true;
        case '2': keycode = kVK_ANSI_2; return true;
        case '3': keycode = kVK_ANSI_3; return true;
        case '4': keycode = kVK_ANSI_4; return true;
        case '5': keycode = kVK_ANSI_5; return true;
        case '6': keycode = kVK_ANSI_6; return true;
        case '7': keycode = kVK_ANSI_7; return true;
        case '8': keycode = kVK_ANSI_8; return true;
        case '9': keycode = kVK_ANSI_9; return true;
      }
    }
  }

  if (key_token.rfind("f", 0) == 0) {
    int fnum = std::stoi(key_token.substr(1));
    if (fnum >= 1 && fnum <= 24) {
      keycode = kVK_F1 + (fnum - 1);
      return true;
    }
  }

  if (key_token == "space") {
    keycode = kVK_Space;
  } else if (key_token == "tab") {
    keycode = kVK_Tab;
  } else if (key_token == "enter" || key_token == "return") {
    keycode = kVK_Return;
  } else if (key_token == "escape" || key_token == "esc") {
    keycode = kVK_Escape;
  } else if (key_token == "backspace") {
    keycode = kVK_Delete;
  } else if (key_token == "delete") {
    keycode = kVK_ForwardDelete;
  } else if (key_token == "insert") {
    keycode = kVK_Help;
  } else if (key_token == "home") {
    keycode = kVK_Home;
  } else if (key_token == "end") {
    keycode = kVK_End;
  } else if (key_token == "pageup") {
    keycode = kVK_PageUp;
  } else if (key_token == "pagedown") {
    keycode = kVK_PageDown;
  } else if (key_token == "up") {
    keycode = kVK_UpArrow;
  } else if (key_token == "down") {
    keycode = kVK_DownArrow;
  } else if (key_token == "left") {
    keycode = kVK_LeftArrow;
  } else if (key_token == "right") {
    keycode = kVK_RightArrow;
  } else if (key_token == "plus" || key_token == "equal") {
    keycode = kVK_ANSI_Equal;
  } else if (key_token == "minus") {
    keycode = kVK_ANSI_Minus;
  } else {
    return false;
  }

  return true;
}

}  // namespace

class ShortcutManagerImpl final : public ShortcutManager::Impl {
 public:
  explicit ShortcutManagerImpl(ShortcutManager* manager) : manager_(manager) {}

  ~ShortcutManagerImpl() override {
    for (const auto& [id, hotkey] : hotkeys_) {
      UnregisterEventHotKey(hotkey);
    }
    hotkeys_.clear();

    if (handler_) {
      RemoveEventHandler(handler_);
      handler_ = nullptr;
    }
  }

  bool IsSupported() override { return true; }

  bool RegisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    EnsureHandler();

    UInt32 modifiers = 0;
    UInt32 keycode = 0;
    if (!ParseAcceleratorMac(shortcut->GetAccelerator(), modifiers, keycode)) {
      return false;
    }

    EventHotKeyID hotkey_id;
    hotkey_id.signature = 'ntap';
    hotkey_id.id = static_cast<UInt32>(shortcut->GetId());

    EventHotKeyRef hotkey_ref = nullptr;
    OSStatus status = RegisterEventHotKey(keycode, modifiers, hotkey_id,
                                          GetApplicationEventTarget(), 0, &hotkey_ref);
    if (status != noErr || !hotkey_ref) {
      return false;
    }

    hotkeys_[shortcut->GetId()] = hotkey_ref;
    return true;
  }

  bool UnregisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    auto it = hotkeys_.find(shortcut->GetId());
    if (it == hotkeys_.end()) {
      return false;
    }

    UnregisterEventHotKey(it->second);
    hotkeys_.erase(it);
    return true;
  }

  void SetupEventMonitoring() override { EnsureHandler(); }

  void CleanupEventMonitoring() override {
    // Keep handler installed as long as shortcuts exist.
  }

 private:
  static OSStatus HotKeyHandler(EventHandlerCallRef next_handler, EventRef event, void* user_data) {
    auto* self = static_cast<ShortcutManagerImpl*>(user_data);
    if (!self) {
      return noErr;
    }

    EventHotKeyID hotkey_id;
    OSStatus status = GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, nullptr,
                                        sizeof(EventHotKeyID), nullptr, &hotkey_id);
    if (status != noErr) {
      return noErr;
    }

    self->HandleHotKey(static_cast<ShortcutId>(hotkey_id.id));
    return noErr;
  }

  void EnsureHandler() {
    if (handler_) {
      return;
    }

    EventTypeSpec event_type;
    event_type.eventClass = kEventClassKeyboard;
    event_type.eventKind = kEventHotKeyPressed;

    InstallEventHandler(GetApplicationEventTarget(), HotKeyHandler, 1, &event_type, this,
                        &handler_);
  }

  void HandleHotKey(ShortcutId shortcut_id) {
    auto shortcut = manager_->Get(shortcut_id);
    if (!shortcut) {
      return;
    }

    if (!manager_->IsEnabled() || !shortcut->IsEnabled()) {
      return;
    }

    manager_->EmitShortcutActivated(shortcut_id, shortcut->GetAccelerator());
    shortcut->Invoke();
  }

  ShortcutManager* manager_;
  std::unordered_map<ShortcutId, EventHotKeyRef> hotkeys_;
  EventHandlerRef handler_ = nullptr;
};

ShortcutManager::ShortcutManager()
    : pimpl_(std::make_unique<ShortcutManagerImpl>(this)), next_shortcut_id_(1), enabled_(true) {}

ShortcutManager::~ShortcutManager() {
  UnregisterAll();
}

}  // namespace nativeapi
