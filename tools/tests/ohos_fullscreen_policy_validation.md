# HarmonyOS fullscreen-mode policy

2026-10-16, `macbook-pro`, DevEco Studio 26.0.0.851, HarmonyOS product 7.0.0.107 / API 26
ARM64 (computer OS fullname OpenHarmony-7.0.0.105). The [full API objective](ohos_api_coverage.md) remains active.

## Implementation

SetFullScreenable replaces the empty backend with real SDK supported-mode
requests; IsFullScreenable replaces its unconditional live-wrapper result with
an established mode policy. FULL_SCREEN is toggled while the other known mode
bits are retained. The SDK couples immersive fullscreen and nonimmersive
maximize: disabling FULL_SCREEN prevents both, including programmatic requests.
This is separate from the existing caption-button policies and phone immersive
content-layout/system-bar behavior.

The Window method is public in the installed API 26 declarations. If absent on
an attached primary window, the backend uses the public API 15+ WindowStage
method, after validating the Stage's actual main-window ID. Unknown/malformed
methods fail without publishing policy. UI calls are serialized across aliases;
workers wait for their own SDK result. SDK errors keep the last policy, and
native destruction/environment cleanup invalidates queued requests.

There is no public live mode-mask getter. The public attached
UIAbilityContext.abilityInfo.supportWindowModes gives the primary declaration.
A floating child establishes FLOATING without guessing SPLIT or an unobserved
floating capability for an already-fullscreen child. A resulting empty/invalid
mask is refused rather than silently enabling a different mode. Unknown child
policy returns false until a successful request or host-published mask establishes
it; false does not prove native incapability.

The independent SDK probe confirms that AbilityInfo stays [0,1,2] after actual
SDK overrides to FLOATING only. Hosts with higher-priority StartOptions or direct
SDK overrides must publish their known mask through the private
SetWindowModeBaseline seam. This calls no SDK setter, validates enum values and
window combinations, rejects duplicates, and refuses to overwrite pending work.
Unreported host changes are outside the getter's observations. See the
[host integration](../../core/src/platform/ohos/README.md).

Reference: installed public `@ohos.window.d.ts` and
`bundleManager/AbilityInfo.d.ts`, and the [WindowStage API](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-WindowStage.md#setsupportedwindowmodes15).
No public C++ signature, C ABI entry point or language export changed.

## Runtime verification

Both profiles use ordinary normal, non-system bundles, with no extra permissions
requested or shell-granted. Exactly one named guest runs. Mode-policy,
fullscreen and SDK-reference suites send no synthetic input. The title
regression retains its guarded guest-only real gestures; no host input or keys
are sent.

| Guest / suite | PID / UID | Checks | Scenes |
| --- | --- | ---: | ---: |
| Computer mode policy | 14619 / 20020058 | 716 | 43 |
| Computer fullscreen regression | 8840 / 20020058 | 517 | 31 |
| Computer title regression | 10843 / 20020058 | 318 | 31, 20 real double-clicks |
| Phone mode policy | 4545 / 20020074 | 716 | 43 |
| Phone fullscreen/layout regression | 7410 / 20020074 | 517 | 31 |
| Phone title regression | 9368 / 20020074 | 318 | 31, 20 real double-clicks |

Each mode suite makes 27 public policy calls and eight fullscreen calls through
C++/C on UI/workers. Exact SDK argument masks are captured by an adapter that
delegates to the original SDK methods; normal outcomes are unmodified. Public
SDK status/properties, native logical bounds and independent WMS frames agree.
Property event identities, UI delivery and object/manager/C counts agree.

Computer evidence includes actual fullscreen refusal while disabled, successful
reenablement, ordinary-child fullscreen enablement, three-request cross-alias
queues, repeated-value event deduplication and real Stage fallback. A host
FLOATING-only override remains without SPLIT after enabling/disabling fullscreen:
actual SDK masks are 5 and 4. FULL_SCREEN-only removal is refused with unchanged
native fullscreen geometry, policy and SDK-call trace. Invalid/duplicate masks,
empty/foreign/released wrappers and actual native closure are covered.

Computer property event counts are main [12,12,12], eligible child [7,7,7] and
ordinary child [3,3,3]. Phone counts remain zero: actual SDK mode requests reject
with 801, so primary declared policy stays true and unknown child policies stay
false. Phone content-layout behavior is independently retained by the fullscreen
regression, rather than being reported as an enlarged native child window.

## In-flight destruction and late SDK outcomes

A test adapter holds only delivery of the real SDK result. The first child
request actually reaches the SDK; desktop applies FLOATING, while phone reports
its real 801 refusal. Three UI requests and one C worker then remain pending in
one window's queue. A competing host baseline is refused. The desktop's direct
SDK fullscreen request independently rejects with 1300004 while the public
policy still reflects its last completed request.

Actual SDK child destruction clears all four requests and releases the waiting
worker, whose subsequent query reports false. The three queued follow-ups never
reach the SDK. Releasing the original late success (computer) or rejection
(phone) does not change policy, masks, events, pending counts or SDK-call trace.
The archive checks these late snapshots independently. This establishes native
window teardown; a real in-flight environment teardown remains open.

## Reproduction and artifacts

From core with one unlocked named guest:

```sh
NATIVEAPI_TEST_SUITE=window-mode-policy \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

Run window-fullscreen, window-title and window-mode-probe with the same HAP using
`--skip-build` for the remaining gates. The independent 28-scene SDK probe records
ordinary-child upgrades, original rectangles, normal-app input permission
results, mode restrictions, phone layout flags and unchanged declarations.
It is separate from nativeapi acceptance. The forced Window-method-absence
case validates the actual Stage path on API 26, not runtime on an older image.

Remote: `~/Library/Huawei/Logs/mode-policy-validation-20261016/`.
Local ignored copy: `core/build-ohos/verification/mode-policy-validation-20261016/`.

- Phone final: `phone-1792153098018403000`, SDK probe PID 12657.
  HAP SHA-256 `7017d5359c9da72360b899b93881f365232087c94f112b02849d5d49dfc01b10`.
- Computer final: `2in1-1792154162574384000`, SDK probe PID 8076.
  HAP SHA-256 `6fffef8862ce8cf38882d87ebfd4519b52e25c8d28c774437cb8e5ab1835fea2`.

Each final archive contains the matched HAP/build log, all four suites, normal
installed metadata, native logs, phase JSON, WMS rows, trees, decoded screenshots
and 30 source fingerprints with source copies. Retrieved HAP and source hashes
are compared with local sources. Earlier computer checkpoints (682/41, then
716/43 before the final Stage identity guard) are retained separately and do not
replace final-source verification. The first 41-scene HAP was overwritten;
only its diagnostic results/driver log are claimed. No unavailable binary is
claimed for that checkpoint.

The final handover checks the live computer PID/UID, owned passed-page text,
visible primary WMS row, current HAP hash and a fresh decoded guest screenshot.
The phone is stopped; NativeAPI_Desktop remains running.

## Other checks and remaining scope

Local macOS CTest passes 35/35. Rust, Dart, C#, JS, Python and Go host checks pass;
JS/Python each retain one existing skip. Go tests run without the result cache.
Codegen/raw Rust/Dart refresh and codegen check pass (75 C ABI and 339
binding/reference outputs, seven existing skips and no new omissions).
Availability documentation regenerates the website reference; changelogs record
the new SDK policy and unknown-child semantics. Only ARM64 guest runtime is
claimed; HAPs also contain compiled x86_64 native libraries.

Unreported external mode changes, real in-flight environment teardown, missing
or malformed metadata/fault boundaries, multiple Stages/Abilities, follow-parent
and other SDK window variants, old images and remaining Window/API audits stay
open. The private host publication contract cannot infer a foreign caller's
unreported StartOptions. This checkpoint does not finish the full API objective.

## Follow-up: immersive layout completion

The later [content-layout checkpoint](ohos_window_layout_validation.md) fixes
worker completion semantics and pending native-window cleanup for
SetContentUnderTitleBar, independently distinguishes layout flags from native
fullscreen/caption geometry, and reruns both fullscreen and mode-policy
regressions. Its final handover replaces this checkpoint's historical page.
