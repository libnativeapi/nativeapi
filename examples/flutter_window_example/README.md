# window_example

A control panel for the nativeapi `Window` and `WindowManager` API, drawn with
DazzUI over the shared `flutter_dazzui_host` package (no Material).

- **Canvas** — a to-scale, zoomable map of every display (with its work area)
  and every window (with its content bounds). Tap a window to select it; the
  cards under the map read back its identity, state and geometry, with the
  four most used actions.
- **State**, **Geometry**, **Appearance**, **Behaviour** — every getter and
  setter of the selected window (pick it on the canvas or in the toolbar's
  window list): show / hide, maximize / minimize / full screen, focus, size,
  position and size limits, drag and resize, title and title bar, shadow,
  opacity, visual effect, background colour, stacking, capabilities and the
  platform-specific flags. Every row shows what the native getter returns
  after the change.
- **Events** — the `WindowManager` event log (focused, blurred, minimized,
  maximized, restored, full screen, created, closed, moved, resized); the
  latest lines also show in the footer of every other page.

The toolbar's `…` menu minimizes, restores, shows or hides all windows at once.

```bash
flutter run -d macos   # or windows, linux
```

`tools/gui/flutter_window_events_test.py` drives it (it clicks **Events** and
reads the `Window #<id> …` log lines).
