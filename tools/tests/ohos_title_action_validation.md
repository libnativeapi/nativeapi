# HarmonyOS custom title-bar actions

2026-10-16, `macbook-pro`, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 /
API 26 ARM64. The [full API objective](ohos_api_coverage.md) remains active.

## Implementation

PerformTitleBarDoubleClick replaces its false stub with public SDK maximize
(EXIT_IMMERSIVE) from floating mode and recover from maximized/split-screen mode.
The public API 22+ isInFreeWindowMode query must establish live workspace
support; query refusal/false never falls back. Only SDKs that lack the query use
a 2in1 device fallback. An applied disabled maximize policy,
fullscreen/minimized/unknown modes, native
destruction and a pending earlier request reject the action. UI callers report
promise submission without blocking; worker callers wait for the real SDK result.
SDK failures do not fabricate a window transition. A shared per-window request
protects C++/C aliases against overlapping actions, and native/environment cleanup
invalidates pending work. Public signatures and the other five platforms are unchanged.

The host explicitly forwards a custom component's real double-tap/double-click
callback. This API does not change the native system title-bar gesture policy.
The [preceding caption-only limitation](ohos_window_policy_validation.md) still
applies to the system's own double-click.

Eligible SDK children are included. Direct public SDK measurements establish that
`maximizeSupported: true` enables actual child maximize/recover on this computer,
while its individual title-button setter still rejects with 1300004. An unavailable
caption-control API is therefore not treated as an applied disabled policy.
Ordinary child maximize is independently rejected with 1300004. A real successful
main-window SetMaximizable(false) still blocks this custom action.

Primary references: [Window maximize, recover, modes and child options](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md)
and the installed public `@ohos.window.d.ts`. SDK source inspection additionally
confirms that ordinary-child maximize can throw before creating its promise;
other SDK rejections may arrive asynchronously.

## Runtime evidence

Both profiles use ordinary `normal`, non-system bundles. No permission is
shell-granted and no system-app variant is used. Exactly one named guest runs.

| Guest / suite | PID / UID | Native checks | Scenes | Actual guest input |
| --- | --- | ---: | ---: | --- |
| Computer custom title | 4493 / 20020058 | 318 | 31 | 20 custom double-clicks; native split menu |
| Computer caption regression | 8196 / 20020058 | 131 | 32 | One system maximize click; two native title double-clicks |
| Phone custom title | 9430 / 20020074 | 318 | 31 | 20 custom double-clicks; refusal paths |
| Phone caption regression | 12829 / 20020074 | 124 | 32 | No system-title input on this profile |

The custom-title suite exercises C++/C on UI/workers for main and eligible child
maximize/recovery. SDK properties, density-adjusted C++/C bounds and independently
parsed WMS rectangles must agree. Object, manager and C resize events agree on
counts, window identity and UI delivery. Accepted transitions require changed
real geometry and additional resize events. Refused gestures require unchanged
rectangles, status and event counts.

Computer scenes cover disabled policies in floating and already-maximized modes,
actual fullscreen/minimized rejection, three immediate cross-alias overlap
attempts, ordinary floating-child refusal, native child destruction, retained
closed wrappers, empty/foreign objects and released C handles. The minimized main
scene invokes UI and worker operations directly while invisible; its normal Want
foregrounds the Ability before Restore. No independent foregrounding claim is
made for Restore alone.

For the split scene, a guarded hover opens the actual native maximize menu and
an owned menu item selects the left side. A normal Want foregrounds only the
test Ability and dismisses the partner picker.
The next scene independently requires a
fresh public SDK status **5** before the custom double-click, then a C-worker
recover result, status **4**, the original floating rectangle and real WMS/event
agreement. This is independent of the SDK-only split exploration.

The phone follows the same component gesture route. Its actual fullscreen main
and unsupported child operation paths must reject without a fake successful
transition. It does not exercise a real minimized/fullscreen-transition setup or
native split menu on this profile; those desktop setup operations are skipped.
All child wrappers are tested after actual SDK destruction.

## Reproduction and retained diagnostics

From core with one unlocked named guest:

```sh
NATIVEAPI_TEST_SUITE=window-title \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
NATIVEAPI_TEST_SUITE=window-policies \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
NATIVEAPI_TEST_SUITE=window-title-probe \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
```

The seven-scene SDK probe independently records child maximize, recover, caption
buttons, decorations and destruction. It sends no input and is not nativeapi
acceptance. An earlier computer native suite passed 299 checks/29 scenes before
adding the real split case; it remains a separate earlier checkpoint.

Phone PID 4138 disproved using a fulfilled promise as capability evidence: the
eligible-looking child's SDK maximize returned 0 but retained floating status 4
and an unchanged 800×500 rectangle, with `The device is not supported` in its
native log. SDK recover returned 801. The initial native helper incorrectly
returned true in that case, and the fixture failed rather than counting a window
transition. The implementation now checks the public live free-window capability
before submitting. Independent SDK snapshots retain that false capability and
the real no-op reference result. This fix preserves capable desktop child actions.

Initial fixture diagnostics are retained separately. A duplicated snapshot
exceeded the system log's message limit, so the driver could not parse its JSON;
the final log protocol avoids repeated copies. Another run used a never-shown
child's zero rectangle as a recovery reference, then correctly observed recovery
to its real measured 800×500 frame. Children are now shown and measured before
capturing references. Ordinary-child refusal is exercised while actually floating,
so a minimized-state guard cannot hide unsupported-type handling. Intermediate
ArkTS compilation failures are not counted as passing runtime runs.

All input checks fresh process/UID, owned native roots and positive WMS visibility.
A popup can overlap its own parent; every root covering an injected point must
belong to the tested app. No host input or keyboard keys are injected. Normal
metadata, scoped logs, input ownership, trees, frame snapshots and decoded
screenshots are retained.

Remote: `~/Library/Huawei/Logs/title-action-validation-20261016/`.
Local ignored copy: `core/build-ohos/verification/title-action-validation-20261016/`.
Final source-matched profile archives:

- Computer: `2in1-1792148826962723000`, SDK probe PID 11206.
  HAP SHA-256 `c134c49b3039fff29900d232c3874255f311bf8e2c8c6157f912b0dfe69a8782`.
- Phone: `phone-1792148350085474000`.
  HAP SHA-256 `b82aaf31c9c292fafadca8b7e9917d07af619be404acb20362d95b57972697e1`.

Computer calls record 16 accepted real transitions out of 26 API calls; the phone
records zero accepted requests. All 40 custom double-clicks route through the
actual test components. The independent SDK probe observes live free-window
support on all six live computer snapshots and false on all six phone snapshots.

Each final profile archive contains the matched HAP/build log, all three suites,
normal installed metadata, direct SDK observations and 25 source fingerprints.
The HAP contains ARM64 and x86_64 native libraries; only ARM64 runtime is claimed.
Retrieved HAP hashes and local/remote source equality are verified.

The same computer HAP was then rerun with no rebuild to leave the acceptance
page open: PID 11989 / UID 20020058, again 318 checks, 31 scenes, 20 real custom
double-clicks and 16 accepted transitions with zero failures. Its complete evidence
is retained in the final computer archive's `restored-title-suite/`. The phone
is stopped and NativeAPI_Desktop remains running; the actual process identity was
checked after the suite completed.

## Other verification and remaining scope

Local macOS CTest passes 35/35. Rust, Dart, C#, JS, Python and Go checks pass;
JS has one existing skip and Python one existing skip. Go tests were also run
without the test-result cache. Generation and raw Rust/Dart refresh pass; codegen
check confirms 75 C ABI and 339 binding/reference outputs, seven existing skips
and no new omissions. No public signature or language-binding export changes
were required; the two added NAPI exports belong only to the fixture.

Other SDK/window variants, forced service failures, real in-flight environment
teardown, independent windows/multiple Abilities, remaining Window actions and
the wider API/binding audits stay open. This checkpoint does not establish
complete native user-action restrictions or finish the full objective.
