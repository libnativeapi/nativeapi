# gpui_tray_icon_example

A control panel for the nativeapi `TrayIcon` API: tray icons created,
animated, tinted, titled and menu'd from a window drawn with
[DazzUI for GPUI](https://github.com/dazzlabs/dazzui/tree/main/packages/rust/dazzui)
— the same design system the Flutter example's host uses. It is the GPUI
counterpart of `flutter_tray_icon_example`, built on the Rust binding.

## Running

```bash
cd examples/gpui_tray_icon_example
cargo run
```

This crate is its own cargo workspace, not a member of the root one: GPUI is
a large dependency with its own platform requirements, and the root
workspace's CI builds every member. It enables GPUI's `runtime_shaders`
feature, so building on macOS does not need Xcode's Metal toolchain.

`dazzui` is consumed as a git dependency, which resolves through the Cargo
workspace at the root of the dazzui repository.

`TRAY_POPUP=1 cargo run` starts in popup mode: a click on the icon shows the
window and losing focus hides it again, the way a tray utility behaves.

`TRAY_SMOKE=1 cargo run` is a scripted pass for the GUI test harness: it
plays the spinner, adds a sign, opens its preview and quits, so the
`[checklist]` and `[popup]` lines a script asserts on appear without a hand
on the mouse.

## Using it

The page has the same sidebar, three tabs and footer as the Flutter example:

- **Sidebar** — every tray item as a card (`Icons` and `Signs` groups), and
  **+ Add**: a default icon, one of the three scenes, three icons at once, or
  a sign (macOS).
- **Content** — the animation gallery: eight tiles drawn live, one click
  plays the animation in the tray (`Spinner`, `Pulse`, `Blink`, `Progress`,
  `Wave`, `Rotate`, `Clock`, `Any widget`); icon presets (Download, Recording,
  Syncing, Three icons); and the frame rows — still icon (asset, drawn star,
  base64 diamond), rate (10/24/30/60 fps), resolution (1×/2×/3×) and colour
  (Auto follows the menu bar's tint on macOS). For a sign, the style cards,
  its content fields and display options instead.
- **Properties** — what the native getters return after every write: id,
  visibility, trigger, title, tooltip, bounds, the click counters, and rows
  for Title (with the `Title…` dialog), Tooltip, Visible, Trigger, Popup,
  Menu (open from code, close after 2 s) and the backend on Windows.
- **Checklist** — the acceptance list: the auto items tick themselves from
  events and return values, the manual ones need eyes on the tray; the
  footer copies a plain-text report or resets the list.
- **Preview strip** — the very image the tray received, frame by frame, with
  Pause / Step / Stop, or the drawn sign. For a sign, **Enlarge preview**
  opens a native window with the sign's own views.
- **Footer** — the last event, the newest call, the log popover and Clear.

Right-click the tray icon for its menu: Show window, an Animate submenu with
every animation, a checkbox, a disabled item, a submenu event, About and
Quit.

## How it maps to the Flutter example

Feature-for-feature, file-for-file: `tray_controller.rs` ↔
`tray_controller.dart`, `context_menu.rs` ↔ `context_menu.dart`,
`icon_animations.rs` / `icon_animator.rs` / `icon_tint.rs` ↔ the Dart files
of the same names, `signs/` ↔ `signs/`, `tabs/` ↔ `tabs/`, and `host.rs` ↔
the Flutter examples' `dazzui_host` package (Studio Light/Dark by the
window's appearance, OptionChip/ActionChip/OptionRow/Hint on the same DazzUI
primitives).

Two things GPUI cannot do, done the closest way instead:

- The **Any widget** animation screenshots a live Flutter widget in the
  Flutter example; GPUI cannot capture an element, so the same counter badge
  (ring with a bouncing digit) is drawn on the CPU with tiny-skia. Every
  other animation is drawn by the same math.
- The sign's in-window preview embeds the real native view through a macOS
  PlatformView in the Flutter example; GPUI cannot host platform views, so
  the preview is the drawn replica (`sign_art.rs`, the same geometry the
  Flutter thumbnails use). The tray itself carries the real native sign, and
  the enlarged preview window is the native one.

The Flutter example's macOS runner also rounds the sign's face and tunes its
label fonts through a method channel; nativeapi's own views and labels are
used as they are here.

`TrayManager`'s `get`/`getAll` currently return nothing on every platform —
a core gap the Flutter example's checklist shows the same way — so the
*managed* item fails here as it does there.
