# HarmonyOS immersive content layout completion

2026-10-16, macbook-pro Apple Silicon, DevEco Studio 26.0.0.851,
HarmonyOS product 7.0.0.107 / API 26, ARM64. The
[full API objective](ohos_api_coverage.md) remains active.

## Change and platform semantics

SetContentUnderTitleBar retains the existing OHOS public SDK immersive-layout
mapping, but replaces submission-only handling with an owned SDK request.
Workers await their actual completion and verify the NDK isLayoutFullScreen
value matches the requested value. A fulfilled SDK request with a different
live flag returns false. UI calls report submission and must observe the live
getter. Repeated requests whose live value already matches remain successful.

One request is active per native window. Other C++/C aliases refuse overlapping
requests without calling the SDK. A synchronous SDK throw, non-promise result or
rejection releases the guard. Actual native destruction cancels pending work,
wakes a waiting worker and prevents late completions from refreshing state or
emitting fullscreen events. No public signature, C ABI symbol or language export
changes. The existing private SdkRequest implementation is reused.

This is a limited platform mapping, not a transparent native-caption overlay.
The installed public SDK declares immersive status/navigation-bar layout; its
WindowProperties documentation also limits the main-window layout flag to
fullscreen mode. In the tested floating computer main window, direct SDK calls
fulfill while the live flag stays false; the worker wrapper now returns false
for that enabling request. Child flags can become true, but their decorated
70-physical-pixel drawable inset remains. None of these layout requests enlarges
the native frame or changes its fullscreen/maximized mode.

On the phone, the independent SDK and library both change all three layout
flags. The main ArkUI content expands from its status/navigation-bar avoidance
area to the full window, then returns to the exact original layout. Native
frame, drawableRect and window mode remain unchanged; the main ArkUI safe-area
layout is independently read from the real component tree. System bars stay
visible, isFullScreen stays false, and all object/manager/C fullscreen event
counts remain zero on both profiles. SDK layout flags are not misreported as
native fullscreen entry or caption removal.

Primary sources: installed public @ohos.window.d.ts (WindowProperties and
setWindowLayoutFullScreen), public window_manager/oh_window.h and the
[official Window reference](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md).
The capability/other-mode, older-image and transparent-caption audits stay open.

## Runtime acceptance

Exactly one named emulator runs, with an ordinary normal non-system test bundle,
no new permissions or shell grants. Layout and both regression suites inject no
input. Guest-only guarded unlock helpers are separate; no host input or keys
are sent.

| Guest | Suite | PID / UID | Native checks / scenes |
| --- | --- | --- | ---: |
| NativeAPI_Phone | layout | 10594 / 20020074 | 947 / 46 |
| NativeAPI_Phone | fullscreen regression | 13436 / 20020074 | 517 / 31 |
| NativeAPI_Phone | mode policy regression | 15406 / 20020074 | 716 / 43 |
| NativeAPI_Desktop | layout | 8571 / 20020058 | 947 / 46 |
| NativeAPI_Desktop | fullscreen regression | 3642 / 20020058 | 517 / 31 |
| NativeAPI_Desktop | mode policy regression | 5686 / 20020058 | 716 / 43 |

Each layout suite schedules 31 C++/C UI/worker operations, plus explicit empty,
foreign, zero and released-handle checks. The 34 traced method attempts include
six direct SDK operations and four explicitly controlled faults (two throws,
two non-promise returns); those four are not real service failures. Six additional
unmodified SDK enable/disable calls establish the independent reference before
tracing/listening. Normal trace adapters delegate the original SDK method and
preserve its outcome. Frame/content geometry agrees with independent SDK fields
and WMS rows; native logical coordinates use each window's actual density.

Actual component origBounds prove primary layout. The phone's visible bounds
can be clipped by its overlapping test children; the verifier retains those
visible bounds separately instead of treating occlusion clipping as a layout
origin. Baseline, enable and disable are captured independently. Computer
primary component geometry remains identical across the no-op request.

Holding delivery of an actual successful child SDK result keeps exactly one
request pending. UI and worker attempts through other aliases return false and
add no SDK call. Releasing the result permits a successful retry. A second held
request runs on a C worker; actual SDK child destruction clears the pending
request and returns false to the worker, whose subsequent getter is false.
Releasing the late real success changes no policies, pending states, call trace
or event counts. Closed/invalid wrappers remain inert. Real in-flight environment
teardown, arbitrary service rejection, arbitration across fullscreen/layout
APIs and reentrant listener audits remain open.

## Reproduction and artifacts

```sh
source ~/Library/Huawei/ohos-env.sh
NATIVEAPI_TEST_SUITE=window-layout \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

Run window-fullscreen and window-mode-policy with the same built HAP and
`--skip-build` for the remaining gates. Normal launch Wants acknowledge each
phase. No assertions depend on input timing or a screenshot's appearance.

Remote archive: ~/Library/Huawei/Logs/layout-validation-20261016/.
Local ignored copy: core/build-ohos/verification/layout-validation-20261016/.

- Phone final: phone-1792155924777317000.
  HAP SHA-256 `9c158b2bfcaaab54db2a7ab417f68c87198d0d35c02c5366a95f590e18ca85ce`.
  Main ArkUI layout: [0,117,1320,2148] → [0,0,1320,2232] → original.
- Computer final: 2in1-1792156249965977000.
  HAP SHA-256 `9c4bd0b698dc03a7ecf0db0c88309f4cf0f806df9a2b1c66cf2b2a8c380e2dfe`.
  Main ArkUI origBounds: [515,281,2605,1675] throughout.

Final archives contain each matched HAP/build log, all three suite journals,
normal installed metadata, native logs, SDK/property snapshots, WMS rows,
component trees, decoded screenshots and 33 source fingerprints with source
copies. Local/remote source and retrieved HAP hashes are compared. The final
handover checks live computer PID/UID, its own passed-page text, a visible primary
WMS row and current HAP hash, then decodes a fresh guest screenshot. Phone is
stopped and NativeAPI_Desktop remains running.

Earlier desktop PID 1528 and phone PID 4389 each passed 947/46 before the added
component-origin assertions. They remain diagnostic checkpoints, not substitutes
for final verification. Initial driver/path mistakes and the phone occlusion-
clipped-bounds assertion are retained as diagnostics and not counted as passing
acceptance. The first desktop main-enable assertion also exposed the SDK acknowledgment
with unchanged live layout; an independent SDK reference and worker live-result validation now
preserve this difference instead of claiming positive computer-main layout.

## Other checks and remaining scope

Local macOS CTest: 35/35. Rust, Dart, C#, JS, Python and Go checks pass; C# has zero
warnings/errors, JS/Python each retain one existing skip and Go uses uncached
tests. Codegen, raw Rust/Dart refresh and final codegen check pass (75 C ABI,
339 binding/reference outputs, seven existing skips, no new omissions). Public
availability comments regenerate the website reference; changelogs record the
changed completion/lifetime semantics. HAPs compile ARM64 and x86_64; only ARM64
guest runtime is claimed. Other platform source implementations are unchanged.

The full remaining window, input, picker, environment/binding-runtime and other
module audits remain in scope. This checkpoint does not complete the full goal.
