# Linux shadow visuals — window_manager #594

Verified on 2026-11-14 with KWin 6.8.1, Breeze, GTK 3.24.52 and a private
1920×1000 output configured to 125% through `kscreen-doctor`. The compositor runs
on the `omarchy` Linux host with software rendering, independently of its logged-on
desktop. No mouse or keyboard events are sent.

Core fix: [7247fcb](https://github.com/libnativeapi/nativeapi-core/commit/7247fcb).

## Cause and fix

The earlier background-CSS fix covers RGBA windows. A realized 24-bit X11 window
cannot carry alpha, regardless of its CSS. The previous implementation still
reserved 132 logical pixels on each side: a 480×320 client occupied a 744×584
surface with an opaque border. The baseline failed all six RGB gutter assertions
across initial display, shadow toggles, maximize/restore and remapping.

Core now checks the **existing GdkWindow's visual**, rather than only the widget's
preferred visual. Unsupported surfaces keep their content and requested shadow
policy, without allocating or painting a client-shadow gutter or reparenting the
host's content. RGBA surfaces retain their existing shadow behavior.

Full transparent X11 shadows still require an RGBA visual selected before the
runner realizes its window. The English/Chinese window_manager migration guides
and its Linux example now show that setup. GTK documents the
[pre-realization requirement](https://docs.gtk.org/gtk3/method.Widget.set_visual.html).
This change does not publish new packages; nativeapi 0.6.0 lacks the RGB fallback.

## Native verification

`core/tests/window_shadow_visual_linux_test.cpp` exercises real windows through
the public API. It tests RGB, RGBA and a deliberately late RGBA preference (which
does not replace an existing RGB backing). On Wayland the fixture uses the regular
Flutter-style CSD host, whose system and RGBA visuals both support alpha.

- XWayland: all 87 checks pass, including actual 24/32-bit surface depths, X11 backing
  pixels with alpha zero, content-edge input coverage, shadow-corner input exclusion,
  shadow toggles, maximize/restore, hide/show and exact 480×320 restored content.
- Native Wayland: all 63 visual/geometry checks pass, including transparent gutters
  under an opaque user-priority theme rule and exact restored content dimensions.
- Existing minimum-size restoration suite: 48 checks pass on each backend.
- Existing shadow-gutter helper suite: all 29 checks pass on Wayland. On XWayland,
  27 pass and the mapped-CSD/no-explicit-resize check fails (654×534 instead of
  480×360). Building that helper against the unchanged `5407b59` header reproduces
  the same failure; it is not caused by the RGB fallback. This test does not model
  the non-CSD RGB host from #594.

The earlier deliberately undecorated Wayland probe also exposed an existing
non-CSD hide/show size increase. The Wayland regression here uses the regular CSD
runner shape; this change makes no claim to fix non-CSD Wayland remapping.

## Flutter 3.50 verification

The real application uses window_manager 0.7.0 with local nativeapi bindings and
core, the stock runner's visual behavior by default, and the documented RGBA
selection when `WM_RGBA=1`. Eight phases check native GTK allocation/gutter alpha
and Flutter's actual logical view dimensions against `window.contentSize`.

| Backend / runner | Surface | Shadow result | Full geometry suite |
| --- | --- | --- | --- |
| XWayland, default | 24-bit RGB | No reserved gutter; no opaque band | 16 checks pass; all normal/restored content is 800×600 |
| XWayland, RGBA selected | 32-bit RGBA | Gutter alpha is zero throughout | 6 size failures: normal/restored content is 1064×864 instead of 800×600 |
| Wayland, default | 32-bit RGBA | Gutter alpha is zero; fitted gutter removed | 1 size failure after hide/show (1064×864); other phases are correct |
| Wayland, RGBA selected | 32-bit RGBA | Same as default Wayland | Same hide/show size failure |

Flutter's logical view and native content dimensions agree in all 32 samples.
The RGBA/Wayland runs are **not** reported as fully passing end-to-end tests:
their remaining geometry failures are retained in the fixture's nonzero result.
The RGB fallback and RGBA transparency are verified separately from those results.

For a baseline comparison, the same Flutter executable/assets were rerun with
the pre-fix `libcnativeapi.so` from the #35 verification (core `18d2a41`). Default
XWayland failed all eight gutter checks there and passes all eight with this fix.
The baseline RGBA XWayland and native Wayland runs reproduce exactly the same six
and one size failures above. Those are existing geometry issues, not regressions
introduced by checking the surface visual.

Retained run IDs under `core/build/window-manager-kwin/`:

| Run | ID |
| --- | --- |
| Native XWayland visual/input checks | `1794669493495551711` |
| Native Wayland visual + gutter + minimum suites | `1794669525464289715` |
| Native XWayland minimum suite | `1794669526623343101` |
| Baseline XWayland gutter helper | `1794669509857262439` |
| Flutter default XWayland, fixed / baseline | `1794669858894998493` / `1794669951487367011` |
| Flutter RGBA XWayland, fixed / baseline | `1794669867157932423` / `1794669952670726333` |
| Flutter default Wayland, fixed / baseline | `1794669868349814250` / `1794669953888583951` |
| Flutter RGBA Wayland, fixed | `1794669869657901727` |

## Commands

From a synced Linux workspace with GTK/X11 development files and KWin installed:

```sh
cmake -S core -B core/build -DBUILD_TESTING=ON
cmake --build core/build --target window_shadow_visual_linux_test \
  window_shadow_gutter_linux_test window_minimum_restore_linux_test -j6
GTK_THEME=Breeze python3 tools/gui/window_manager_kwin_test.py \
  --kwin /path/to/kwin_wayland --kscreen-doctor /path/to/kscreen-doctor \
  --backend x11 --scale 1.25 --width 1920 --height 1000 \
  --command core/build/tests/window_shadow_visual_linux_test
```

Repeat with `--backend wayland`. An extracted rootless KWin installation can pass
its library/plugin/data paths using `--environment-json`. Run the other targets
with the same compositor wrapper; retain the known XWayland helper failure above.

The Flutter fixture builds with local binding overrides in an ignored generated
project, never in a published manifest:

```sh
python3 tools/gui/build_shadow_visual_flutter.py
GTK_THEME=Breeze python3 tools/gui/window_manager_kwin_test.py \
  --kwin /path/to/kwin_wayland --kscreen-doctor /path/to/kscreen-doctor \
  --backend x11 --scale 1.25 --width 1920 --height 1000 \
  --command core/build/shadow-visual-flutter/build/linux/x64/debug/bundle/shadow_visual
```

Run once without `WM_RGBA`, then with `WM_RGBA=1`, and repeat on Wayland. The runner
has a read-only GTK measurement hook; the only visual change is the documented
pre-realization RGBA selection when `WM_RGBA=1`. It installs no CSS workaround.
Each run records native surface geometry and gutter alpha while Dart toggles
shadows, maximizes/restores twice, and hides/shows its window.

Logs and actual output configuration are retained under
`core/build/window-manager-kwin/<run>/`. Tests cover programmatic state transitions
and native input regions, not physical dragging, task-switcher thumbnails or GPU
driver behavior.
