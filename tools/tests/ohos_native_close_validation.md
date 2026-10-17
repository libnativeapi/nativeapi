# HarmonyOS system close requests — 2026-10-16

The full OHOS API objective remains active. This checkpoint connects an actual
system title-bar close to the existing per-window cancellable request, instead
of covering only programmatic child Close(). Tested on macbook-pro, DevEco Studio
26.0.0.851, HarmonyOS 7.0.0.107 / API 26 ARM64.

Core implementation: `03db788`.

## Behavior

The attached-window adapter registers the public `windowWillClose` callback when
window observation first starts. The SDK expects a Promise: resolving true
prevents closing; false continues the exact SDK request. All attached C++ aliases
and C listeners share one EventRequest. No manual destroyWindow or terminateSelf
operation substitutes for an approved system close. Programmatic Close() remains
limited to child windows; Ability-owned main windows still reject that method.

A private optional rejection continuation on WindowCloseState releases the SDK
reply when any vote is denied or a listener throws. Worker decisions return to
the owning UI loop. Repeated system clicks and API requests while pending cannot
create a second vote. An installed SDK interceptor stays until native destruction:
removing every listener cannot let the next click bypass existing decisions.
With no observers and no pending request, the SDK close passes through immediately.

Destroyed windows no longer receive SDK off() calls: the SDK already disposed
its subscriptions, so cleanup releases local roots without re-entering a dead
window. The acceptance runner rejects any destroyed-window unsubscribe exception
and compares the application crash/freeze report set before and after the suite.

External native destruction prevents the pending SDK reply and invalidates its
decisions and all retained wrappers. Environment cleanup drops the reply after
detaching the environment, without executing JavaScript. Real full environment
teardown is not separately exercised here. SDK subscriptions are per-window,
not a global window-manager hook; direct host replacement/coexistence remains
unverified and must be coordinated by the host.

Primary reference: the public [Window windowWillClose API](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#onwindowwillclose15).
The installed API 26 declarations confirm its API 15 availability, main/child
scope, Promise semantics and exclusive system title-button trigger. WindowStage
createSubWindowWithOptions supplies real decorated children to the fixture.

## Actual acceptance

| Guest | PID / UID | System clicks | Recorded native assertions | Result |
| --- | --- | ---: | ---: | --- |
| NativeAPI_Desktop / 2in1 | 7001 / 20020058 | 18 | 190 before final main acceptance | 0 failed; original SDK main close ends the Ability |
| NativeAPI_Phone | 4365 / 20020074 | 0 | 175 | 0 failed; eight actual programmatic child-close scenes |

Desktop main scenes cover C++ cancel, C cancel, two independently held C++/C
decisions, worker approval followed by veto, repeated title-button clicks,
exception-as-veto and final approval. The final SDK acceptance must produce the
new native close event, remove the actual process and leave no window-service
record for its PID. Both final suites record zero new application crash/freeze reports and zero
destroyed-window unsubscribe exceptions. Earlier runs retained exceptions from
the old off-after-destruction path; final selected runs include the fix.
The 190 count is the journal snapshot before that terminal
main-window request, rather than a fabricated post-exit query.

Eight child scenes cover C++/C cancellation; worker veto/approval; removing all
object, alias, C and manager observers while votes are pending; actual SDK
destruction during a pending close followed by rejected stale worker votes;
immediate approval; and no-observer SDK closing. Every child is independently
absent from WindowManagerService before the next is created. Object/duplicate/C
request counts agree, and the manager receives no object close requests.
Nine desktop and seven phone worker completions independently require actual
successful or correctly refused EventDecision operations and log no failure.

On the phone, a real createSubWindowWithOptions request includes decorEnabled
true, but fresh native UI trees show no system close button on either main or
child windows. The eight child scenes therefore use the real programmatic API,
not an invoked/fabricated SDK close callback. This does not establish runtime
system-button interception on that phone profile. The unavailable UI path stays
explicit rather than being counted as passing native clicks.

Each click obtains fresh native UI bounds, checks the exact SDK window ID and
fixture bundle PID/UID against WindowManagerService immediately before its atomic
guest click. No host cursor or keyboard input is sent. An initial runner attempt
incorrectly compared a public NativeAPI ID with a system ID; its ownership guard
stopped before input. The corrected runner keeps both identities separately.

## Reproduction and evidence

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=native-close \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

Run with exactly one named, unlocked disposable guest. PowerShell accepts the
same selector but was not run. Remote archives are
`~/Library/Huawei/Logs/native-close-validation-20261016/{desktop-7001,phone-4365}/`.
They contain exact fixture HAP/SHA-256, production/test source snapshots, native
logs, native ownership trees, window-service dumps, screenshots and result JSON.
Local ignored copies live in `core/build-ohos/verification/native-close-validation-20261016/`.

## Host verification and remaining audits

Local macOS: core CTest 33/33, including a new real shared-request rejection/UI
fence assertion; Rust workspace; Dart analysis; C# solution (zero warnings/errors);
JS native/type/test checks (4 pass, 1 baseline skip); Python native/lint/tests
(26 pass, 1 skip); Go native build and cached tests. Public signatures and event
payloads are unchanged. Full codegen and raw Rust/Dart regeneration retain the
same 75 C ABI / 339 binding-reference outputs and eight baseline skips, with no
new skipped API. These are local binding checks; OHOS runtime checks use C++/C.
Windows/Linux/Android/iOS runtime builds were not run for this checkpoint.

Final computer general regression, PID 5943, passes 269 checks: 201 native, 52
picker, 5 dialog UI, 8 picker UI and 3 actual window movement/maximize/restore
checks, with zero failures. Full native logs contain no destroyed-window off()
error. The separate desktop-regression-5943 archive retains 86 fresh UI evidence
files and the installed HAP/SHA-256. The desktop guest remains running on this app.
Other SDK/device
versions, host-owned competing SDK callbacks, registration failure, repeated
acceptance before asynchronous SDK destruction and full environment/process
teardown remain open. This checkpoint does not close the broader window/API audit.
