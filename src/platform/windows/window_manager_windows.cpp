#include <windows.h>
#include <iostream>
#include <string>

#include <psapi.h>
#include "../../window.h"
#include "../../window_event.h"
#include "../../window_manager.h"
#include "../../window_registry.h"
#include "string_utils_windows.h"

#pragma comment(lib, "psapi.lib")

namespace nativeapi {

namespace {

using PFN_ShowWindow = BOOL(WINAPI*)(HWND, int);
using PFN_ShowWindowAsync = BOOL(WINAPI*)(HWND, int);

static PFN_ShowWindow g_original_show_window = nullptr;
static PFN_ShowWindowAsync g_original_show_window_async = nullptr;
static bool g_hooks_installed = false;

static bool IsShowCommandVisible(int cmd) {
  switch (cmd) {
    case SW_SHOW:
    case SW_SHOWNORMAL:
    case SW_SHOWDEFAULT:
    case SW_SHOWMAXIMIZED:
    case SW_SHOWNOACTIVATE:
    case SW_RESTORE:
      return true;
    default:
      return false;
  }
}

static void InvokePreShowHideHooks(HWND hwnd, int cmd) {
  auto& manager = WindowManager::GetInstance();
  Window temp_window(hwnd);
  WindowId window_id = temp_window.GetId();
  if (cmd == SW_HIDE) {
    manager.InvokeWillHideHook(window_id);
  } else if (IsShowCommandVisible(cmd)) {
    manager.InvokeWillShowHook(window_id);
  }
}

static BOOL WINAPI HookedShowWindow(HWND hwnd, int nCmdShow) {
  InvokePreShowHideHooks(hwnd, nCmdShow);
  if (g_original_show_window) {
    return g_original_show_window(hwnd, nCmdShow);
  }
  // Fallback to direct call if original not captured
  auto p = reinterpret_cast<PFN_ShowWindow>(
      GetProcAddress(GetModuleHandleW(L"user32.dll"), "ShowWindow"));
  return p ? p(hwnd, nCmdShow) : FALSE;
}

static BOOL WINAPI HookedShowWindowAsync(HWND hwnd, int nCmdShow) {
  InvokePreShowHideHooks(hwnd, nCmdShow);
  if (g_original_show_window_async) {
    return g_original_show_window_async(hwnd, nCmdShow);
  }
  auto p = reinterpret_cast<PFN_ShowWindowAsync>(
      GetProcAddress(GetModuleHandleW(L"user32.dll"), "ShowWindowAsync"));
  return p ? p(hwnd, nCmdShow) : FALSE;
}

static bool CaseInsensitiveEquals(const char* a, const char* b) {
  if (!a || !b)
    return false;
  while (*a && *b) {
    char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
    char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
    if (ca != cb)
      return false;
    ++a;
    ++b;
  }
  return *a == *b;
}

static void PatchIATInModule(HMODULE module,
                             FARPROC target,
                             FARPROC replacement,
                             const char* func_name) {
  if (!module)
    return;

  auto base = reinterpret_cast<BYTE*>(module);
  auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
  if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE)
    return;

  auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
  if (!nt || nt->Signature != IMAGE_NT_SIGNATURE)
    return;

  auto& import_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (import_dir.VirtualAddress == 0)
    return;

  auto import_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(base + import_dir.VirtualAddress);
  for (; import_desc->Name != 0; ++import_desc) {
    auto dll_name = reinterpret_cast<const char*>(base + import_desc->Name);
    // Only hook USER32.dll to reduce risk
    if (!dll_name)
      continue;
    if (!(CaseInsensitiveEquals(dll_name, "user32.dll")))
      continue;

    auto orig_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(base + import_desc->OriginalFirstThunk);
    auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(base + import_desc->FirstThunk);
    if (!orig_thunk || !thunk)
      continue;

    for (; orig_thunk->u1.AddressOfData != 0; ++orig_thunk, ++thunk) {
      if (IMAGE_SNAP_BY_ORDINAL(orig_thunk->u1.Ordinal)) {
        continue;  // Skip ordinals
      }
      auto import = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(base + orig_thunk->u1.AddressOfData);
      if (!import || !import->Name)
        continue;
      const char* name = reinterpret_cast<const char*>(import->Name);
      if (!CaseInsensitiveEquals(name, func_name))
        continue;

      // Change protection and write new function pointer
      DWORD old_protect;
      if (VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old_protect)) {
        // Store original (first time only)
        (void)target;  // target kept for symmetry; not used here
        thunk->u1.Function = reinterpret_cast<ULONG_PTR>(replacement);
        VirtualProtect(&thunk->u1.Function, sizeof(void*), old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, sizeof(void*));
      }
    }
  }
}

static void RestoreIATInModule(HMODULE module, FARPROC original, const char* func_name) {
  if (!module || !original)
    return;

  auto base = reinterpret_cast<BYTE*>(module);
  auto dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
  if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE)
    return;

  auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
  if (!nt || nt->Signature != IMAGE_NT_SIGNATURE)
    return;

  auto& import_dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (import_dir.VirtualAddress == 0)
    return;

  auto import_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(base + import_dir.VirtualAddress);
  for (; import_desc->Name != 0; ++import_desc) {
    auto dll_name = reinterpret_cast<const char*>(base + import_desc->Name);
    if (!dll_name)
      continue;
    if (!(CaseInsensitiveEquals(dll_name, "user32.dll")))
      continue;

    auto orig_thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(base + import_desc->OriginalFirstThunk);
    auto thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(base + import_desc->FirstThunk);
    if (!orig_thunk || !thunk)
      continue;

    for (; orig_thunk->u1.AddressOfData != 0; ++orig_thunk, ++thunk) {
      if (IMAGE_SNAP_BY_ORDINAL(orig_thunk->u1.Ordinal)) {
        continue;
      }
      auto import = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(base + orig_thunk->u1.AddressOfData);
      if (!import || !import->Name)
        continue;
      const char* name = reinterpret_cast<const char*>(import->Name);
      if (!CaseInsensitiveEquals(name, func_name))
        continue;

      DWORD old_protect;
      if (VirtualProtect(&thunk->u1.Function, sizeof(void*), PAGE_READWRITE, &old_protect)) {
        thunk->u1.Function = reinterpret_cast<ULONG_PTR>(original);
        VirtualProtect(&thunk->u1.Function, sizeof(void*), old_protect, &old_protect);
        FlushInstructionCache(GetCurrentProcess(), &thunk->u1.Function, sizeof(void*));
      }
    }
  }
}

static void ForEachProcessModule(std::function<void(HMODULE)> fn) {
  HMODULE modules[1024];
  DWORD bytes_needed = 0;
  if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &bytes_needed)) {
    // Fallback: at least patch main module
    fn(GetModuleHandle(nullptr));
    return;
  }
  size_t count = bytes_needed / sizeof(HMODULE);
  for (size_t i = 0; i < count; ++i) {
    fn(modules[i]);
  }
}

static void InstallGlobalShowHooks() {
  if (g_hooks_installed)
    return;
  HMODULE user32 = GetModuleHandleW(L"user32.dll");
  if (!user32)
    user32 = LoadLibraryW(L"user32.dll");
  if (!user32)
    return;

  g_original_show_window = reinterpret_cast<PFN_ShowWindow>(GetProcAddress(user32, "ShowWindow"));
  g_original_show_window_async =
      reinterpret_cast<PFN_ShowWindowAsync>(GetProcAddress(user32, "ShowWindowAsync"));
  if (!g_original_show_window)
    return;

  ForEachProcessModule([](HMODULE m) {
    PatchIATInModule(m, reinterpret_cast<FARPROC>(g_original_show_window),
                     reinterpret_cast<FARPROC>(HookedShowWindow), "ShowWindow");
    if (g_original_show_window_async) {
      PatchIATInModule(m, reinterpret_cast<FARPROC>(g_original_show_window_async),
                       reinterpret_cast<FARPROC>(HookedShowWindowAsync), "ShowWindowAsync");
    }
  });

  g_hooks_installed = true;
}

static void UninstallGlobalShowHooks() {
  if (!g_hooks_installed)
    return;
  ForEachProcessModule([](HMODULE m) {
    if (g_original_show_window) {
      RestoreIATInModule(m, reinterpret_cast<FARPROC>(g_original_show_window), "ShowWindow");
    }
    if (g_original_show_window_async) {
      RestoreIATInModule(m, reinterpret_cast<FARPROC>(g_original_show_window_async),
                         "ShowWindowAsync");
    }
  });
  g_hooks_installed = false;
}

}  // namespace

// Custom window procedure to handle window messages
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_WINDOWPOSCHANGING: {
      // Intercept visibility changes BEFORE they happen (pre-show/hide "swizzle")
      // This is the closest Windows analogue to method swizzling NSWindow show/hide
      WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
      if (pos) {
        auto& manager = WindowManager::GetInstance();
        Window temp_window(hwnd);
        WindowId window_id = temp_window.GetId();
        if (pos->flags & SWP_SHOWWINDOW) {
          manager.InvokeWillShowHook(window_id);
        }
        if (pos->flags & SWP_HIDEWINDOW) {
          manager.InvokeWillHideHook(window_id);
        }
      }
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    case WM_SHOWWINDOW:
      // Keep default processing; pre-hooks are handled in WM_WINDOWPOSCHANGING
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    case WM_CLOSE:
      // User clicked the close button
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      // Window is being destroyed
      // For now, we'll exit the application when any window is destroyed
      // In a more sophisticated implementation, we might want to check
      // if this is the last window before calling PostQuitMessage
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

// Private implementation to hide Windows-specific details
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void StartEventListening() {
    // Windows event monitoring would typically be done through:
    // - SetWinEventHook for system-wide window events
    // - Window subclassing for specific window events
    // This is a placeholder implementation
  }

  void StopEventListening() {
    // Clean up any event hooks or monitoring
  }

  void OnWindowEvent(HWND hwnd, const std::string& event_type) {
    // Create a temporary Window object to get the proper WindowId
    Window temp_window(hwnd);
    WindowId window_id = temp_window.GetId();

    if (event_type == "focused") {
      WindowFocusedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "blurred") {
      WindowBlurredEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "minimized") {
      WindowMinimizedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "restored") {
      WindowRestoredEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "resized") {
      RECT rect;
      GetWindowRect(hwnd, &rect);
      Size new_size = {static_cast<double>(rect.right - rect.left),
                       static_cast<double>(rect.bottom - rect.top)};
      WindowResizedEvent event(window_id, new_size);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "moved") {
      RECT rect;
      GetWindowRect(hwnd, &rect);
      Point new_position = {static_cast<double>(rect.left), static_cast<double>(rect.top)};
      WindowMovedEvent event(window_id, new_position);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "closing") {
      WindowClosedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    }
  }

 private:
  WindowManager* manager_;
  // Optional pre-show/hide hooks
  std::optional<WindowManager::WindowWillShowHook> will_show_hook_;
  std::optional<WindowManager::WindowWillHideHook> will_hide_hook_;

  friend class WindowManager;
};

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  StartEventListening();
}

WindowManager::~WindowManager() {
  StopEventListening();
}

// Create a new window with the given options
std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);

  // Register window class if not already registered
  static bool class_registered = false;
  static std::wstring wclass_name = StringToWString("NativeAPIWindow");

  if (!class_registered) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = wclass_name.c_str();
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (RegisterClassW(&wc)) {
      class_registered = true;
    } else {
      DWORD error = GetLastError();
      if (error != ERROR_CLASS_ALREADY_EXISTS) {
        std::cerr << "Failed to register window class. Error: " << error << std::endl;
        return nullptr;
      }
      class_registered = true;
    }
  }

  // Create the window
  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD exStyle = 0;
  std::wstring wtitle = StringToWString(options.title);

  HWND hwnd =
      CreateWindowExW(exStyle, wclass_name.c_str(), wtitle.c_str(), style, CW_USEDEFAULT,
                      CW_USEDEFAULT, static_cast<int>(options.size.width),
                      static_cast<int>(options.size.height), nullptr, nullptr, hInstance, nullptr);

  if (!hwnd) {
    std::cerr << "Failed to create window. Error: " << GetLastError() << std::endl;
    return nullptr;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  auto window = std::make_shared<Window>(hwnd);
  WindowId window_id = window->GetId();
  WindowRegistry::GetInstance().Add(window_id, window);

  // Dispatch window created event
  WindowCreatedEvent created_event(window_id);
  DispatchWindowEvent(created_event);

  return window;
}

// Destroy a window by its ID. Returns true if window was destroyed.
bool WindowManager::Destroy(WindowId id) {
  auto window = WindowRegistry::GetInstance().Get(id);
  if (!window) {
    return false;
  }
  HWND hwnd = static_cast<HWND>(window->GetNativeObject());
  if (IsWindow(hwnd)) {
    DestroyWindow(hwnd);
  }
  WindowRegistry::GetInstance().Remove(id);
  return true;
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  auto cached = WindowRegistry::GetInstance().Get(id);
  if (cached) {
    return cached;
  }
  // Check if the window still exists in the system
  HWND hwnd = reinterpret_cast<HWND>(id);
  if (IsWindow(hwnd)) {
    auto window = std::make_shared<Window>(hwnd);
    WindowRegistry::GetInstance().Add(id, window);
    return window;
  }
  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  return WindowRegistry::GetInstance().GetAll();
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  HWND hwnd = GetForegroundWindow();
  if (hwnd) {
    Window temp_window(hwnd);
    WindowId window_id = temp_window.GetId();
    return Get(window_id);
  }
  return nullptr;
}

void WindowManager::SetWillShowHook(std::optional<WindowWillShowHook> hook) {
  pimpl_->will_show_hook_ = std::move(hook);
  // Install or uninstall global hooks based on whether any hook is present
  bool has_any = pimpl_->will_show_hook_.has_value() || pimpl_->will_hide_hook_.has_value();
  if (has_any) {
    InstallGlobalShowHooks();
  } else {
    UninstallGlobalShowHooks();
  }
}

void WindowManager::SetWillHideHook(std::optional<WindowWillHideHook> hook) {
  pimpl_->will_hide_hook_ = std::move(hook);
  bool has_any = pimpl_->will_show_hook_.has_value() || pimpl_->will_hide_hook_.has_value();
  if (has_any) {
    InstallGlobalShowHooks();
  } else {
    UninstallGlobalShowHooks();
  }
}

void WindowManager::InvokeWillShowHook(WindowId id) {
  if (pimpl_->will_show_hook_) {
    (*pimpl_->will_show_hook_)(id);
  }
}

void WindowManager::InvokeWillHideHook(WindowId id) {
  if (pimpl_->will_hide_hook_) {
    (*pimpl_->will_hide_hook_)(id);
  }
}

void WindowManager::StartEventListening() {
  pimpl_->StartEventListening();
}

void WindowManager::StopEventListening() {
  pimpl_->StopEventListening();
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);
}

}  // namespace nativeapi
