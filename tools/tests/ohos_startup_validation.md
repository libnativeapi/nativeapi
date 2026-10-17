# HarmonyOS launch-at-login status — 2026-10-16

LaunchAtLogin now reads actual current-bundle startup status through the public
API 21+ autoStartupManager SDK, rather than returning a constant false. The normal
public self-service module has queries only: programmatic registration is still
unavailable, so IsSupported, Enable and Disable return false. The user controls
startup in system Settings. This is a status-query checkpoint, not full startup
control or completion of the HarmonyOS objective.

## Public SDK and semantics

The installed SDK exposes getAutoStartupStatusForSelf (Promise<boolean>, API 21)
and isAutoStartupSupported (API 26). The official
[OpenHarmony self-status reference](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-ability-kit/js-apis-app-ability-autoStartupManager.md)
documents the same surface. A search of all installed SDK API declarations also
finds enterprise applicationManager auto-start mutators, which require an
activated EnterpriseAdminExtensionAbility and ENTERPRISE_MANAGE_APPLICATION.
They are not ordinary self-service registration APIs and are not silently used
by this backend. No hidden system API or startup database writes are used.

The SDK query is asynchronous. Workers wait for its real completion; UI calls
request a refresh and return the last completed result for that environment,
initially false, without pumping or blocking the UI loop. SDK failure invalidates
the cached value. A private environment generation invalidates snapshots after
teardown/reinitialization, including reused napi_env addresses. Full live
environment replacement and failure injection remain runtime audits.
The shared private SdkRequest gains an optional checked result decoder; its old
operation-only users retain their previous behavior. Boolean false is a successful
SDK result, not a request failure.

Default identifiers and labels resolve from actual AppInfo metadata, including
objects constructed before runtime initialization. Explicit foreign/empty IDs
cannot query or impersonate the current bundle. Display-name setters record
local metadata; system UI continues to use the bundle label. Arbitrary executable
paths/arguments are refused and remain empty. IsSupported describes programmatic
registration, so its false value does not prohibit explicit IsEnabled reads.
No public C++ signatures or C ABI symbols change.

## Real guests and native assertions

macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26, ARM64:

| Guest | PID / UID | SDK device capability / initial state | Main rounds / assertions | Result |
| --- | --- | --- | ---: | --- |
| NativeAPI_Desktop / 2in1 | 20109 / 20020058 | true / false | 7 / 175 | zero failures; four real Settings transitions |
| NativeAPI_Phone | 3582 / 20020074 | false / false | 3 / 75 | zero failures; direct SDK query succeeds with false |

Before the desktop scenario, a direct SDK probe (PID 12625) observed disabled
status. The actual Settings startup list was initially empty. Adding only
NativeAPI Smoke through the system application selector made the direct SDK and
C++/C getters return true, including fresh application processes (PID 21464,
then 26894). The final principal desktop scenario (PID 20109) retains one process and the same
C++/C objects across add/remove/add/remove operations; every state agrees with
independent direct ArkTS SDK reads before and after the native checks. Each
state has 25 assertions. Finally cleanup verifies the original disabled state in another round. A
preliminary process (PID 26973) also records an explicit original-state restoration:
uiBefore and cUiBefore are true, while fresh worker results and completed UI
snapshots become false. Its restoration is archived separately. The final
reported 175 excludes its extra 25-assertion finally round. The task's original
disabled setting is restored.

Every round covers pre-initialization refusal, default/explicit/foreign/empty
identities, actual package metadata, fresh and retained C++/C SDK queries, UI
cached snapshots after worker completion, local display-name metadata, all
unsupported startup mutators and executable/argument requests, and released
C handles. Native work uses asynchronous N-API work and returns to UI; it does
not join a worker that needs the UI thread. Empty C ABI strings follow the
existing to_c_str contract (nullptr), and empty argument lists have count zero.
The page independently re-reads SDK status after native mutation attempts to
prove no fake local enable/disable success.

The phone profile explicitly reports isAutoStartupSupported=false. Its direct
getAutoStartupStatusForSelf still succeeds with false (code 0), and native reads
match on all three rounds. This is not evidence of a positive phone startup
setting or actual boot-time launch. Older images, real hardware, actual login/
boot launch, parameter/error/teardown and wider binding runtime audits remain open.

## Input-monitor permission boundary and fixture repair

Each round independently calls public NDK OH_Input_AddKeyEventMonitor in the
ordinary fixture. Both guests return INPUT_PERMISSION_DENIED (201). An unexpected
success would immediately remove that probe and fail the assertion. No keyboard
input is generated by this suite. The primary
[restricted-permission specification](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/security/AccessToken/restricted-permissions.md)
classifies INPUT_MONITORING as system_basic/system_grant, available to normal
applications through ACL approval. This proves the ordinary-profile refusal,
not that KeyboardMonitor is impossible. Its permission-enabled backend, ACL
fixture and actual event delivery remain implementation/verification work.

The smoke module also had an export-count regression: four source/close
functions were described but only two were exported. It now derives the count
from the descriptor array. The startup page verifies sourceState/sourceBegin
are actual functions. The production drag backend is unchanged. A separate real drag regression is
run after the export-presence check; its observed result is recorded below.

## Guards, reproduction and archives

Normal read-only acceptance:

```sh
NATIVEAPI_TEST_SUITE=startup ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

After the fixtures are built and installed, the desktop Settings scenario is:

```sh
python run_startup_suite.py "$DEVECO_STUDIO_HOME" --settings-transitions
```

It restores its initial state in finally. This task additionally removes the
entry created during the preliminary positive-state probe, restoring the task's
original disabled setting. Phone does not run the desktop-only list controller.
The optional --expected true/false makes a desired SDK state an explicit gate.
The Settings adapter locates the exact fixture label and its associated control,
checks fresh UI bounds/overlap, actual Settings PID/UID and WindowManager focus,
then sends only a named-guest click. All native UI trees, owner records and state
pairs are retained. No host cursor, host keys or guest key injection are used.

Remote archives: ~/Library/Huawei/Logs/startup-validation-20261016/.
Local ignored copies: core/build-ohos/verification/startup-validation-20261016/.
The desktop-20109 and phone-3582 directories contain source/HAP hashes, the SDK
API declaration, actual native logs, SDK/native state pairs, UI screenshots,
and desktop Settings ownership/trees. After copying back, source snapshots and
HAP SHA-256 are checked. The phone snapshot predates documentation-only expansion of display-name
availability into six-platform notes and a stricter uniqueness guard in the
desktop-only Settings adapter. The final desktop run executes that stricter
guard. All production implementation/runtime/request snapshots match this
checkpoint; the latest desktop archive also matches the public header and
fixture scripts. Earlier failing prototype output is not acceptance:
its C ABI empty-string expectation was corrected against the generator's actual
nullptr contract before the successful rounds above.

## Host verification

macOS CTest passes 33/33. Rust workspace check, Dart analysis, C# solution build
(0 warnings/errors), JS native/type build/tests (4 passed, 1 baseline skip), Python
native/lint/tests (26 passed, 1 baseline skip), and Go native build/tests pass.
Codegen emits 75 C ABI and 339 binding/reference files with the same eight baseline
skips; raw bindgen/ffigen introduce no API change, and codegen check passes.
These host checks do not prove OHOS runtime delivery in every language binding.
Windows/Linux/Android/iOS runtime and the PowerShell runner were not run here.

Final computer regression PID **16664** passes **268 checks**: 200 native
integration + 52 native picker + 8 picker UI + 5 dialog UI + 3 real window
movement/maximize/restore checks. Its unique successful native labels exactly
match the previous 201-check baseline; one duplicate clipboard notification is
coalesced. The initial process 8951 had passed native integration/picker checks,
but its focused-window-only UI verifier observed Settings instead of the test
app. The fix reads the test process's owned native UI tree and verifies the
hosted modal picker is gone. A live read-only replay with Settings still focused
passes; the complete rerun then passes. Desktop gesture verification explicitly
activates the same completed fixture process before owner-guarded input.

Export repair regression: computer PID **20025** passes **211 native assertions**
over **27 real guest touch/mouse drags**, 56 commands, zero failures. This includes
C++/C sources, SDK-reference data, actual bytes, custom previews, refusal/cancel,
self/worker destruction and native mouse gestures. Its archive is
`desktop-source-20025`; the ordinary regression is `desktop-regression-16664`.
No keyboard input is sent. The phone export presence is checked by its startup
page; a new full phone drag scenario was not run in this checkpoint.
The computer guest and the final passing startup app remain running.

Core checkpoint: `288443e75`. Workspace sync includes the core pointer, generated
API reference, binding changelogs and validation evidence. No commits are pushed.
