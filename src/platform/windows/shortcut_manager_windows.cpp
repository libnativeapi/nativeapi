#include <algorithm>
#include <atomic>
#include <cctype>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <windows.h>

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

bool ParseAcceleratorWindows(const std::string& accelerator, UINT& modifiers, UINT& vk) {
  modifiers = 0;
  vk = 0;

  std::vector<std::string> modifier_tokens;
  std::string key_token;
  if (!ParseAcceleratorTokens(accelerator, modifier_tokens, key_token)) {
    return false;
  }

  for (const auto& token : modifier_tokens) {
    if (token == "ctrl" || token == "control" || token == "cmdorctrl") {
      modifiers |= MOD_CONTROL;
    } else if (token == "alt") {
      modifiers |= MOD_ALT;
    } else if (token == "shift") {
      modifiers |= MOD_SHIFT;
    } else if (token == "cmd" || token == "command" || token == "super" ||
               token == "meta") {
      modifiers |= MOD_WIN;
    }
  }

  modifiers |= MOD_NOREPEAT;

  if (key_token.size() == 1) {
    char ch = key_token[0];
    if (std::isalpha(static_cast<unsigned char>(ch))) {
      vk = static_cast<UINT>(std::toupper(ch));
      return true;
    }
    if (std::isdigit(static_cast<unsigned char>(ch))) {
      vk = static_cast<UINT>(ch);
      return true;
    }
  }

  if (key_token.rfind("f", 0) == 0) {
    int fnum = std::stoi(key_token.substr(1));
    if (fnum >= 1 && fnum <= 24) {
      vk = VK_F1 + (fnum - 1);
      return true;
    }
  }

  if (key_token == "space") {
    vk = VK_SPACE;
  } else if (key_token == "tab") {
    vk = VK_TAB;
  } else if (key_token == "enter" || key_token == "return") {
    vk = VK_RETURN;
  } else if (key_token == "escape" || key_token == "esc") {
    vk = VK_ESCAPE;
  } else if (key_token == "backspace") {
    vk = VK_BACK;
  } else if (key_token == "delete") {
    vk = VK_DELETE;
  } else if (key_token == "insert") {
    vk = VK_INSERT;
  } else if (key_token == "home") {
    vk = VK_HOME;
  } else if (key_token == "end") {
    vk = VK_END;
  } else if (key_token == "pageup") {
    vk = VK_PRIOR;
  } else if (key_token == "pagedown") {
    vk = VK_NEXT;
  } else if (key_token == "up") {
    vk = VK_UP;
  } else if (key_token == "down") {
    vk = VK_DOWN;
  } else if (key_token == "left") {
    vk = VK_LEFT;
  } else if (key_token == "right") {
    vk = VK_RIGHT;
  } else if (key_token == "plus" || key_token == "equal") {
    vk = VK_OEM_PLUS;
  } else if (key_token == "minus") {
    vk = VK_OEM_MINUS;
  } else {
    return false;
  }

  return true;
}

}  // namespace

class ShortcutManagerImpl final : public ShortcutManager::Impl {
 public:
  explicit ShortcutManagerImpl(ShortcutManager* manager) : manager_(manager), running_(false) {}

  ~ShortcutManagerImpl() override { StopThread(); }

  bool IsSupported() override { return true; }

  bool RegisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    EnsureThread();

    UINT modifiers = 0;
    UINT vk = 0;
    if (!ParseAcceleratorWindows(shortcut->GetAccelerator(), modifiers, vk)) {
      return false;
    }

    int hotkey_id = 0;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      hotkey_id = next_hotkey_id_++;
    }
    if (!RegisterHotKey(hwnd_, hotkey_id, modifiers, vk)) {
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      shortcut_to_hotkey_[shortcut->GetId()] = hotkey_id;
      hotkey_to_shortcut_[hotkey_id] = shortcut->GetId();
    }

    return true;
  }

  bool UnregisterShortcut(const std::shared_ptr<Shortcut>& shortcut) override {
    int hotkey_id = 0;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = shortcut_to_hotkey_.find(shortcut->GetId());
      if (it == shortcut_to_hotkey_.end()) {
        return false;
      }
      hotkey_id = it->second;
      shortcut_to_hotkey_.erase(it);
      hotkey_to_shortcut_.erase(hotkey_id);
    }

    UnregisterHotKey(hwnd_, hotkey_id);
    return true;
  }

  void SetupEventMonitoring() override { EnsureThread(); }

  void CleanupEventMonitoring() override {
    // Keep thread alive while shortcuts might still be registered.
  }

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == WM_NCCREATE) {
      auto* create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
      SetWindowLongPtr(hwnd, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    auto* self =
        reinterpret_cast<ShortcutManagerImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!self) {
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    switch (msg) {
      case WM_HOTKEY:
        self->HandleHotKey(static_cast<int>(wparam));
        return 0;
      case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
      default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
  }

  void HandleHotKey(int hotkey_id) {
    ShortcutId shortcut_id = 0;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = hotkey_to_shortcut_.find(hotkey_id);
      if (it == hotkey_to_shortcut_.end()) {
        return;
      }
      shortcut_id = it->second;
    }

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

  void EnsureThread() {
    if (running_.load()) {
      std::unique_lock<std::mutex> lock(thread_mutex_);
      thread_cv_.wait(lock, [this]() { return hwnd_ready_; });
      return;
    }

    running_.store(true);
    thread_ = std::thread([this]() { ThreadMain(); });

    std::unique_lock<std::mutex> lock(thread_mutex_);
    thread_cv_.wait(lock, [this]() { return hwnd_ready_; });
  }

  void ThreadMain() {
    const wchar_t* class_name = L"NativeApiShortcutManager";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = class_name;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, class_name, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                                wc.hInstance, this);

    {
      std::lock_guard<std::mutex> lock(thread_mutex_);
      hwnd_ = hwnd;
      hwnd_ready_ = true;
    }
    thread_cv_.notify_all();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  void StopThread() {
    if (!running_.load()) {
      return;
    }

    running_.store(false);
    if (hwnd_) {
      PostMessage(hwnd_, WM_CLOSE, 0, 0);
    }
    if (thread_.joinable()) {
      thread_.join();
    }
    hwnd_ = nullptr;
    hwnd_ready_ = false;
  }

  ShortcutManager* manager_;
  std::mutex mutex_;
  std::unordered_map<ShortcutId, int> shortcut_to_hotkey_;
  std::unordered_map<int, ShortcutId> hotkey_to_shortcut_;
  int next_hotkey_id_ = 1;

  std::atomic<bool> running_;
  std::thread thread_;
  std::mutex thread_mutex_;
  std::condition_variable thread_cv_;
  HWND hwnd_ = nullptr;
  bool hwnd_ready_ = false;
};

ShortcutManager::ShortcutManager()
    : pimpl_(std::make_unique<ShortcutManagerImpl>(this)), next_shortcut_id_(1), enabled_(true) {}

ShortcutManager::~ShortcutManager() {
  UnregisterAll();
}

}  // namespace nativeapi
