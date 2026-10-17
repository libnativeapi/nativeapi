# HarmonyOS native fullscreen children

2026-10-16, `macbook-pro`, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26
ARM64. The [complete API objective](ohos_api_coverage.md) remains active.

## Implementation and scope

SetFullScreen previously used native immersive maximize/recover only for a 2in1
primary window. Every child instead received content-layout/system-bar operations,
which do not enter the child's supported native fullscreen mode. Eligible freeform
children now use the real SDK mode transition and recover to their previous
maximized or floating state. IsFullScreen uses actual SDK native status in this
workspace. Object/manager/C enter/exit events report confirmed native changes.

A shared private free-window query is also used by custom title actions. The API
22+ public query must return true; false/errors never use a device fallback. Only
SDKs lacking the method use the existing 2in1 fallback. Desktop-main behavior is
preserved. Non-freeform phone content-layout/system-bar requests retain their
SDK restrictions. Void setters submit asynchronously and do not claim success
from request submission. Public signatures and other platform implementations
are unchanged; availability documentation now describes their actual backends.

References: the installed public `@ohos.window.d.ts` and
[Window APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md).

## Runtime acceptance

Both fixtures are ordinary `normal`, non-system apps, with no extra permissions
requested or shell-granted. Exactly one named guest is running. The fullscreen
runner sends no synthetic input; normal Wants acknowledge scenes only for its
own Ability. The separate title-action regression sends guarded input only inside
the named guest and owned app roots, as described in its [validation](ohos_title_action_validation.md).

| Guest / suite | PID / UID | Native checks | Scenes |
| --- | --- | ---: | ---: |
| Computer fullscreen | 7856 / 20020058 | 517 | 31 |
| Computer title regression | 4183 / 20020058 | 318 | 31, including 20 real custom double-clicks |
| Phone fullscreen/layout | 20776 / 20020074 | 517 | 31 |
| Phone title regression | 17054 / 20020074 | 318 | 31, including 20 real custom double-clicks |

Each final fullscreen suite performs 22 native API calls. The independent SDK
probe adds 21 reference scenes per profile, outside these assertion totals.

The fullscreen suite uses real main, eligible SDK child (`maximizeSupported:
true`) and ordinary SDK child windows. Its 31 scenes cover C++/C UI/workers,
repeated true/false requests without duplicate events, floating recovery,
restoring a previously maximized child, actual direct SDK entry/exit and ordinary
child rejection on the computer, explicit SDK mode restriction/restoration and
actual SDK child
destruction, retained closed wrappers, empty/foreign wrappers and released C
handles. Every scene checks SDK status/properties, density-adjusted native bounds
and independently parsed WMS rectangles. Computer native fullscreen enters at
the screen origin with actual enlarged
geometry; recovery returns to the original rectangle. Phone content-layout
transitions explicitly require unchanged child rectangles.

All three enter/exit event paths agree on counts, IDs and UI delivery. Computer
main counts are [2 enters, 2 exits], eligible child [5,5], ordinary child [0,0].
Phone main counts are [2,2], eligible child [4,4] and ordinary child [1,0].
The ordinary child is actually destroyed while its layout flag is set; retained
wrappers become false and no synthetic exit is emitted. Phone children preserve
content-layout semantics: both eligible and ordinary
children can report SDK fullscreen/layout flags with status 4 and unchanged
800×500 / 300×180 frames. Independent direct SDK layout entry/exit returns 0
and confirms those exact flag/rectangle changes. The original fullscreen fixture
incorrectly expected refusal, then incorrectly expected native status 1 for this
layout transition; both failures are retained. Its phone oracle now explicitly
requires status 4 and unchanged child frames, while comparing SDK/C++/C flags and
all three real event paths. The production phone behavior was preserved.
Phone main content/system-bar behavior is also checked against SDK properties.
HAPs include ARM64
and x86_64 native libraries; only ARM64 runtime is claimed.

## Independent capability evidence

The twenty-one-scene SDK-only mode probe is separate from nativeapi acceptance.
On the computer, removing FULL_SCREEN from supported modes also prevents
nonimmersive maximize: main calls reject with 1300002 and child calls with
1300004. Restoring FULL_SCREEN allows actual native status 1 and child recovery
to its original 800×500 rectangle. A repeated recover on an already-floating
window returns 1300001. These observations do not implement SetFullScreenable:
mode-mask preservation and its coupled maximize behavior remain open.

The public mouse monitor requires INPUT_MONITORING and a screen-recording
scenario, according to the installed NDK header and
[Input Kit API reference](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md#oh_input_addmouseeventmonitor).
The computer probe (PID 2313 at the earlier checkpoint) and phone probe (PID
6187 during layout investigation) both observe self-permission false and actual
mouse-monitor registration result 201. C++/C primary and pointer-only session
starts all return false and remain inactive. Final probe PIDs are recorded with
the final archives below; no screen-recording scenario was started.
This is an ordinary-app denial checkpoint, not a claim that recording or
privileged contexts are impossible. A component's local mouse stream cannot
establish WindowDragSession's global release/retarget contract. Native
StartDragging remains a separately implemented operation. The misleading stub
comment that HarmonyOS has no shared desktop has been corrected.

## Reproduction and evidence

From core with one unlocked named guest:

```sh
NATIVEAPI_TEST_SUITE=window-fullscreen \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
NATIVEAPI_TEST_SUITE=window-title \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
NATIVEAPI_TEST_SUITE=window-mode-probe \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
```

Remote: `~/Library/Huawei/Logs/fullscreen-validation-20261016/`.
Local ignored copy: `core/build-ohos/verification/fullscreen-validation-20261016/`.

- Phone final: `phone-1792150684498873000`, SDK probe PID 16979.
  HAP SHA-256 `1cff5498accbb2f03b5613afa07ac633b1372ceb887632479bcc865c1b03ae84`.
- Computer final: `2in1-1792150944203024000`, SDK probe PID 3512.
  HAP SHA-256 `38daa088a4f1e846f6197f39bfa1908c5c39b25b76f0f2e888c288910381de55`.

The final desktop handover additionally checks the live PID/UID, the actual
owned tree's `Fullscreen acceptance passed` text, visible primary WMS row,
current HAP hash and a fresh decoded guest screenshot. The phone is stopped;
NativeAPI_Desktop remains running on the passed fullscreen acceptance page.

Archives retain normal installed metadata, process/UID identity, native logs,
phase JSON, independent WMS frame snapshots, trees, decoded screenshots, the
matched HAP/build log and 25 source fingerprints with source copies. Retrieved
HAP hashes and local/remote source equality are verified for final runs.

An initial mode probe attempted recover while the main window was already
floating, so the SDK rejected with 1300001 before the scenes began. The fixture
now checks status before initial normalization. This diagnostic is retained and
is not counted as passing runtime acceptance. Intermediate successful computer
runs (452 checks/27 scenes before repeated-request/stale-handle additions, then
517/31) remain separate checkpoints. An intermediate phone run passed 518 checks
with 23 native calls, including an extra native exit in the phase named as an
SDK reference. That branch now always invokes the SDK's recover operation;
its actual phone refusal is retained, and the final native-call count is 22.
A final rebuild after documentation updates
provides the final source-matched evidence. Failed/earlier driver diagnostics
are retained separately; no unavailable original binary is claimed.

## Other checks and remaining scope

Local macOS CTest passes 35/35; Rust, Dart, C#, JS, Python and Go checks pass.
JS/Python each retain one existing skip; Go uses an uncached test run. Generation,
raw Rust/Dart refresh and codegen check pass (75 C ABI, 339 binding/reference
outputs, seven existing skips, no new omissions). Public availability changes
regenerate the website reference; no binding export changes are required.

SetFullScreenable mode policy, serialized conflicting fullscreen requests,
restoration failure/rollback, native destruction while fullscreen or in flight,
recording/privileged drag-session environments, multi-display/secondary Ability
variants and the wider remaining API/binding audits stay open. This checkpoint
does not finish the full HarmonyOS API objective.

## Follow-up: supported-mode policy

The later [fullscreen-mode policy checkpoint](ohos_fullscreen_policy_validation.md)
implements SetFullScreenable and serialized supported-mode requests, verifies
native destruction with four queued requests and late real SDK outcomes, and
reruns both fullscreen and title regressions on final sources. This does not
close conflicting fullscreen-transition serialization or the other remaining
audits listed above. Its handover replaces this checkpoint's historical page.
