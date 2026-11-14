# Hidden-window minimum size restoration (#35)

Verified on 2026-11-14. The regression was reproduced on `omarchy` with GTK
3.24.52 and a private KWin 6.8.1 Wayland compositor. These tests do not use the
logged-on desktop or inject mouse/keyboard input.

## Cause and fix

The previous minimum hint included the difference between the GDK surface and
the visible frame. For a hidden-title-bar window this includes the 132-pixel
shadow gutter on each side. During a maximize transition, that surface minimum
can outlive the gutter it describes. GDK constrains the configure with the stale
minimum and loses the normal restore size.

GTK windows with content now express the minimum through the content widget's
size request. GTK derives its surface minimum from that requisition and the
current decorations in the same layout pass. The public minimum remains a frame
size: visible decorations are subtracted when calculating the content request.
The implementation preserves the host's previous size request, restores it when
the minimum is cleared, follows replacement children, and keeps the request on
the native window rather than on a temporary C++ wrapper. Maximum and aspect
ratio hints retain their existing behavior; windows without content and bare
GDK windows retain the surface-hint path.

Relevant GTK implementation:
[GtkWindow geometry hints and requisitions](https://github.com/GNOME/gtk/blob/3.24.52/gtk/gtkwindow.c)
and [Wayland configure constraints and saved sizes](https://github.com/GNOME/gtk/blob/3.24.52/gdk/wayland/gdkwindow-wayland.c).

## Results

| Scenario | Before | Fixed |
| --- | --- | --- |
| Native GTK, minimum 800×600, three maximize/restore cycles | 1280×864 after every restore | 800×600 after every restore |
| Same native test without minimum | 800×600 | 800×600 |
| Native GTK, Breeze, Wayland, 125% output scaling | — | 48 checks passed |
| Native GTK, Breeze, XWayland, 125% output scaling | — | 48 checks passed |
| Existing size-constraint suite, Wayland | — | 36 checks passed |
| Existing size-constraint suite, Breeze, XWayland | — | 48 checks passed |
| Existing content aspect-ratio suite, Wayland | — | 9 checks passed |
| Flutter 3.50 stock runner, Breeze, Wayland, minimum enabled/omitted | — | Initial size and all three restores are 800×600 in both runs |
| Same Flutter binary, XWayland, minimum enabled/omitted | — | Initial size and all three restores are 800×600 in both runs |

The new native test covers windows with and without an application header bar,
both minimum-size modes, below-minimum and above-maximum resize requests,
clearing the minimum, and preserving the host's existing widget size request.
It measures native geometry, not screenshots. Interactive resize drags are not
part of this test.

The Flutter fixture is built in a freshly generated project with an unmodified
Linux runner, using Flutter 3.50.0 / Dart 3.14.0 and local `window_manager` 0.7.0
plus the local nativeapi packages. Its Linux bundle was built on `ubuntu-nuc`
and executed on `omarchy` against GTK 3.24.52. The compositor's saved
`outputs.json` confirms scale 1.25 on a 1920×1000 output (1536×800 logical).
Flutter logged subsurface-frame timeout warnings during some transitions, but
the settled initial/restore geometry checks passed. Fedora 45 and KWin 6.8.2
were not independently tested.

The older GTK 3.24.41 / KWin 5.27.11 setup on `ubuntu-nuc` did not reproduce the
native restoration failure. The failing baseline used core `1f3c8f5`; no package
release is part of this fix.

## Reproduce

On a Linux host with GTK development packages, Flutter, KWin and kscreen-doctor:

```bash
cmake -S core -B core/build -DNATIVEAPI_BUILD_EXAMPLES=OFF -DBUILD_TESTING=ON
cmake --build core/build --target window_minimum_restore_linux_test window_size_constraints_linux_test window_aspect_ratio_linux_test -j6
python3 tools/gui/build_minimum_restore_flutter.py

GTK_THEME=Breeze python3 tools/gui/window_manager_kwin_test.py --scale 1.25 --width 1920 --height 1000 --command core/build/tests/window_minimum_restore_linux_test
GTK_THEME=Breeze python3 tools/gui/window_manager_kwin_test.py --scale 1.25 --width 1920 --height 1000 --command core/build/minimum-restore-flutter/build/linux/x64/debug/bundle/minimum_restore
```

Add `--backend x11` before `--command` to run the client through the private
XWayland server. Set `WM_SKIP_MINIMUM=1` for the Flutter control. For extracted
KWin packages, provide `--kwin <path>` and `--environment-json <paths.json>`;
`--kscreen-doctor <path>` overrides its location. The runner verifies the actual
output scale, retains compositor/client logs, native exit status and output
configuration under `core/build/window-manager-kwin/<run-id>`, and fails on
`FAIL` output even if a Flutter window exits with status zero.

Recorded runs on `omarchy`:

- Native baseline: `1794666475766643792`.
- Native Breeze Wayland: `1794666920544885957`.
- Native Breeze XWayland: `1794666799177312944`.
- Existing constraints: `1794666673585545866`; aspect ratio: `1794666697894478020`.
- Existing constraints on XWayland: `1794667024894672839`.
- Flutter Wayland with minimum: `1794666920304680859`; control: `1794666986824772499`.
- Flutter XWayland with minimum: `1794666987953511038`; control: `1794667026079410430`.
