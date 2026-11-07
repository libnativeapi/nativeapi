# window_manager #593–#596 verification

Verified on 2026-11-07. These fixes live in core; window_manager consumes them
through the next nativeapi release. No package release was made during testing.

#593 and #595 have source fixes and passing targeted regressions. #594 has a
verified GTK background-priority fix but remains open for KDE/FlView confirmation.
#596 remains open because its reported performance regression was not reproduced.

## #593: completion must return before an approved close can destroy the engine

Event acknowledgment releases its payload, then posts the implicit vote to the
main loop. The request continuation therefore runs after the foreign call returns.
Embedders without a dispatcher retain the existing synchronous fallback.

- The event-delivery regression checks both cancellation and final approval before
  and after draining the dispatcher, including payload/borrowed-handle lifetimes
  and concurrent duplicate acknowledgments: 40 checks passed on macOS and Linux.
- Windows 11 build 26200, Flutter 3.47.7: three real legacy close flows, each
  canceling twice and then approving, passed all 15 checks. Processes exited with
  code 0 and no Dart isolate assertion. Flutter 3.47.9 also passed both immediate
  approval and cancel-then-approve flows.
- These runs explicitly merge Flutter's UI and platform threads, recording their
  thread IDs. With the separate-thread policy, `GetCurrent()` finds no active
  window on the Dart thread, so the legacy listener is not connected. This occurs
  with published 0.5.5 and the local build and is **not** counted as verification.
- Published nativeapi 0.5.5 on Flutter 3.47.6, 3.47.7 and 3.47.9 did not reproduce
  the reported VM assertion in this fixture. The deterministic regression proves
  the unsafe inline continuation is removed; the desktop runs prove the fixed
  legacy flow exits normally on this test machine.

## #594: transparent GTK backing must survive KDE theme providers

The gutter background rule previously had application priority. KDE's GTK modules
install user-priority providers; a higher-priority opaque `window` rule overrides
the intended transparent backing. The scoped gutter rule now uses user priority
plus one, matching those providers while retaining its more specific selector.
Removing the gutter class restores the ordinary window background.

A real GTK/Wayland fixture injects an opaque provider at the KDE modules' priority.
Before the change, GTK reports background alpha 1 in all six hidden-window phases.
After the change it reports 0, and restoring normal decorations returns alpha 1.
The Cairo gutter samples remain nonopaque and content geometry/input are checked
independently. This fixture does not claim to capture Flutter's GPU surfaces.

## #595: input bounds must use the allocation being applied

During GTK's `size-allocate`, the child already has the new gutter offset while
the GdkWindow may still have its previous dimensions. Clipping the shape to that
old surface truncates the right and bottom edges. Rasterization now uses the
current GTK toplevel allocation; non-GTK windows retain their GDK bounds.

The test records `WAYLAND_DEBUG=client` and checks the input regions actually
submitted to the compositor. Before the fix, five phases lost content corners;
the region was 132 pixels shorter than the content at the right and bottom.
After the fix, all four content corners accept input, while the gutter passes it
through. The resize phase must actually reach its requested 560×360 content size.

Combined #594/#595 results:

| Desktop | GTK | Result |
| --- | --- | --- |
| GNOME 46, private Wayland, software rendering | 3.24.41 | 25 checks passed |
| KWin 6.8.1, private Wayland, software rendering | 3.24.52 | 25 checks passed |

Coverage: hidden title bar, resize, shadow off/on, hide/show remapping, maximize,
restore and normal decorations. KWin was run in a private runtime/config/bus,
using extracted packages without changing the logged-on desktop. An earlier KWin
run with an unsuitable compositor backend did not apply resizes; its results were
discarded. Fedora 44/KWin 6.8.2 with the reporter's exact GPU is not covered.

The real window_manager/FlView fixture was built with Flutter 3.47.6. On GNOME,
all seven phases kept Flutter's logical view size equal to the native content
size, including 560×360 resize, shadow toggles, maximize/restore and normal
decorations. In the private KWin 6.8.1 session, that same bundle's Flutter view
stayed at 1280×720 while native geometry changed; all seven layout checks failed.
The KWin host then became unreachable, preventing further diagnosis. This
FlView run is **not** counted as a passing end-to-end KDE verification; the
25-check KWin result above covers GTK background and submitted input regions.

## #596: no attributable resize regression reproduced

The same minimal Flutter app runs in three modes: control without any nativeapi
calls, `getCurrent()` only, and `getCurrent()` plus a no-op listener. The control
has zero nativeapi DLLs loaded; the other modes have one. Each uses the same SDK,
runner, window size and initialization timing.

Windows 11 build 26200, Flutter 3.47.9, published nativeapi 0.5.5, 60 native
`SetWindowPos` resizes per mode:

| Mode | Frames | Raster p95 | UI build p95 | SetWindowPos mean / p95 |
| --- | --- | --- | --- | --- |
| Control | 120 | 10.435 ms | 2.963 ms | 14.35 / 18.77 ms |
| getCurrent | 120 | 10.560 ms | 3.068 ms | 14.83 / 18.77 ms |
| No-op listener | 120 | 10.534 ms | 3.153 ms | 15.19 / 18.83 ms |

Separate owner-guarded physical mouse resize drags on Flutter 3.47.6 produced 162
frames per mode, with raster p95 11.669 / 11.650 / 12.041 ms respectively. Those
gestures were explicitly authorized, checked idle/ownership and stopped on user
activity or focus changes. No keyboard events were sent.

The screenshot in the issue shows a 165 Hz budget (about 6 ms) and most raster
time in `FlutterWindowsCompositor::PresentLayers`. That points to a presentation
wait worth investigating, but does not identify its cause. A DevTools timeline
export, exact Flutter/nativeapi versions, build mode, GPU/driver, and monitor
refresh/scaling are needed to reproduce it. This issue remains open; no speculative
Windows rendering change is included.

## Repeating the tests

Build the Windows fixture with local packages or a pinned published baseline:

```sh
python tools/gui/build_window_manager_reports.py --flutter <flutter.bat>
python tools/gui/build_window_manager_reports.py --flutter <flutter.bat> --published 0.5.5 --label baseline
powershell -NoProfile -ExecutionPolicy Bypass -File tools/gui/window_manager_windows_reports_test.ps1 -Case Close -VetoCount 2
powershell -NoProfile -ExecutionPolicy Bypass -File tools/gui/window_manager_windows_reports_test.ps1 -Case Resize
```

The PowerShell runner schedules only its own fixture into the logged-on desktop.
`-AllowPointer` additionally performs guarded real drags and requires consent per
the gui-test skill. `-ExpectCrash` is an optional diagnostic assertion, not an
assumption that every published baseline must fail.

On Linux, build core first, then use an isolated compositor:

```sh
cmake --build core/build --target nativeapi event_delivery_test -j6
core/build/tests/event_delivery_test
NATIVEAPI_REPORT_HIGH_PRIORITY_THEME=1 python3 .agents/skills/gui-test/scripts/linux/gnome_headless.py --size 1536x1000 -- python3 tools/gui/window_manager_linux_reports.py
NATIVEAPI_REPORT_HIGH_PRIORITY_THEME=1 python3 tools/gui/window_manager_kwin_test.py --kwin <kwin_wayland>
python3 tools/gui/build_window_manager_reports.py --platform linux --flutter <flutter>
NATIVEAPI_REPORT_HIGH_PRIORITY_THEME=1 python3 .agents/skills/gui-test/scripts/linux/gnome_headless.py --size 1536x1000 -- python3 tools/gui/window_manager_linux_flutter_test.py
```

The KWin runner accepts `--environment-json` for rootless library/plugin/data paths
and `--flutter` for the FlView fixture. Both compositor suites send no synthetic
input. Build overrides and runner instrumentation are restored in `finally`;
bundles, raw frame timings, app logs and Wayland traces remain in ignored build
directories. Remote commands use fastforge; never sync over an active build.
