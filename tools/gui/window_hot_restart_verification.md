# Windows hot restart (#49)

Run `python3 tools/gui/flutter_hot_restart_test.py --host windows-laptop` from the
workspace root. The test uses the stock `flutter_window_example` Windows runner,
the local bindings and core, and sends no synthetic mouse or keyboard input.

Verified on the Windows x64 `windows-laptop` host with Flutter 3.50.0 / Dart 3.14.0
and core `9307412`:

- Startup and three actual hot restarts returned the same non-null window ID and
  HWND in the same process. A new generation timestamp confirms `main()` ran again.
- The restarted isolate received and cancelled a native close request; the window
  remained available through the VM service with exactly one close notification.
- A second, allowed native close shut down the Flutter engine and the attached
  runner exited successfully.

The implementation fix is already on main in core `7b2e366`: `GetCurrent()` finds
the process's foreground, last-focused, visible or hidden startup window when the
calling Dart thread has no active window. The published cnativeapi 0.6.1 source
only queries `GetActiveWindow()`. This test and the Unreleased changelog entries
cover that existing fix; this PR does not change the core API or publish a release.
Flutter 3.50.1 and manual caption-button clicks were not tested in this run.
