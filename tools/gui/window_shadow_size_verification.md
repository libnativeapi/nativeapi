# Linux shadow size regression verification

Verified on 2026-11-15. Core fix:
[bf72f78](https://github.com/libnativeapi/nativeapi-core/commit/bf72f78).
This resolves the RGBA XWayland and non-CSD Wayland size failures retained in
the [original #594 verification](window_shadow_visual_verification.md).

## Fix

On XWayland, the non-CSD host requested its enlarged shadow surface before
publishing the new frame extents. KWin interpreted the request using the old
extents and enlarged the client again. Core now declares the extents before
requesting the surface resize. An 800×600 client keeps that size inside its
1064×864 shadow surface, including the first display without a corrective resize.

On Wayland, GTK 3 updates its next-map content-size cache while the old shadow
margins are still installed. After declaring new extents, core refreshes that
cache with the current surface dimensions and final margins. This refresh is
skipped while maximized, fullscreen or tiled: a resize in those states would
overwrite GDK's saved restore dimensions. GTK-owned CSD geometry remains on its
existing path. There are no public API or generated-binding changes.

## Results

All compositor runs below use private KWin desktops, Breeze, a 1920×1000 output
and an actual 125% output scale verified through `kscreen-doctor`. No keyboard or
mouse events are sent. `omarchy` runs KWin 6.8.1 / GTK 3.24.52; `ubuntu-nuc` runs
rootless KWin 5.27.11 / GTK 3.24.41. Flutter is 3.50.0, with local window_manager
and nativeapi bindings. The Flutter bundle was built on Ubuntu and also executed
on omarchy against that host's GTK/compositor.

| Suite | Host / backend | Result |
| --- | --- | --- |
| Native RGB/RGBA visual, input and size regression | Ubuntu / XWayland | 168 checks pass |
| Native visual and size regression, including CSD and non-CSD hosts | Ubuntu / Wayland | 109 checks pass |
| Native shadow-gutter helper | Ubuntu / Wayland | 29 checks pass |
| Native minimum/maximum-size restoration | Ubuntu / both backends | 48 checks pass on each |
| Full Flutter RGBA fixture | omarchy / XWayland | 22 checks pass |
| Full Flutter default fixture | omarchy / Wayland | 22 checks pass |
| Full Flutter default RGB and selected RGBA fixtures | Ubuntu / XWayland | 22 checks pass for each |
| Flutter RGBA shadow toggles and repeated hide/show, without maximize | Ubuntu / Wayland | 14 checks pass |

The full Flutter fixture checks native gutter transparency and exact normal
content size, plus the actual Flutter logical view against native content size.
It toggles shadows, hides/shows three times, maximizes/restores twice, and then
hides/shows again. All normal and restored content remains 800×600. The native
fixture also checks first display before any corrective resize and three remaps
after restoring; the maximized dimensions must not replace the saved normal size.

For a direct Wayland comparison, the **same updated Flutter executable and assets**
were rerun on KWin 6 with the old shared library retained from the #594 verification.
That baseline passes the remaps before maximization, but the final hide/show after
maximize/restore grows the content to 1064×864 and fails. The fixed library passes
all 22 checks. The earlier six RGBA XWayland size failures likewise pass with the
fixed library, now including the additional repeated-remap checks.

## Reproduction

Use the build and compositor commands in the
[original verification](window_shadow_visual_verification.md#commands), with the
current sources. Run the Flutter fixture on both backends, with and without
`WM_RGBA=1`. The default remains the full test; `WM_SKIP_MAXIMIZE=1` selects the
focused shadow-toggle/remap test and must not be reported as maximize coverage.

KWin 5 on Ubuntu stalls the Flutter fixture during unmaximize with both the old
and fixed library. Its focused remap run is supplemental; the complete Wayland
result comes from KWin 6. Ubuntu's rootless XWayland also needs an `Xwayland -shm`
launcher with `LIBGL_ALWAYS_SOFTWARE` unset to start successfully with its Mesa
installation. Those are test-environment accommodations, not application changes.

The earlier mapped-CSD/no-explicit-resize X11 helper failure remains outside these
non-CSD fixes, as documented in the original report. A separate attempt on the
logged-on GNOME session produced invalid-monitor and pending-allocation warnings
and failed geometry checks; it is not counted as passing GNOME validation. This
report establishes the two requested fixes on KWin, not general GNOME coverage.

## Retained logs

Each run's `client.log`, `kwin.log`, `exit-code` and `outputs.json` are under
`core/build/window-manager-kwin/<id>/` on the named host.

| Host / run | ID |
| --- | --- |
| Ubuntu native XWayland visual | `1794686059034736231` |
| Ubuntu native Wayland visual + gutter + minimum | `1794699824627090846` |
| Ubuntu Flutter RGBA XWayland | `1794699825819829928` |
| Ubuntu Flutter RGB XWayland + native minimum | `1794700671054961820` |
| Ubuntu Flutter focused RGBA Wayland | `1794695355691432198` |
| omarchy Flutter full Wayland, fixed | `1794700716195659700` |
| omarchy Flutter full Wayland, baseline | `1794700751339484225` |
| omarchy Flutter full RGBA XWayland, fixed | `1794700752502578945` |

This change does not publish new packages.
