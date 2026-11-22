# Windows frameless focus (#50)

Build on the Windows host from `examples/flutter_window_example`:

```sh
fastforge --no-version-check host exec windows-laptop -- flutter build windows --debug --target ../../tools/gui/window_frameless_focus_flutter.dart
```

Run from the workspace root:

```sh
fastforge --no-version-check host exec windows-laptop --no-sync -- powershell -NoProfile -ExecutionPolicy Bypass -File tools/gui/flutter_frameless_focus_test.ps1
```

Verified on `windows-laptop`, Windows 11 x64 build 26300, Flutter 3.50.0 /
Dart 3.14.0, core `9307412`: **33 checks passed**, process exit code 0.
The scenario sends no synthetic input. It switches native focus between two
owned windows three times and asserts both focus states, frameless state,
disabled DWM non-client rendering, matching native client/outer dimensions, and
black screen pixels at the midpoints of all four edges. It then restores and
checks the normal frame. Results/logs remain under `core/build/frameless-focus-*`
on the host. The anchor stays inside the host's content, away from those samples.

Core `29c0b9b` already suppresses Windows 11's DWM border for explicit frameless
windows; published cnativeapi 0.6.1 does not include that suppression. This PR
adds coverage and Unreleased notes for the existing implementation fix.
`DWMWA_BORDER_COLOR` is a set-only attribute according to the
[Microsoft documentation](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute),
so the test uses actual desktop pixels instead of attempting to read it back.
It checks settled focus states, not every intermediate animation frame; physical
clicks, Flutter 3.50.1 and other Windows builds were not tested in this run.
