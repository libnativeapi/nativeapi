# HarmonyOS native window occlusion checkpoint

2026-10-15, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107
(API 26), ARM64 NativeAPI_Desktop (MateBook Pro / 2in1) and NativeAPI_Phone
(Pura X View). This checkpoint adds real window occlusion queries and events;
the full API and boundary/lifetime audits remain open.

## Implementation

Window::GetOcclusionState reads the latest public SDK occlusionStateChanged
observation, with windowVisibilityChange as a registration fallback. SDK absent
and partial occlusion both map to Visible; full occlusion maps to Occluded.
The backend never infers an unobstructed window merely from show/focus or its
rectangle. A shown window without an observation remains Unknown. Actual hidden
windows and owning-Ability SDK hiding are Occluded; closed/foreign wrappers and
SDK getter failures return Unknown. Worker calls marshal to the UI environment.
IsOcclusionStateSupported checks initialized Window.SessionManager capability.

SDK subscriptions remain active for attached windows even when all public
nativeapi listeners are removed, because shown windows have no synchronous public
occlusion getter. The existing private WindowPropertyDispatch caches aggregate
state and delivers WindowOcclusionChangedEvent through object, manager and C ABI
paths. SDK partial-to-unobstructed transitions do not emit a duplicate Visible
change. Environment cleanup invalidates states and releases callback roots without
executing JavaScript; wider teardown/race scenarios remain open.

The primary source is the [public OpenHarmony Window reference](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#onocclusionstatechanged22),
consistent with installed `openharmony/ets/api/@ohos.window.d.ts` (since API 22).
SDK rules count non-main window shadows and treat transparent/animated/masked
windows according to the compositor's own calculation. This port reports that
system result, without a replacement screen-rectangle calculation.

## Actual scene verification

Two real native child windows provide twelve accepted scenes:
uncovered, partial, full, transparent, full again, uncovered, hidden, reshown,
fully covered and visible while unsubscribed, then hidden and reshown after
resubscribing. Closing retains the C++/C wrappers and verifies Unknown afterwards.
A separate direct ArkTS SDK listener supplies the reference value. Every phase
checks C++ and C ABI queries on both UI and worker threads and compares object,
manager and C event counts and event payloads/current getter values. Removing and
adding public listeners must not replay events; queries remain current while
unsubscribed. Invalid handles/tokens and pre-initialization capability are checked.

The external verifier also checks actual system window IDs/z-order/rectangles and
rendered pixels: magenta when exposed, cyan when covered, separate magenta/cyan
samples under partial coverage, and the actual blended color through a half-alpha
cyan window. After hiding, both native rows are hidden. A guarded Next button
acknowledges each scene only after all independent reads complete.

A retained early desktop run (PID 17096) shows why shadow semantics matter: target
content was fully covered by the cyan child in the screenshot while the direct SDK
reported PARTIAL_OCCLUSION and nativeapi correctly reported Visible. Disabling the
test children's shadow radius through public setWindowShadowRadius(0) produces an
unambiguous fully covered scene, which the SDK reports as FULL_OCCLUSION. This is
part of the scene setup, not a production change to the SDK's result or default
window styling. The library's wider shadow API audit remains open.

## Reproduction

From core, with exactly the selected disposable guest running and unlocked:

```sh
NATIVEAPI_TEST_SUITE=occlusion \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The harness verifies guest model/name, fresh process/UID, native window owner and
an unobstructed own button before each press. No host mouse or keyboard input is
used. Evidence is retained under `test-results/occlusion-suite/<kind-time>/` and
`~/Library/Huawei/Logs/occlusion-validation/{desktop,phone}/` on macbook-pro.
Ignored local copies: `core/build-ohos/verification/occlusion-validation/`.

## Remaining work

- Main-window occlusion from another application, primary application hide/restore
  event ordering, listener reentrancy and destruction during callbacks, pending
  environment teardown and multiple owning environments.
- Fully off-screen/display changes, rotation/folding and workspace transitions,
  irregular masks, animated overlays, shadow/radius boundaries and opacity variants.
- Actual SDK registration/getter failure and older-SDK fallback runtime scenarios.
- All remaining Window, WindowManager, shadow/shape and other APIs in the full ledger.

## Generated bindings and host checks

Public signatures are unchanged. All 75 C ABI and 339 binding/reference outputs
are current with the same eight baseline skips. Raw Rust/Dart regeneration has
no diff. Local macOS checks pass: CTest 30/30, Rust workspace, Dart analysis,
C# build (zero warnings/errors), JS 4 pass/1 skip, Python 26 pass/1 skip plus Ruff,
and Go tests (cached). The Python harnesses pass lint/format checks. Shell syntax
and PowerShell syntax (on windows-laptop) pass. Both HAP architectures compile;
actual OHOS runtime evidence is ARM64 on macbook-pro, not Windows or x86_64.

Phone final occlusion **PID 3657: 104 native assertions, zero failures**, plus all
twelve system-window/pixel scenes and retained closed-wrapper checks. The larger
count than the earlier 79-check desktop run adds explicit independent SDK showing-
state versus occlusion comparisons and owning-application visibility checks; fully
covered windows remain logically shown, and hiding a child leaves the app visible.
Phone lifecycle regression also passes: full PID 6088 **49** assertions and required
PID 6357 **25**, seven guarded actions, real background/foreground, rendered theme
pixels and actual termination. Desktop application integration regression PID 27089
passes **62** assertions and five main/child hide/restore cycles; the improved parser
handles spaces in native window names and requires both original native IDs to be
hidden. No hidden child can be omitted by the row matcher.

A phone badge regression exposed a precondition in the earlier harness: public
moveAbilityToBackground can reveal Settings from the back stack instead of Home.
The test now requires the actual own launcher icon before asserting badge absence
or presence. When necessary it restores the own fixture, verifies its fresh native
frame/PID/UID and sends a bounded Home touch gesture from that frame; it verifies
Home afterwards. It sends no touch into Settings and no keyboard input. The failed
PID 7557 evidence is retained; badge visibility is not replaced by SDK getter-only
verification.

The Settings-backstack regression now passes **PID 14260: 25 native assertions**,
with an actually executed guarded Home gesture and real launcher 9/99+/clear nodes.
The two extra assertions versus the earlier 23-check phone integration run are
real activation/deactivation state checks during the necessary restore/Home step.
Final evidence: `phone/{settings-backstack-regression.log,14260/,phone-1792077323098280000/}`.

Final computer occlusion **PID 3398: 104 native assertions, zero failures**,
with the same twelve scene checks, explicit logical-showing versus occlusion
assertions and actual native child destruction after retained-wrapper checks.
Evidence: `desktop/{complete-suite.log,2in1-1792077429642388000/}`. This and phone
PID 3657 are the final expanded fixture; desktop PID 22898 is the earlier
79-check checkpoint. The final HAP contains both arm64-v8a and x86_64 native test
libraries (recorded in `desktop/hap-architectures.json`).

Final computer general regression **PID 4766** passes **201 base + 52 picker +
5 dialog UI + 8 picker UI + 3 actual drag/maximize/restore checks = 269**,
zero failures. Evidence: `desktop/{general-regression.log,window-regression.log,general-4766/}`.
The computer emulator is left running with the real passing app.

Core checkpoint: `0de373e` on main. No changes are pushed.
