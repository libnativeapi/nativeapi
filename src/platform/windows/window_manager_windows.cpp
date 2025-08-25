#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "../../window_manager.h"
#include "../../window.h"

#pragma comment(lib, "dwmapi.lib")

namespace nativeapi {

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {}

std::shared_ptr<Window> WindowManager::Get(WindowID id) {
    auto it = windows_.find(id);
    if (it != windows_.end()) {
        return it->second;
    }
    
    // Check if the window still exists
    HWND hwnd = reinterpret_cast<HWND>(id);
    if (IsWindow(hwnd)) {
        auto window = std::make_shared<Window>(hwnd);
        windows_[id] = window;
        return window;
    }
    
    return nullptr;
}

// Callback function for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<HWND>* windows = reinterpret_cast<std::vector<HWND>*>(lParam);
    
    // Only include visible windows that are not tool windows
    if (IsWindowVisible(hwnd)) {
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (!(exStyle & WS_EX_TOOLWINDOW)) {
            // Check if window has a title
            wchar_t title[256];
            if (GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t)) > 0) {
                windows->push_back(hwnd);
            }
        }
    }
    
    return TRUE;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
    std::vector<std::shared_ptr<Window>> result;
    std::vector<HWND> windowHandles;
    
    // Enumerate all top-level windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowHandles));
    
    for (HWND hwnd : windowHandles) {
        WindowID windowId = reinterpret_cast<WindowID>(hwnd);
        
        // Check if we already have this window
        auto it = windows_.find(windowId);
        if (it == windows_.end()) {
            auto window = std::make_shared<Window>(hwnd);
            windows_[windowId] = window;
        }
        
        if (auto window = windows_[windowId]) {
            result.push_back(window);
        }
    }
    
    return result;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        WindowID windowId = reinterpret_cast<WindowID>(hwnd);
        return Get(windowId);
    }
    return nullptr;
}

}  // namespace nativeapi