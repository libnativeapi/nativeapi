# Android Platform Implementation

This directory contains Android-specific implementations of the nativeapi library.

## Overview

The Android implementation uses the Android NDK to provide access to native system APIs. Due to Android's unique application model, some desktop concepts are mapped differently:

- **Windows** → Activities/Views (`ANativeWindow`)
- **Menus** → Context menus or ActionBar menus
- **Tray Icons** → System notifications
- **Display Management** → Display Manager API
- **Keyboard Monitoring** → Accessibility Services

## Files

- `window_android.cpp` - Window management using `ANativeWindow`
- `window_manager_android.cpp` - Window manager for Activity lifecycle
- `display_android.cpp` - Display information (stub)
- `display_manager_android.cpp` - Display manager (stub)
- `menu_android.cpp` - Menu implementation (stub)
- `tray_icon_android.cpp` - Tray icon using notifications (stub)
- `tray_manager_android.cpp` - Tray manager (stub)
- `keyboard_monitor_android.cpp` - Keyboard monitoring via AccessibilityService (stub)
- `accessibility_manager_android.cpp` - Accessibility manager (stub)
- `application_android.cpp` - Application lifecycle (stub)
- `image_android.cpp` - Image loading (stub)

## Building

These files are automatically included when building with the Android NDK:

```bash
cmake -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_ANDROID_NDK=/path/to/android-ndk \
      ..
```

## Android NDK Requirements

- Android NDK r21 or later
- Minimum Android SDK: API level 24 (Android 7.0)
- Recommended: API level 29+ (Android 10.0+)

## Key Differences from Desktop

### Window Management

Android uses `ANativeWindow` instead of platform-specific window handles:

```cpp
// Desktop: HWND (Windows), NSWindow* (macOS), GdkWindow* (Linux)
// Android: ANativeWindow*

void* Window::GetNativeObjectInternal() const {
    return static_cast<void*>(pimpl_->native_window_);
}
```

### Activity Lifecycle

Windows in Android are created and managed through the Activity lifecycle. Window creation happens automatically when the Activity creates a native window surface.

### Logging

Android uses the logging API instead of standard output:

```cpp
#include <android/log.h>

#define LOG_TAG "NativeApi"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
```

## Implementation Status

| Component | Status | Notes |
|-----------|--------|-------|
| Window | ✅ Implemented | Uses ANativeWindow API |
| Window Manager | ✅ Implemented | Activity lifecycle integration |
| Display | ⚠️ Stub | Basic implementation |
| Display Manager | ⚠️ Stub | Basic implementation |
| Menu | ⚠️ Stub | Needs Context menu integration |
| Tray Icon | ⚠️ Stub | Needs Notification API |
| Tray Manager | ⚠️ Stub | Needs Notification API |
| Keyboard Monitor | ⚠️ Stub | Needs AccessibilityService |
| Accessibility Manager | ⚠️ Stub | Needs Android permissions |
| Application | ⚠️ Stub | Basic lifecycle |
| Image | ⚠️ Stub | Needs Bitmap loading |

## See Also

- [Android NDK Documentation](https://developer.android.com/ndk)
- [ANativeWindow API](https://developer.android.com/ndk/reference/group/native-activity#anativewindow)
- [docs/ANDROID.md](../../../docs/ANDROID.md) - User-facing Android documentation

