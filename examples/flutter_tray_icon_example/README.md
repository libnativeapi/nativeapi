# tray_icon_example

A playground for `TrayIcon` and `TrayManager`, built to do three jobs in one small
(800 × 600) window:

- **show animated tray icons** — Flutter renders every frame and hands it to the tray;
- **accept a platform** — every API has a control, and a checklist ticks itself;
- **record a demo** — everything is one click, no keyboard needed.

It is drawn with [DazzUI](https://pub.dev/packages/dazzui) on the examples' shared host
([`flutter_dazzui_host`](../flutter_dazzui_host)) — no material and nothing hand-drawn:
every control is a `Toggle`, `Button`, `Badge`, `OptionCard`, `Card`, `SegmentedControl`
or `Dialog` from the design system. The theme follows the platform's light or dark
appearance, and on [Omarchy](https://omarchy.org) the desktop's own palette and
rounding, live across `omarchy-theme-set`.

```bash
flutter run -d macos   # or windows, linux
```

## The window

| Part | What it is for |
| --- | --- |
| **Icons / Signs** sidebar | Independently select, edit and remove images and custom content views. |
| **Add** | A single menu creates a default icon, Download / Recording / Syncing scene, three animated icons, or any of the four sign styles. Existing items stay intact. |
| **Content** | The selected item's editor: live animation tiles and frame controls for icons; a compact row of sign styles, text and display options for signs. |
| **Properties** | Native getter read-back, visibility, tooltip, menu trigger and popup mode. Signs supply their own text rather than an icon title. |
| **Checklist** | Existing automatic and manual API acceptance checks. |
| Event footer | Latest event and call; Log opens the history. |

## Native signs

Native signs are integrated into this example alongside image icons.
The four styles are Missing You, City Welcome, Travel and Scenic Guide. Each item
keeps its own content for every style, colors and directions. The Missing You pinyin
strip always displays. All controls are in English; the editable sample sign text
remains Chinese.

On macOS, each sign owns a native `View` / `Label` tree mounted in
`TrayIcon.contentView`, with 4 pt of transparent space on each side. A separate tree
is embedded in Flutter through an AppKit `PlatformView`: the inline face is 40 pt
high in an 80 pt preview area. Style choices fit on one row. **Enlarge preview**
opens a separate native window that follows edits. Closing the settings window
keeps tray items running; click a sign to select it and return to its editor.

Native typography, PlatformView registration and window ownership are in the macOS
runner; the sign geometry and lifetimes remain in Dart/nativeapi. Windows and Linux
keep the image-icon example and disable sign creation because custom tray content
views are currently available on macOS.

## Animated icons

`IconAnimator` runs a timer at the chosen rate. Per frame it draws on a canvas (or
screenshots a widget), encodes a PNG, wraps it in a nativeapi `Image` and assigns
`trayIcon.icon`. A frame that is not finished when the next tick arrives counts as
dropped, so the numbers in the preview are the real cost of animating a tray icon.

- *Spinner, Pulse, Blink, Progress, Wave, Rotate* — canvas drawings over time.
- *Clock* — driven by data (the wall clock), not by a loop.
- *Any widget* — a live `Container` + `Text` widget captured through a
  `RepaintBoundary`: whatever Flutter can lay out can be a tray icon.
- Scenes pair an animation with a title that follows it (*Download*: `42%`,
  *Recording*: `00:12`); *Three icons* animates three icons together.

What a platform cannot do is not offered, and its checklist items step aside:

- Windows tray icons have no title (`setTitle` is a no-op, `getTitle` returns null).
- A Linux tray icon is a StatusNotifierItem drawn by the shell: `getBounds` is empty and
  `openContextMenu` returns false, so there is no *Window to icon* and no *Open menu*.
  Frames also cost far more there (GPU read-back, PNG, D-Bus): the preview's numbers
  tell what rate a machine really holds.

macOS shows tray images as 18 pt templates, so there the colour row only changes the
preview's alpha shape and the menu bar picks the tint; resolution still matters (1x / 2x
/ 3x pixels for the same 18 pt). On Omarchy the *Auto* colour is the desktop theme's
foreground, like the bar's own icons, and follows a theme switch; since the asset
still is a fixed white image, an icon showing it switches to the drawn star there.

## Popup mode

*Properties → Popup* (or `TRAY_POPUP=1` in the environment) makes the window behave like
a tray utility's: no title bar, a click on the icon shows it, a click while it shows
hides it, and losing the focus hides it (`WindowBlurredEvent` → `hide()`). Every show is
a fresh `show()` after a `hide()`, not a raise.

Where the window appears is the platform's business. On macOS and Windows *Window to
icon* places it next to the icon from `getBounds`. On Wayland the app can neither read
the icon's position nor place its own window, so the compositor has to do it. On
Hyprland (0.56+, Lua config) a window rule evaluated when the window maps does — the
cursor is still on the icon at that moment:

```lua
-- ~/.config/hypr/hyprland.lua: right edge 20 px right of the click, top 20 px below it
hl.window_rule({
  match = { class = "^com\\.example\\.tray_icon_example$" },
  float = true,
  pin = true,
  no_anim = true,
  move = { "(cursor_x-window_w+20)", "(cursor_y+20)" },
})
```

The example sizes the window itself (800 × 600), so the rule needs no `size`. Static
rules run once per map, which is why popup mode hides the window instead of
lowering it. `tools/gui/flutter_tray_popup_test_hyprland.py` installs this rule for a run
and checks the whole sequence with a real click in Omarchy's bar.

What it looks like there: about 150 ms from the click to the first frame, which is
already the final layout. With a title bar it would re-lay out once more: Hyprland
reports every toplevel as tiled and maximized, floating or not, so that it draws no
client-side shadow, and GTK maps a decorated window with its CSD shadow margins first
and drops them when those states arrive. Without the title bar core suppresses GTK's
decoration margin as well, and never adds its own shadow gutter before the compositor
has answered (`core/tests/window_shadow_remap_linux_test.cpp`).

## Files

| File | |
| --- | --- |
| `lib/tray_controller.dart` | every `TrayIcon` / `TrayManager` call, scenes, events, checklist wiring |
| `lib/icon_animator.dart` | the frame loop: canvas or widget → PNG → `TrayIcon.icon` |
| `lib/icon_animations.dart` | what the frames look like |
| `lib/signs/` | Native sign geometry, per-item state, PlatformView preview and template thumbnails |
| `lib/widgets/add_tray_menu.dart` | The single Add menu for icons, scenes and signs |
| `lib/context_menu.dart` | the tray's context menu (normal, checkbox, disabled, submenu) |
| `lib/checklist.dart` | checklist model and report |
| `lib/tabs/`, `lib/widgets/` | the UI; the chips, the event footer and the theme come from `dazzui_host` |

## Testing and recording

The workspace repo drives this example with real mouse input:
`tools/gui/flutter_tray_icon_test.py` (asserts on the frame counters, the read-back
state and the `[checklist]` lines this app prints) and
`tools/gui/flutter_tray_icon_demo.py --record` (the demo video).

`tools/gui/flutter_tray_icon_sign_test.py` checks the integrated native signs, independent state, PlatformView attachment and disposal on macOS without synthetic input.
