# HarmonyOS live parent-window checkpoint

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26,
ARM64 NativeAPI_Desktop and NativeAPI_Phone. This checkpoint covers existing
Window.SetParentWindow / GetParentWindow; the complete API goal remains open.

## Implementation

The OHOS backend previously always refused reparenting and returned only its
creation-parent record. It now calls public setParentWindow and reads public
getParentWindow for the actual relationship, including changes made by the ArkTS
embedding host. Returned attached parents use the canonical shared Window identity.
No public signatures, C ABI symbols or non-OHOS platform implementations change.

Setter receivers must be attached non-main windows; parents must be live, attached
windows in the same UI environment. nullptr detachment, self/descendant cycles,
invalid/foreign tokens, destroyed windows and overlapping requests are refused.
Native ancestry is read before submission, and pending ancestors also refuse a
competing change. The SDK remains the authority for device/window restrictions.
The tested service rejects a repeated current-parent request with 1300009; workers
report that failure while the relationship remains intact. UI callers report
submission, and workers wait for actual SDK completion. Child destruction and
environment cleanup cancel waiting requests; broad teardown races remain untested.

The getter returns nullptr for current SDK query failure or an unattached parent.
Only SDKs without the getter retain the old creation-parent fallback. API 19/older
images and unavailable-getter fallback are not runtime-verified in this checkpoint.

Primary reference: [OpenHarmony Window API, setParentWindow and getParentWindow](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#setparentwindow19).
The installed public SDK declarations agree: these APIs are available since API 19,
new parents may be same-process main/child/floating windows, and setters depend on
device/freeform capability. No private API, system-app signing or extra permission
is used for these operations.

## Real guest verification

The ordinary dev.nativeapi.smoke fixture creates three actual native children.
Seventeen phases compare C++/C UI and worker getters with independent SDK parent
window IDs. The runner requires exactly 420 native assertions and zero failures.

Cases include initial/main parents, C++ UI changes, C worker changes, direct SDK
changes, native and externally-created ancestry cycles, UI C submission, worker
completion, same-parent SDK refusal, a second UI request while one is pending,
retry, invalid wrappers/handles, null/wrong-type handles and actual destruction.
Owning C parent handles are released on every query without affecting the native
relationship. Closed retained wrappers return no parent and refuse changes.

On the computer, changing the child's parent from A to B lets it survive actual
SDK destruction of A. Destroying B then destroys the child in the system service;
the canonical lookup and retained wrappers are invalidated. These observations
establish real ownership changes, beyond accepting a setter or caching an ID.
No synthetic mouse or keyboard input is sent by this scenario. The shared guest
fixture verifies the named emulator, live application PID/UID and bundle metadata.

| Guest | Parent acceptance | Window-manager regression |
| --- | --- | --- |
| NativeAPI_Desktop | PID 17508, UID 20020058; 420/420, 17 phases; direct SDK reparent code 0 | PID 17562; 1,188/1,188, nine successful SDK stack comparisons |
| NativeAPI_Phone | PID 3482, UID 20020074; 420/420, 17 phases; direct SDK reparent code 801 | PID 3590; 1,188/1,188, nine successful SDK stack comparisons |

Final computer archive: `~/Library/Huawei/Logs/window-parent-validation-20261016/2in1-1792128356507754000/`.
The exact HAP SHA-256 is `9c4b4412bde91f67ca4c290486231a897a24e94e99a9e8be622dc76a2cd179ae`.
Final phone archive: `~/Library/Huawei/Logs/window-parent-validation-20261016/phone-1792128455915039000/`.
Its HAP SHA-256 is `063dc44641cfb8fe7f652a9731909ee5675dee6376b284009a747084012cda09`.
The current phone profile exposes real parent getters, but the direct SDK refuses
reparenting with 801. Native worker setters also return false, and every settled
relationship stays equal to the independent SDK. The phone does not execute the
successful reparent/external-mutation/cycle-ancestry paths that require that SDK
capability; identically named phases preserve and check the real refusal instead.
No claim is made that every phone/freeform configuration must refuse it.

Preliminary runs (including the first same-parent expectation failure) are excluded
from these counts; the independent SDK comparison corrects that expectation.

## Reproduction and retained evidence

From core/tests/ohos_smoke on the remote host with one named guest running:

```sh
NATIVEAPI_OHOS_DEVICEINFO_WRITER=0 NATIVEAPI_OHOS_KEYBOARD_ACL=0 \
NATIVEAPI_TEST_SUITE=window-parent \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh /Applications/DevEco-Studio.app/Contents
```

Each archive contains the installed HAP, build log, per-phase SDK/native JSON,
ordinary-bundle metadata, real process logs, three production source snapshots
and source/HAP SHA-256 hashes. The manifest/device lifecycle is checked again
before archiving. The same installed HAP then runs the full window-manager suite.
Local ignored copies are under `core/build-ohos/verification/window-parent-validation-20261016/`.

## Downstream checks and remaining work

75 C ABI and 339 binding/reference outputs are current with the same eight baseline
skips. Raw Rust bindgen and Dart ffigen regeneration add no diff. macOS host checks
pass: CTest 33/33, Rust workspace, Dart analysis, C# build (zero warnings/errors),
JS native/type checks and 4 pass/1 skip, Python native build/lint and 26 pass/1 skip,
and Go native build/tests. These are host checks, not OHOS runtime tests of every
language. Both ARM64 and x86_64 native libraries are packaged and compile; actual
guest calls run on ARM64 and use C++/C plus independent ArkTS.
The Python runner passes Ruff and formatting; the shell runner parses.

Multiple environments/Abilities, unattached parents, old SDK behavior, service
failure, window moves/minimization after reparenting, modal focus transfer, foreign
SDK mutation during an outstanding change and teardown races remain open.
Custom shadow rendering and all other outstanding coverage-ledger items also
remain in scope. This does not close the complete API goal. No changes are pushed.

The computer guest was cold-started again and the exact archived computer HAP
was hash-verified, manifest-checked and reinstalled. PID 3329 (UID 20020058) passes
420 checks over all 17 scenes again and is left running. Extra evidence is under
`desktop-final-relaunch/` with `desktop-left-running.json`; excluded from main totals.

Core checkpoint: `ff90350fb` on main.
