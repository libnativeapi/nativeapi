# HarmonyOS native keyboard monitoring — 2026-10-16

KeyboardMonitor now delivers actual public NDK key-down/key-up events through
the C++ and C APIs. Positive verification requires an SDK system-app debug
profile on the tested images. A normal application's ACL can grant the permission
while the native monitoring service still refuses registration. This checkpoint
does not make global monitoring available to ordinary HarmonyOS applications,
and does not complete the full HarmonyOS API objective.

## SDK and permission boundary

The installed Input Kit header and the official
[NDK reference](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md)
provide AddKeyEventMonitor/RemoveKeyEventMonitor (API 12), require
ohos.permission.INPUT_MONITORING, and allow only the first native registration
in a process. The SDK permission definition and actual guest definition both
identify a system_basic/system_grant permission eligible for ACL provisioning.

The fixture uses the official [ACL debugging workflow](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/security/AccessToken/declare-permissions-in-acl.md),
[profile structure](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/security/app-provision-structure.md)
and [signing tools](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/security/hapsigntool-guidelines.md).
Only dev.nativeapi.keyboard.monitor is recreated. Its manifest declares only
INPUT_MONITORING and has a distinct NativeAPI Keyboard Monitor label. SDK sample
keys are copied into ignored output; original SDK keys/templates and personal
signing material are untouched. Debug profiles expire one day after creation and contain the
current named emulator's UDID. HAP signatures, bundle/device/permission contents,
installed app privilege and actual native results are checked independently.
These are debug fixtures, not distribution signatures or app-market approval.

## Actual permission matrix

Host: macbook-pro, Apple Silicon, DevEco Studio 26.0.0.851; HarmonyOS 7.0.0.107,
API 26, ARM64. One cold-booted guest runs at a time.

| Guest / profile | PID / UID | Self permission | Direct NDK result | Native assertions / chords |
| --- | --- | --- | --- | ---: |
| Desktop / ordinary normal app | 6906 / 20020058 | false | 201, denied | 7 / 4 |
| Desktop / normal app with ACL | 16761 / 20020062 | true | 201, denied | 7 / 4 |
| Desktop / system_basic system-app debug | 21206 / 20020062 | true | 0, success | 356 / 14 |
| Phone / ordinary normal app | 4801 / 20020074 | false | 201, denied | 7 / 4 |
| Phone / normal app with ACL | 6890 / 20020076 | true | 201, denied | 7 / 4 |
| Phone / system_basic system-app debug | 8987 / 20020076 | true | 0, success | 356 / 14 |

All final cases have zero failures. Native assertion counts exclude the repeated
finally cleanup check. Ordinary apps use dev.nativeapi.smoke; both signed variants
use the isolated keyboard namespace. Self permission comes from the independent
public OH_AT_CheckSelfPermission API. Installed bundle/token records distinguish
normal/hos_normal_app from system_basic/hos_system_app. The ordinary-ACL refusal
is an observed service boundary on these images, not evidence that ACL permission
granting failed. No root token replacement or system permission database edit is
used. The debugging permission tool also refused attempts to grant this system
permission manually; that path is not used for the successful fixture.

## Input and lifetime evidence

The human explicitly authorized only Ctrl+Alt+F9, including left/right Ctrl/Alt
variants, in NativeAPI_Phone and NativeAPI_Desktop. Every input rechecks the single
named instance, bundle, actual process/UID and native WindowManager focus. The
guest's uinput sends the bounded chord and a finally release. No host keyboard
input or other chord is sent. Ordinary/ACL-denied cases require real component
F9 input with Ctrl and Alt while all native monitor counters remain empty.

The positive cases assert the exact native press/release sequence and key codes
for two C++ monitors and a C monitor sharing the one SDK slot. Coverage includes:

- Four left/right modifier variants, concrete event type and UI-thread delivery.
- A separate SDK peer UID holding actual focus while all three monitors receive
  the input. Before any command returns source focus, the archived source log
  already contains its 18 new UI deliveries. Phone checks this delta directly;
  a read-only desktop archive audit confirms 90 deliveries: 72 from four focused
  chords plus 18 from the background chord.
- Repeated Start/Stop, one monitor stopping while others continue, final native
  unregistration, worker stop/restart, and listener removal/re-addition.
- Holding actual NDK callbacks in the test dispatcher, then replacing an epoch
  and destroying another monitor before draining: neither old C++ recipient
  delivers, while the unaffected C recipient receives the original events.
- Self-stop on F9 down suppressing queued releases, C handle release from its
  callback, and destruction of the last C++ monitor from its own callback.
- Invalid/released C handles and exactly-once user-data cleanup after the next
  UI-loop turn. The first ordinary run exposed an overly early cleanup assertion
  in the fixture; production cleanup follows the existing asynchronous contract.

Production registration/removal uses a process registry with separate operation
and callback locks. Borrowed SDK events are reduced to copied scalar fields;
delivery uses the main-thread dispatcher and guarded emitter lifetime. Each
Start has a separate active registration epoch and environment identity. Stop
invalidates queued old deliveries; failed native removal retains cleanup ownership
for a later retry. No public C++ signatures or C ABI symbols change.

The backend emits press/release events for native modifier key codes as well,
but does not manufacture ModifierKeysChangedEvent snapshots. The public native
event has no historical modifier-state payload; a later current-key query can
already reflect releases from the same injected chord.

## Regression and reproducibility

The standalone build/run commands and required authorization flag are documented
in core/tests/ohos_smoke/README.md. The verifier never treats installation success,
a profile field or a wrapper's active flag alone as event-delivery evidence.

Desktop ordinary regression PID 12396 passes 200 native, 52 picker, five dialog
UI, eight picker UI and three window UI checks (268 total). Real title-bar drag,
maximize and restore are observed on native geometry. An initial run PID 8193
timed out behind the own app's optional desktop-icon installation prompt; the
fixture-specific guarded prompt helper dismisses it before the repeated run.
This also extends that helper to the isolated keyboard app's exact distinct label.
The regression HAP precedes the final fixture's additional self-delete command;
its production keyboard implementation is identical. Final keyboard fixtures
exercise the additional command on both guests. No new full phone window/source
regression is claimed by this checkpoint.

Host checks pass: macOS CTest 33/33; Rust workspace check; Dart analysis; C# build
with zero warnings/errors; JS native build/type check/tests (4 pass, 1 baseline
skip); Python native build/lint/tests (26 pass, 1 baseline skip); Go native build
and tests. Codegen, bindgen and ffigen complete; final codegen check passes with
the same eight known skips. The website reference receives the availability and
lifetime documentation. Windows/Linux/Android/iOS runtime verification and
language-binding runtime use inside OHOS are not claimed.

Evidence is archived under
`~/Library/Huawei/Logs/keyboard-monitor-validation-20261016/` on macbook-pro and
locally under ignored `core/build-ohos/verification/keyboard-monitor-validation-20261016/`.
Final directories are desktop-system-21206, desktop-normal-acl-16761,
desktop-ordinary-6906, phone-system-8987, phone-normal-acl-6890,
phone-ordinary-4801 and desktop-regression-12396. They retain signed profiles/HAPs
where applicable, signature verification logs, bundle/token records, per-command
journals, real focus/owner captures, background logs and source hashes. Earlier
desktop-system-32443 is preliminary and excluded from final counts.

Modifier-state parity, cancellation/repeat and other actual key inputs, multiple
physical keyboards, live permission revocation, service loss, removal failures,
environment teardown/reinitialization and remaining binding/runtime audits stay
open. The full API ledger remains active.
