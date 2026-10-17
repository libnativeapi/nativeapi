# HarmonyOS primary shadows and helper recovery

2026-10-16, `macbook-pro`, Apple Silicon, DevEco Studio 26.0.0.851,
HarmonyOS 7.0.0.107 / API 26 ARM64. This extends the
[custom-shadow checkpoint](ohos_custom_shadow_validation.md); the complete
[API objective](ohos_api_coverage.md) remains active.

## Actual change

The Stage companion now observes destruction of its private helper as well as
its target. Removing a surface releases both subscriptions. It also checks the
helper's native liveness before reuse because a destroy promise can settle before
the event arrives. Subsequent configuration recreates a destroyed helper.
Cleanup carries the exact old surface identity so a delayed old callback cannot
remove its replacement. Public C++/C signatures and language bindings are unchanged.

This repairs a real failure: after independently destroying the helper through
the public SDK, the previous companion retained the invalid object. The next
native C request tried to resize that dead window and failed with SDK 1300002.
Diagnostic desktop PID 12595 records `apply SDK rejected 1300002` and the failed
`helper-recreated` action. The native implementation at this diagnostic was core
`e295fe2ea`; only the fixture had been extended. The baseline log and raw result
folder are retained; its overwritten HAP is not claimed as an archived baseline.

## Runtime qualification

Both HAP lifecycle manifests and installed metadata must identify an ordinary
`normal`, non-system application. The existing SET_WINDOW_TRANSPARENT permission
is declared normally; no shell grant or system-app fixture is used. One named
guest runs at a time.

| Guest / suite | PID / UID | Native checks | Scenes | Actual input |
| --- | --- | ---: | ---: | ---: |
| Computer primary | 7918 / 20020058 | 107 | 18 | 18 clicks |
| Computer child | 10348 / 20020058 | 131 | 21 | 38 touches |
| Phone primary refusal | 3790 / 20020074 | 25 | 6 | 6 clicks |
| Phone child | 4726 / 20020074 | 131 | 21 | 38 touches |

All final suites pass with zero failures. Phone main-shadow SDK returns **801**;
its decoration setter independently succeeds with **0**. Decorator success does
not imply support for the main-shadow operation.

Independent scene JSON confirms actual helper identities change on reconstruction:
computer primary 76 → 77, computer child 81 → 82, phone child 38 → 39.
Final screenshots were also visually inspected after retrieval.

A convenience desktop rerun exposed an early observation bug in the new fixture:
after a resize promise, the old main frame and old helper briefly agreed. The
fixture accepted that pair before the new SDK frame was published, then sampled
outside the old image. The same PID 3684 later showed the expected green pixel
at (2751,1048): (53,246,54), without a new request or restart. The fixture now waits
for its explicitly requested main rectangle, and the external runner independently
checks both that rectangle and the helper's exact margin geometry. Final
qualification uses this strengthened verifier; earlier runs are excluded.

The desktop primary suite checks soft red, hard red/green, copied C++/C getters,
UI and worker execution, disable/enable, actual helper destruction/reconstruction,
native move and resize, title-bar restoration and resumption, decorated refusal
preserving the old configuration, clear, native default off/on, independent SDK
default reference and original geometry/decorations restored. It also checks
stable focus, helper input flags, primary identity and registry isolation.

Custom colors/alpha are compared with independently captured desktop pixels
at the same physical coordinates. A hard alpha-192 source must match source-over
composition within nine channel levels and differ visibly from the backdrop.
The default shadow must visibly darken at least 64 exterior pixels and its mean
channel darkening must exceed 0.5. Cleared/default-on strips must match the direct
SDK reference within eight channel levels. This qualifies exterior edge pixels
on the hidden-title-bar main window in this computer profile. Its visible native
corners remain rounded even though the SDK property reports `isRoundCorner:false`;
the current custom contour is rectangular. Exact primary corner-contour agreement
is not established by these edge samples and remains open.

The phone primary extension independently records the direct SDK main-shadow
result, then checks refused C++/C/UI/worker configurations and clear, unchanged
native state and zero helpers. It does not turn a refusal into a success or infer
rendering from it. The complete child suite remains positive on both devices:
soft/hard alpha over white, triangle/off-window clipping, copied configuration,
queued visibility, real native geometry/hide/show, helper reconstruction, defaults
and target destruction.

Each primary scene includes one real guarded click on the app's content probe.
The child suite includes two touches per visible scene, checking actual target
delivery and shadow/mask-hole pass-through to the app's main window. Every press
uses fresh own PID/UID, UITest ownership and WMS visibility evidence. No press is
sent to the desktop/wallpaper or another app; there is no keyboard or host input.

## Reproduction and archives

From `core/tests/ohos_smoke`, using the selected disposable guest:

```sh
NATIVEAPI_TEST_SUITE=window-shadow-main \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh "$DEVECO_STUDIO_HOME"
NATIVEAPI_SHADOW_GEOMETRY_ONLY=0 NATIVEAPI_TEST_SUITE=window-shadow \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh "$DEVECO_STUDIO_HOME" 127.0.0.1:5555 --skip-build
```

Remote: `~/Library/Huawei/Logs/main-shadow-validation-20261016/`.
Local: `core/build-ohos/verification/main-shadow-validation-20261016/`.
Each final archive contains both full summaries, normal-app metadata, logs,
screenshots, actual SDK scene JSON, ownership evidence, the HAP/build log and
eighteen source fingerprints. The compiled canonical companion is checked against
the Hvigor-consumed copy. Source equality and retrieved HAP hashes are verified.

- Computer: `2in1-1792140438086988000`; child run `2in1-1792140494877231000`.
  HAP SHA-256 `0c026cb44712915e9e7dbd802bc1d8c4345a00c5e8e280ca211307822fd8342e`.
- Phone: `phone-1792140731331367000`; child run `phone-1792140747485258000`.
  HAP SHA-256 `3811eabb45e17e6541aa9b78d82f69121d3f60ee061b4edaaf2a0a30eb7d89af`.
- Diagnostic baseline: `diagnostics/2in1-1792139522797422000`.
- Early resize observation: `diagnostics/2in1-1792140216582104000`, including
  the later same-process screenshot and observation values.
- `host-checks/` retains generator/lint logs, baseline companion source, baseline
  failure and streamed qualification output.

After phone acceptance the exact qualified desktop HAP was reinstalled. The
strengthened primary suite again passed 107 checks and 18 actual scenes/clicks,
PID 3408, including the resize case. This additional cold-start convenience run
is stored under `desktop-restored/2in1-1792140955035440000`, excluded from the table
totals, and the passing app is left running on NativeAPI_Desktop.

Local checks: generator verification (75 C ABI / 339 outputs current, seven
existing skips), shell parsing, Python syntax, and the new primary runner's Ruff
E/F/W/B/UP/I checks. This change is confined to the ArkTS companion and OHOS test
fixtures; it does not change or claim new language-runtime acceptance. Both HAP
native architectures compile; runtime evidence is ARM64.

## Remaining scope

Screen edges/multiple displays, primary rounded contours, multiple Stages/host
replacement, fault rollback during partial image/geometry publication, automatic
recovery without a subsequent request, and broader environment/concurrency teardown
remain open. This test establishes explicit reconstruction on the next mutation;
it does not claim that a destroyed helper silently reappears on its own.

The public [Window API](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#setwindowshadowenabled20)
documents the main-shadow permission and device restrictions. Installed API 26
declarations and independent calls establish the actual device behavior here;
helper z-level changes themselves do not request focus.
