# HarmonyOS native window polygon masks

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26.
The ARM64 guests are NativeAPI_Desktop (MateBook Pro / 2in1, density about 1.9)
and NativeAPI_Phone (Pura X View, density 3). This is a checkpoint in the active
full API audit, not completion of Window or the overall goal.

## Implemented behavior

SetShape now sends a real public SDK compositor mask for an initialized host's
undecorated native child window. The polygon is copied on entry, closed implicitly,
and rasterized at physical pixel centers with the specified even-odd rule. The
SDK alpha-mask method receives a rooted Uint8Array; a compatibility path constructs
the older nested-array mask. Clearing calls clearWindowMask rather than repainting
a rectangular fake view. Mask changes preserve the native window ID and bounds.

The shared native-window record changes only after SDK success. C++ and C aliases
agree; direct SDK edits intentionally do not rewrite IsShaped. A UI call reports
submission, a worker waits for actual completion, and a pending operation rejects
overlap. Failed operations preserve the prior record. Closed and foreign windows
return safe defaults. Window invalidation cancels pending requests and releases
their roots; broad teardown/race coverage remains open.

The complete inverse requires API 24+. API 26's alpha path is bounded to 16 Mi
physical pixels and the legacy nested-array path to 1 Mi; neither dimension may
exceed 16384 physical pixels. Main windows are rejected. The direct valid-size SDK
main-window call independently returns 1300004 on both tested device types.
No additional permission or debug shell permission grant is needed for masking.

The SDK applies the mask to both drawing and touch hit testing. This does not
implement SetInputShape, which promises independent input clipping without changing
drawing. The new setTouchableAreas API accepts at most ten rectangles; an adapter
for that separate API remains an open investigation, not a claim that every possible
input-area operation is impossible.

Primary source: [public Window mask and clear APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#setwindowmask12).
The installed API 26 public declaration additionally specifies
setWindowMaskWithAlpha(Uint8Array, width, height).

## Actual acceptance

The fixture creates a magenta native child over its own white parent. An independent
geometric oracle checks actual compositor pixels, and guarded guest touches must
produce the expected callback in the child or underlying parent. Each press checks
fresh native layout, window-service identity, process and UID ownership. No host
input or keyboard input is sent.

Thirteen scenes cover an unconfigured clear rejection, a copied triangle and pending
clear rejection, repeated rectangle restoration, a concave polygon, even-odd
self-intersection, an off-window polygon making the entire child transparent, a
full rectangle, resizing and reapplication, and an independent legacy SDK mask and
clear. The C builder also checks fractional points, invalid/nonfinite coordinates,
out-of-range reads, reuse/clear and the 4096-vertex boundary. C++/C queries are checked
at every phase, and retained wrappers reject mutations after actual WMS destruction.

Computer **PID 32207: 56 native assertions, 13 rendered scenes, 702 compositor pixel
samples and 26 real guarded touch probes, zero failures**.

Phone **PID 10887: 56 native assertions, the same 13 scenes, 702 sampled pixels
and 26 real guarded touch probes, zero failures**. Its early resize request also
returns false without changing state, and the retry succeeds.

A resize promise can resolve before either NDK or public SDK window properties
reflect the new rectangle. Earlier runs (PIDs 24692 and 28260) correctly exposed a
mask-size rejection at this transition; their failure evidence is retained. The
final suite explicitly checks the early request's result against IsShaped, waits
for the new SDK rectangle, then reapplies successfully. On the final computer run
the first request returns false and leaves the unshaped record intact; its retry
renders and routes touches correctly. The backend does not hide an SDK refusal or
promise an implicit retry. Public availability notes document the required wait.

## Reproduction and retained evidence

With exactly one named, unlocked disposable guest running, from core:

```sh
NATIVEAPI_TEST_SUITE=window-shape \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

Guest-only evidence is retained remotely under
`~/Library/Huawei/Logs/window-shape-validation/{desktop,phone}/` and copied locally
to ignored `core/build-ohos/verification/window-shape-validation/`. It includes native
logs, result JSON, screenshots, per-pixel oracles, WMS/layout dumps and touch-owner
records. Computer final result directory: `2in1-1792081602252567000`; phone:
`phone-1792081778949659000`.

Phone corner/shadow regression **PID 14552** also passes 50 native assertions and
14 real child scenes after the new mask implementation. Computer style regression
**PID 4387** passes 60 assertions and 19 rendered child/main scenes. Computer
general/window regression **PID 6560** passes 201 base assertions, 52 picker
assertions, 5 dialog UI checks, 8 picker UI checks and 3 actual title-bar
drag/maximize/restore checks: **269 passed, zero failures**. Its normal result
window remains visible in NativeAPI_Desktop.

Local SDK-independent raster verification checks 1268 samples/validation cases,
including concavity, fractional scales, self-intersection, repeated/degenerate
vertices, clipping and memory/size limits. macOS CTest passes 31/31. The full
generator and raw Rust/Dart FFI steps run successfully; codegen check reports 75
C ABI and 339 binding/reference files current, with the same eight baseline skips.
Local Rust, Dart, C#, JS, Python and Go checks pass. JS reports 4 passed/1 skipped;
Python reports 26 passed/1 skipped. These are local binding checks, not claims of
runtime acceptance of every language binding inside HarmonyOS.

The HAP builds both ARM64 and x86_64 libraries; actual runtime acceptance here is
ARM64 only. The extended PowerShell runner is not executed or parser-checked in
this checkpoint (Windows host offline, no local PowerShell). API 24/25 fallback
runtime, other device versions, main-window mask support, isolated input polygons,
custom shadows and the broader window/event/teardown/binding audits remain open.
