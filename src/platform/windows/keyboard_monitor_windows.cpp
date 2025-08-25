#include <windows.h>
#include <iostream>
#include <string>
#include <memory>
#include "../../keyboard_monitor.h"

namespace nativeapi {

class KeyboardMonitor::Impl {
 public:
  Impl(KeyboardMonitor* monitor) : monitor_(monitor), hook_(nullptr) {}

  HHOOK hook_;
  KeyboardMonitor* monitor_;
  static Impl* instance_;
};

KeyboardMonitor::Impl* KeyboardMonitor::Impl::instance_ = nullptr;

KeyboardMonitor::KeyboardMonitor() : impl_(std::make_unique<Impl>(this)), event_handler_(nullptr) {}

KeyboardMonitor::~KeyboardMonitor() {
  Stop();
}

// Low-level keyboard hook procedure
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode < 0) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  if (KeyboardMonitor::Impl::instance_ == nullptr) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  auto* monitor = KeyboardMonitor::Impl::instance_->monitor_;
  auto* eventHandler = monitor->GetEventHandler();
  if (!eventHandler) {
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
  }

  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT* pKbStruct = (KBDLLHOOKSTRUCT*)lParam;
    
    // Handle key events
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
      eventHandler->OnKeyPressed(pKbStruct->vkCode);
    } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
      eventHandler->OnKeyReleased(pKbStruct->vkCode);
    }

    // Check for modifier key changes
    uint32_t modifierKeys = static_cast<uint32_t>(ModifierKey::None);
    
    // Check current state of modifier keys
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
      modifierKeys |= static_cast<uint32_t>(ModifierKey::Shift);
    }
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
      modifierKeys |= static_cast<uint32_t>(ModifierKey::Ctrl);
    }
    if (GetAsyncKeyState(VK_MENU) & 0x8000) {  // Alt key
      modifierKeys |= static_cast<uint32_t>(ModifierKey::Alt);
    }
    if (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000) {
      modifierKeys |= static_cast<uint32_t>(ModifierKey::Meta);
    }
    if (GetAsyncKeyState(VK_CAPITAL) & 0x0001) {  // Caps Lock (toggled state)
      modifierKeys |= static_cast<uint32_t>(ModifierKey::CapsLock);
    }
    if (GetAsyncKeyState(VK_NUMLOCK) & 0x0001) {  // Num Lock (toggled state)
      modifierKeys |= static_cast<uint32_t>(ModifierKey::NumLock);
    }
    if (GetAsyncKeyState(VK_SCROLL) & 0x0001) {  // Scroll Lock (toggled state)
      modifierKeys |= static_cast<uint32_t>(ModifierKey::ScrollLock);
    }

    // Only call if modifier keys changed
    static uint32_t lastModifierKeys = 0;
    if (modifierKeys != lastModifierKeys) {
      eventHandler->OnModifierKeysChanged(modifierKeys);
      lastModifierKeys = modifierKeys;
    }
  }

  return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void KeyboardMonitor::Start() {
  if (impl_->hook_ != nullptr) {
    return;  // Already started
  }

  // Set the global instance pointer
  KeyboardMonitor::Impl::instance_ = impl_.get();

  // Install low-level keyboard hook
  impl_->hook_ = SetWindowsHookEx(
      WH_KEYBOARD_LL,
      LowLevelKeyboardProc,
      GetModuleHandle(nullptr),
      0);

  if (impl_->hook_ == nullptr) {
    std::cerr << "Failed to install keyboard hook. Error: " << GetLastError() << std::endl;
    KeyboardMonitor::Impl::instance_ = nullptr;
    return;
  }

  std::cout << "Keyboard monitor started successfully" << std::endl;
}

void KeyboardMonitor::Stop() {
  if (impl_->hook_ == nullptr) {
    return;  // Already stopped
  }

  // Uninstall the hook
  if (UnhookWindowsHookEx(impl_->hook_)) {
    std::cout << "Keyboard monitor stopped successfully" << std::endl;
  } else {
    std::cerr << "Failed to uninstall keyboard hook. Error: " << GetLastError() << std::endl;
  }

  impl_->hook_ = nullptr;
  KeyboardMonitor::Impl::instance_ = nullptr;
}

bool KeyboardMonitor::IsMonitoring() const {
  return impl_->hook_ != nullptr;
}

}  // namespace nativeapi