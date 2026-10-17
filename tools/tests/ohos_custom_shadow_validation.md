# HarmonyOS custom window shadow validation

Checkpoint: 2026-10-16. This advances the continuing [API coverage ledger](ohos_api_coverage.md).
It does not complete the full HarmonyOS API audit.

`Window::SetCustomShadow` and `GetCustomShadow` now have a real OHOS backend.
Core copies the configuration, clips the source contour to the actual window,
renders a three-pass blur on a worker, and supplies straight RGBA pixels to an
optional Stage companion. A private transparent SDK subwindow displays the
image, cannot focus or receive pointer input, and is absent from nativeapi's
window registry. Its visible interior is removed from the raster. Custom colors,
alpha, blur, offsets and explicit polygon shapes use the existing public API;
there are no new public signatures or handles.

Applications must attach `NativeWindowShadowHost` from the library's canonical
`src/platform/ohos/ets/NativeWindowShadowHost.ets`, provide a registered transparent
surface page using `@Entry({useSharedStorage:true})`, and retain/dispose the host
with its Stage. See the [integration instructions](../../core/src/platform/ohos/README.md#window-corners-and-shadows).
The qualified target is an attached child with a hidden title bar. Without the
companion, configuration returns false. UI calls report submission; worker calls
wait for the actual SDK promise. Overlapping configuration requests are refused;
visibility switches during a pending request are serialized. Getters return
independent copies. Clearing restores the library's native default shadow policy.

## Actual emulator results

Host: `macbook-pro`, Apple Silicon; DevEco Studio 26.0.0.851, HarmonyOS
7.0.0.107 / API 26, ARM64. One named guest ran at a time. Both lifecycle-specific
HAP manifests and installed bundle metadata were checked: ordinary `normal` app,
not a system app. No permissions were granted through shell commands.

| Guest | Custom PID / UID | Native checks | Scenes | Real touches | Style regression |
| --- | --- | ---: | ---: | ---: | --- |
| NativeAPI_Phone | 4209 / 20020074 | 124 | 20 | 36 | PID 7880; 50 checks / 14 scenes |
| NativeAPI_Desktop | 9793 / 20020058 | 124 | 20 | 36 | PID 13451; 60 checks / 19 scenes |

Both custom suites have zero failures and `geometryOnly: false`.

The custom suite performs twenty scenes: baseline, disabled configuration, soft
enable/disable through C++ and C, hard red/green through worker/UI paths, queued
off/on, triangle and fully off-window masks, mask reset, native move/resize,
hide/show, clear, default off/on and native SDK destruction. It checks C++/C
configuration agreement and copy isolation, independent SDK helper geometry,
non-focusability/non-touchability, stable focus where applicable, registry
isolation, and retained closed wrappers refusing mutation. Every visible scene
has two guarded real touch probes: the shadow or mask hole reaches the app's
underlying main window, and actual target content reaches the child.

Pixels are asserted from guest screenshots. Hard red/green over white must have
the full source channel and the expected alpha-composited other channels (63,
tolerance 6 for alpha 192). The soft-shadow oracle independently counts the
three uniform discrete box convolutions at a scanline away from vertical edges;
expected alpha is compared with a four-level tolerance. Triangle interior/hole,
off-window clipping, visible contents and disabled/cleared colors are checked.
The separate style suite compares actual native default shadows against direct
SDK reference pixels and checks corner masks and closed wrappers.

Every touch checks fresh SDK identities/rectangles, PID/UID, UITest roots and WMS
visibility before input. On this phone, UITest sometimes clips the main root's
visible rectangle below the child although the main window owns points beside
it. Only known own SDK windows may use their `origBounds`, and the independently
reported SDK frame must contain the press point. Visible foreign roots still
reject input. No host input or keyboard input was sent by this suite.

## Defects found and repaired

- A default `@Entry` did not share the helper's supplied LocalStorage, producing
  a white image. The registered surface explicitly uses shared storage.
- Premultiplying source RGB and passing it through the initial PixelMap path
  reduced the supposedly full red channel. Straight source RGB plus an explicit
  unpremultiplied PixelMap now passes real red/green and soft-alpha checks.
- Phone `getWindowDecorVisible` returns 801, while the direct
  `setWindowDecorVisible(true)` reference succeeds with 0. Query refusal therefore
  does not imply setter refusal. Successful native style setters/queries record
  the known mode; an initially exact full-frame drawable may establish hidden
  mode if no known mode exists. A known decorated mode is never overwritten by
  that heuristic.
- After resizing from 540×360 to 620×420, the phone NDK frame changed but its
  drawable stayed 540×360. This was still present in the settled observation,
  not merely assumed to be a brief delay. It incorrectly looked like decoration
  and prevented shadow refresh. The implementation preserves the established
  caption mode and consumes both public SDK rectangle and size notifications.
  Geometry generations reject stale images while the host waits for the specific
  SDK size. The final phone helper becomes 696×496 for the hard-shadow margin 38.
- Geometry subscriptions remain active while a custom shadow exists, independently
  of business event listeners. Native destruction invalidates pending work and
  removes the helper through the companion's SDK observer.

## Reproduction and evidence

From `core/tests/ohos_smoke`, with the selected disposable guest connected and
the DevEco environment loaded:

```sh
NATIVEAPI_SHADOW_GEOMETRY_ONLY=0 NATIVEAPI_TEST_SUITE=window-shadow \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh "$DEVECO_STUDIO_HOME"
NATIVEAPI_TEST_SUITE=window-style \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh "$DEVECO_STUDIO_HOME" 127.0.0.1:5555 --skip-build
```

Remote archives: `~/Library/Huawei/Logs/custom-shadow-validation-20261016/`.
Local copies: `core/build-ohos/verification/custom-shadow-validation-20261016/`.
Each contains HAP, build log, source fingerprints, bundle metadata, own-process
logs, scene JSON, screenshots, owner checks and the full result summary; the
same-HAP style regression is stored beside it. Compiled canonical companion
source is checked against the copy consumed by Hvigor.

- Phone: `phone-1792138744645207000`, HAP SHA-256
  `d6681825672a52223991f91c554e07d52b8d002ec1eacc0227101469116dc125`.
- Desktop: `2in1-1792138440578966000`, HAP SHA-256
  `c5dfe1f826c584009678998ccdf72daa4d52a7eaf1a59cab8377090d323109c4`.
- Phone style: `phone-1792138824453296000`; desktop style: `2in1-1792138526485316000`.
- `host-checks/` contains local verification logs and streamed guest results.

The archived HAP hashes and all thirteen compiled-source fingerprints match the
retrieved packages/current source; both guests use identical implementation and
fixture fingerprints. Soft, hard-color and triangle screenshots were also
visually inspected after retrieval.

The qualified desktop HAP was reinstalled after phone acceptance and left
running at the soft-shadow scene, PID 3331. Its actual colored pixels, SDK helper
flags and process identity were checked without sending input. This convenience
preview is archived under `desktop-left-running/2in1-1792138984096691000` and is
excluded from the full-suite totals above.

Earlier failed runs are diagnostic evidence only. The phone geometry-only run
`phone-1792137115596979000` explicitly has zero touches and no pixel checks; it
is excluded from qualification. Earlier phone/desktop successes preceded the
final geometry-version validity comparison and are excluded from the final table.
Phone run `phone-1792138681725433000` stopped when UITest did not return a layout
before a guarded touch; the pending touch was not sent. A subsequent read-only
layout query succeeded, then the entire same-HAP suite was rerun. The aborted
run is retained under `diagnostics/`, outside the qualification totals.
The direct-SDK `window-shadow-probe` is a feasibility tool, not native API acceptance.

Host macOS checks passed: CTest 35/35 (including 14 raster checks), generated C ABI
and all 339 binding/reference outputs current, raw Dart/Rust FFI generation,
Dart analysis, Rust workspace check, C# solution build, JS build/typecheck/tests
(4 passed, 1 skipped), Python build/lint/tests (26 passed, 1 skipped), and Go
native build/tests. Seven existing generator skips remain; none is introduced.
Both ARM64 and x86_64 libraries compile into the HAP; guest runtime evidence is
ARM64 only. The generated API reference reflects the new OHOS availability note.
Other language runtimes were checked on macOS, not executed inside HarmonyOS.

## Remaining scope

Main-window custom shadow compatibility, screen edges/multiple displays, multiple
Stages/host replacement, SDK/renderer fault injection and rollback, and broader
concurrent teardown remain open. The main-window SDK shadow-suppression path
requires the normal `SET_WINDOW_TRANSPARENT` permission on supported devices;
this child checkpoint does not qualify that path. Existing default-shadow and
corner evidence does not imply every window property or lifecycle is complete.

Primary references: [official Window API](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md)
for shadow radius, focusability, touchability and geometry events;
[LocalStorage guide](https://github.com/openharmony/docs/blob/master/en/application-dev/ui/state-management/arkts-localstorage.md)
for shared page storage. API 26's installed `@ohos.window.d.ts`, compiler and actual
runtime were used to resolve version-specific behavior described above.

Follow-up: [primary-window and helper-recovery validation](ohos_primary_shadow_validation.md) extends this historical checkpoint with computer main-window pixels/defaults, phone primary refusal, and repaired helper reconstruction on both guests.
