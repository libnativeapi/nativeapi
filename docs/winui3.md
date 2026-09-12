# Optional WinUI 3 backend on Windows

`NATIVEAPI_ENABLE_WINUI3=ON` selects WinUI 3 for new Windows menus and
message dialogs. Menus use `Microsoft.UI.Xaml.Controls.MenuFlyout`; message
dialogs use `Microsoft.UI.Xaml.Controls.ContentDialog`. Both share a lazy XAML
runtime and use private XAML Islands, including in tray-only applications.
Future WinUI components should use this same option and runtime.

With the option OFF (the default), menus and dialogs retain their Win32
implementations. Public C++ and C interfaces stay platform independent.

## Build

The optional backend requires MSVC, a Windows 10/11 SDK, and Windows App SDK 1.6.
The default build, including MinGW, has no new dependencies. This integration
currently pins the 1.6 package layout; newer SDK versions require validation before
changing the version guard.

From the repository root, in PowerShell:

```powershell
./cmake/RestoreWinUI3.ps1
$packages = (Resolve-Path build/winui3-packages).Path
cmake -S . -B build/menu-modern -G "Visual Studio 17 2022" -A x64 `
  -DNATIVEAPI_ENABLE_WINUI3=ON `
  "-DNATIVEAPI_WINAPPSDK_DIR=$packages/winappsdk" `
  "-DNATIVEAPI_CPPWINRT_EXE=$packages/cppwinrt/bin/cppwinrt.exe" `
  "-DNATIVEAPI_WEBVIEW2_DIR=$packages/webview2"
cmake --build build/menu-modern --config Debug --target menu_example message_dialog_example menu_backend_test winui3_dialog_test
./build/menu-modern/examples/menu_example/Debug/menu_example.exe
```

The restore script verifies pinned package hashes. It downloads build dependencies
only, without installing a runtime. WebView2 metadata is required to generate WinUI
projections; the menu does not instantiate WebView2 or require its browser runtime.

Install the matching [Windows App Runtime 1.6](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads-archive)
on deployment machines. For each consuming executable, call
`nativeapi_deploy_winui3(your_executable_target)` after `add_subdirectory(nativeapi)`
and creating the target. This copies the bootstrap DLL beside the executable.
For a DLL/plugin, deploy beside the **host executable**. The library dynamically
loads the bootstrap DLL only on first WinUI component use; a missing runtime does not
prevent native menus from working. An existing WinUI host's package graph and
dispatcher are reused.

The host executable must declare **PerMonitorV2** DPI awareness in its application
manifest to render crisp text at high display scaling. The bundled examples and
WinUI smoke test embed `cmake/winui3-example.manifest`. Add equivalent DPI settings
to your own application's manifest (a DLL manifest cannot set the host's DPI mode).
The library does not change process-wide DPI settings, since that would affect
other windows owned by the host. Without DPI awareness Windows bitmap-scales the
finished menu, which makes text and edges blurry.

## C++ and C usage

To try the tray integration, build and run the tray example instead:

```powershell
cmake --build build/menu-modern --config Debug --target tray_icon_example
./build/menu-modern/examples/tray_icon_example/Debug/tray_icon_example.exe
```

Right-click the tray icon whose tooltip is **nativeapi WinUI3 Tray Test - right-click**
(it may be in the tray overflow). Select **Exit** to stop the example.

```cpp
Application::GetInstance(); // Initializes COM on the UI thread.
auto menu = std::make_shared<Menu>();
menu->AddItem(std::make_shared<MenuItem>("Open"));
menu->Open(PositioningStrategy::CursorPosition()); // WinUI3 when compiled ON.

MessageDialog dialog("Information", "This is a real WinUI 3 ContentDialog.");
dialog.SetModality(DialogModality::Application);
dialog.Open();
```

```c
native_menu_t menu = native_menu_create();
/* The build option selects the default backend for the C API too. */
/* Add items, register listeners, and open using the existing C API. */
native_menu_free(menu);
```

`IsBackendSupported()` reports compile-time capability. `SetBackend()` rejects
unsupported backends, changes while open, and modern presentation for menus wrapping
an external `HMENU`. Runtime initialization failure makes `Open()` return false
and logs a diagnostic. Submenus use their root's backend.

## Message dialogs

Run `./build/menu-modern/examples/message_dialog_example/Debug/message_dialog_example.exe`.
No runtime switch or new API is required.

- `None` returns after presentation. Keep the dialog alive and dispatch the UI
  message loop (normally `Application::Run()`). `Close()` dismisses it.
- `Application` pumps messages until dismissal and temporarily disables the
  application's currently visible, enabled top-level windows.
- `Window` disables the calling thread's active visible window. The public API
  has no explicit parent parameter yet; if no active window exists, the dialog
  opens without an owner. It never blocks other applications.
- `SetTitle` and `SetMessage` update displayed content. Long messages scroll.
  The OK button, title-bar close, and `Close()` dismiss the dialog. Previously
  enabled windows are restored on dismissal, initialization failure, or destruction.
- Create, mutate, open, close, and destroy on the same STA UI thread. Reopening
  an already open instance is rejected. WinUI runtime errors return false from
  `Open()` and are logged, with no silent fallback to Win32.
- Hosts and example executables use PerMonitorV2 sizing, including monitor changes.

The implementation follows Microsoft's [ContentDialog hosting requirements](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls.contentdialog?view=windows-app-sdk-1.6).

## Menu behavior and limits

- Call creation, opening, closing, and mutations on the same STA UI thread. Keep
  the root menu alive until `Open()` returns. One modern menu session can run per
  thread; nested root opens are rejected.
- `Open()` continues pumping messages until dismissal. `Close()`, clicks outside,
  and normal WinUI dismissal end the session. `WM_QUIT` is preserved for the host.
- Normal items, separators, checkbox/radio items, icons, tooltips, disabled items,
  and nested submenus use WinUI controls. Checked state remains application-owned,
  matching the existing example: update it in the click listener.
- Labels, icons, enabled/checked state, tooltips, and shortcut text update while
  displayed. Structural edits (insert/remove/replace submenu) are reflected on the
  next root open. Root opening listeners run before constructing the item tree;
  submenu lifecycle events follow the child presenter's loading/unloading.
- Keyboard accelerator labels are displayed. This backend does not register
  application/global shortcuts; use the existing shortcut API for those.
- WinUI checkbox controls are two-state. Mixed checkbox state is currently shown
  as unchecked. Use the native backend if tri-state presentation is required.
- `GetNativeObject()` still returns the library-owned `HMENU`, never a WinRT
  pointer. It represents the native mirror; external direct `HMENU` changes do not
  update the modern presentation. Do not destroy this borrowed handle.
- Positioning uses the existing screen/DIP conversion followed by WinUI placement
  and edge avoidance. Themes and accessibility come from WinUI. Appearance can
  differ from File Explorer's shell menu.

## Verification

`ctest --test-dir build/menu-modern -C Debug -R menu_backend_test --output-on-failure`
checks backend selection without requiring the runtime. The explicit interactive
smoke test opens and closes the menu twice and checks lifecycle/reentry:

```powershell
./build/menu-modern/tests/Debug/menu_backend_test.exe --winui3
```

The test also exercises live property updates and cancellation from an opening
listener. Add `--interactive` to keep each popup open for up to a minute in a
preview window.

The same selection test should pass in a default build with the feature disabled.

The explicit desktop dialog integration test covers modal and modeless opening,
reentry rejection, live updates, programmatic/title-bar close, owner restoration,
destruction while open, and sharing the runtime with a menu:

```powershell
./build/menu-modern/tests/Debug/winui3_dialog_test.exe
```
