# HarmonyOS notifications checkpoint — 2026-10-15

This adds the public SDK notification backend and verifies normal behavior on
phone and computer guests. The [full API audit](ohos_api_coverage.md) remains open.
Public signatures and the existing generated event payload are unchanged.
Implementation and runnable tests are committed in core **`f2a491f`**.

## Implementation

`NotificationManager` uses `@ohos.notificationManager` and public immutable
WantAgents targeting the attached UIAbility. Initialize obtains normal system
authorization when needed. Show and Remove wait for the actual SDK publish/cancel
callback; submitting a request is not treated as success. Failures retain SDK
code/message in GetLastError. These synchronous operations require a worker while
the ArkTS UI thread continues handling callbacks; an already initialized manager
can return immediately from Initialize.

Each tag is a label under a reserved nativeapi notification ID. The same ID/label
pair replaces an existing notification; Remove addresses that pair without a
hash collision or an in-memory ID map. Tags contain 1-192 UTF-8 bytes without NUL.
Titles/messages must be nonempty; SDK display-length limits apply. Body and optional
button intents differ and emit `tag=<percent-encoded tag>&action=open` or
`action=button` through the existing main dispatcher and generated C event bridge.

The host forwards the original Want from both onCreate and onNewWant via the
internal `ohos::HandleNotificationWant` bridge. Pre-initialization cold Wants are
queued (up to 64), so applications subscribe before Initialize. Treat activation
arguments as untrusted launch input. Shutdown detaches delivery without deleting
notifications; shared registration state and per-operation environment hooks avoid
retaining the manager in SDK callbacks and invalidate registration at teardown.

Primary references: the public
[notificationManager API](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-notification-kit/js-apis-notificationManager.md)
documents authorization, publication and cancellation; the
[WantAgent notification guide](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/notification/notification-with-wantagent.md)
documents notification/body/button intents. Declarations were also checked against
the installed API 26 SDK, including current actionType/actionFlags properties.

## Real runtime evidence

Both isolated ARM64 guests run on **macbook-pro**, DevEco Studio **26.0.0.851**,
HarmonyOS **7.0.0.107 / API 26**. Only one guest runs at a time.

| Guest | Successful process | Results |
| --- | --- | --- |
| NativeAPI_Desktop / MateBook Pro / 2in1 | 4185 | 21 native checks, actual action-button and body interactions; zero failures |
| NativeAPI_Phone | 8720 | 21 native checks, actual action-button and body interactions; zero failures |

Assertions cover SDK capability and C ABI support, unregistered calls, explicit
UI-thread rejection, worker authorization, idempotent initialization, tag/content
validation, publication/replacement, independent C ABI publication/removal,
diagnostic clearing, C++/C event payloads and UI-thread delivery, shutdown,
reinitialization, retained-notification removal and final system-record cleanup.
The actual SDK active-notification list must contain exactly the updated fixture
record with the expected Unicode message and action button. It must be empty of
fixture notifications after cleanup. No assertion relies only on a cached setter.

The system authorization prompts were accepted for the disposable test HAP on
both guests. The permission UI is an extension process; guards bind its precise
process name and UID to the installed notification-dialog package. Each guest
gesture checks the named instance, model, fresh test PID, current compositor
window/PID/Z-order and unique ownership at the press point. Notification targets
must be system-rendered cards for dev.nativeapi.smoke with the expected tag and
title. Desktop opens its native notification-center control; phone swipes from
the current native status-bar clock bounds. No host mouse/keyboard or guest key
input is used. Only known fixture tags are cleaned; no global notification clear.

Use the independent group after building/installing for the current guest:

```sh
NATIVEAPI_TEST_SUITE=notification ./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

It is separate from the default window/service/dialog/picker group because system
notification-center interactions change foreground focus. A normal-path group
passing does not establish all notification or HarmonyOS behavior.

The phone base regression then passed **188 integration + 5 dialog UI checks**
in process **8823**, zero failures. It did not invoke the still-open phone
multi-file/folder picker paths. Clipboard callback assertions can coalesce, so
the aggregate integration count can vary across runs.

The final computer regression in process **4744** passed **201 integration,
52 picker, 5 dialog UI, 8 picker UI and 3 desktop-window UI checks**, zero failures.
The window assertions compare actual compositor geometry after title-bar dragging,
maximize and restore. The passing fixture and computer guest remain open.

Evidence is archived on macbook-pro under
`~/Library/Huawei/Logs/notification-validation/{phone,desktop-final}/`, including exact
PIDs, native reports, guarded ownership/geometry snapshots, interaction logs,
build logs and HAP SHA-256. Local copies are under ignored
`core/build-ohos/verification/notifications/`.

## Cold launch, refusal and retention verification — 2026-10-15

Both guests now additionally pass two real cold launches and explicit authorization
refusal using the extended fixture in core **`f824492`**. Each cold-launch case
below passed **5 preparation + 5 recovery assertions**. The
preparation process publishes through the real SDK, calls Shutdown, and queries the
active system record before any removal. The harness binds its PID/UID to the
installed fixture, force-stops it, and requires no running fixture process at each
notification press. Only the system notification click launches the recovery
process; no launch command or simulated activation argument is used for recovery.

| Guest | Body: stopped PID → new PID | Button: stopped PID → new PID | Refusal PID | Normal group repeated |
| --- | --- | --- | --- | --- |
| NativeAPI_Desktop | 13959 → 16914 | 17152 → 17266 | 26455: 5 passed | 28700: 21 passed |
| NativeAPI_Phone | 4183 → 4348 | 4569 → 4685 | 5236: 5 passed | 5578: 21 passed |

Recovery must observe the original system Want in onCreate before Initialize,
subscribe first, initialize through SDK completion, and receive the exact queued
argument once in both C++ and C ABI listeners on the UI thread. Body uses
`tag=cold_open&action=open`; the button uses `tag=cold_button&action=button`.
The button recovery initializes through the C ABI. A later SDK query confirms
cleanup. These runs independently establish Shutdown retention before Remove and
tag-based operation across actual process replacement. Each guest therefore passed
**25 additional lifecycle/refusal checks**, alongside the repeated 21 normal checks.

Refusal uses a freshly built `dev.nativeapi.notificationdenied<nonce>` package with
its own sandbox and UID. The fixture first verifies authorization is disabled.
The guarded system dialog's actual **不允许** button is clicked; Initialize returns
false with **1600004: Notification disabled**, identical through the owned C ABI
diagnostic. Publication remains disabled and the SDK setting remains false.
Successful runs uninstall only this temporary package. The original fixture's
stored data and authorization are retained.

On the computer, a first-install desktop-shortcut prompt can precede authorization.
The successful runner verifies the temporary fixture's name and native SystemUI
ownership and declines that shortcut first. An earlier trial blocked by that
prompt did not establish refusal and is excluded from the results above. Every
repeat uses a fresh package identity and verifies the initial permission state.

Run in the configured SDK shell:

```sh
NATIVEAPI_TEST_SUITE=notification-lifecycle ./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
python3 tests/ohos_smoke/run_notification_denial.py /Applications/DevEco-Studio.app/Contents
```

The denial build replaces the output HAP, so it invalidates the normal build marker.
Normal runners additionally inspect the actual HAP package and phone/computer
lifecycle before installing, including with skip-build. Real denial and wrong-device
HAPs were confirmed to fail these guards before installation. Both normal packages
were rebuilt and their notification groups repeated afterward.

The phone base regression also passed **189 integration + 5 dialog UI checks** in
PID **5841**, zero failures; it did not invoke the unresolved phone picker paths.
The final rebuilt computer fixture, PID **4013**, passed **201 integration,
52 picker, 5 dialog UI, 8 picker UI and 3 desktop-window UI checks**, zero failures.
The actual native window moved, maximized and restored as expected; the passing
computer fixture remains open.
Evidence and exact process manifests are archived at
`~/Library/Huawei/Logs/notification-lifecycle/{desktop,phone}/` on macbook-pro and
ignored `core/build-ohos/verification/notification-lifecycle/` locally.

No production signatures or language binding files changed for this extension.
The read-only codegen check confirms all 75 C ABI and 339 binding/reference files
current. Python/Bash syntax and the changed PowerShell runner's parser checks pass;
Windows runtime execution remains untested.

## Binding and host checks

Code generation and read-only checks confirmed 75 C ABI and 339 binding/reference
files current, with the same eight baseline skipped symbols and none new. Rust
bindgen and Dart ffigen ran; raw ABI files were unchanged. Local macOS checks
passed: Rust workspace check; Dart analysis; C# solution build (zero errors or
warnings); JS typecheck/tests (4 passed, 1 skipped); Python lint/tests (26 passed,
1 skipped); Go tests (cached pass). These are host regressions, not runtime coverage
of every generated language on HarmonyOS. The updated Windows runner passed
PowerShell parsing on windows-laptop; Windows emulator execution was not tested.

## Open verification

Environment teardown and concurrent Shutdown during pending SDK work, Unicode/tag
boundary cases and SDK truncation/rate limits remain separate open assertions.
Both guests now build the current backend including the defensive host-change and
Shutdown guards; their exceptional teardown/concurrency branches are not claimed
as runtime verified. Phone file-picker multiple selection and folder confirmation,
and the other modules in the coverage ledger, also remain open.
