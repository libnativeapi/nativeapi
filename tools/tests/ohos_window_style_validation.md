# HarmonyOS window corner and shadow checkpoint

2026-10-15–16, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26.
Runtime guests: ARM64 NativeAPI_Desktop (MateBook Pro / 2in1) and NativeAPI_Phone
(Pura X View). This is another checkpoint; the complete API audit remains open.

## Changes

- Child corner preferences now call public setWindowCornerRadius. Square uses
  0 vp, RoundSmall 4 vp and Round 8 vp. Default restores the actual radius read
  before the first library request; it does not resume a changing system policy.
  Main windows reject the operation, matching the direct SDK's 1300004 result.
- A shared native-window record updates only on real SDK success. UI callers
  receive submission status, while worker callers wait for completion. A pending
  corner request rejects overlap. Native destruction and environment cleanup
  release pending workers; wider race/error scenarios remain open.
- Child shadow toggling uses the correct public setWindowShadowRadius: 0 disables
  and 18 logical pixels enables. The prior backend used the main-window-only API
  for child windows, which could not implement this operation correctly.
- Main shadows use setWindowShadowEnabled and a normally declared
  SET_WINDOW_TRANSPARENT system_grant permission on supported PCs/tablets. There
  is no shell grant or runtime authorization prompt. Missing-permission SDK error
  201 is independently recorded before the permission is declared.
- The SDK has no existing-shadow getter. HasShadow reports the last successful
  library request, false until configured; SDK failure preserves the record.
  Direct SDK edits do not rewrite that record, just as direct radius edits do
  not rewrite GetCornerPreference. No exact default shadow radius/color/offset
  or custom core-managed shadow is claimed.
- The existing application promise request was moved into a shared private
  helper used by application and corner operations. N-API roots, actual SDK
  completion and cleanup behavior remain explicit. Public signatures are unchanged.

Primary references: [OpenHarmony corner/shadow APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#setwindowcornerradius17)
and [normal system-grant permission definition](https://github.com/openharmony/docs/blob/master/en/application-dev/security/AccessToken/permissions-for-all.md#ohospermissionset_window_transparent).
The installed public SDK declarations agree. The fixture's manifest requests the
permission normally; it does not modify its source manifest in the remote mirror.

## Actual verification

A real native child is rendered magenta above an explicitly white parent. Each
corner scene compares independent SDK getWindowCornerRadius readings and actual
compositor corner-mask pixels. The same SDK window ID and rectangle must survive
all style changes. Tests cover C++/C aliases, UI/worker calls, invalid enums/tokens,
main-window refusal, overlapping requests, direct SDK edits, original-radius
restoration, closed retained wrappers and actual system-service destruction.

Child shadow on/off images are compared with direct SDK reference calls. Enabled
shadows must visibly darken at least 64 exterior pixels and have measurable mean
darkening. Native and reference exterior regions must match within four channel
levels. Main-window on/off and SDK reference scenes use the same independent
pixel comparison after normal permission declaration. No successful setter or
cached flag alone proves a rendered style.

First computer no-permission **PID 21353: 50 native assertions, zero failures**,
seven corner-mask checks, six shadow scenes and final radius restoration. The
original radius was independently read as 16 vp at density about 1.9. Direct main
corner mutation returns 1300004; direct main shadow mutation returns 201.

Computer with declared permission **PID 30905: 60 native assertions, zero failures**.
The SDK main shadow call returns 0. Five additional real main-window shadow/reference
scenes pass, including exterior darkening, stable geometry and restored shadow state.

## Reproduction and evidence

From core, with the selected disposable guest running and unlocked:

```sh
NATIVEAPI_TEST_SUITE=window-style \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The read-only-import fixture helper checks the exact guest, fixture PID/UID and
native ownership before input. Next advances a scene only after system-window and
pixel verification. No host cursor or keyboard input is used. Remote evidence:
`~/Library/Huawei/Logs/window-style-validation/{desktop,phone}/`; ignored local
copies: `core/build-ohos/verification/window-style-validation/`.
Computer initial/permitted evidence is under
`desktop/{2in1-1792078553488100000,2in1-1792078969618114000}/`.

## Remaining work

- SDK failure/refusal and retry, default-radius changes through theme/density,
  multiple environments, rapid mutation/teardown and callbacks during destruction.
- Existing externally configured shadow state cannot be queried with this SDK;
  custom shadow color/offset/blur support and core-rendered shadows remain open.
- All remaining window, shape, input, positioning and other APIs in the coverage
  ledger remain in scope. These results do not close the complete goal.

Phone **PID 3952: 50 native assertions, zero failures**, including all fourteen
child style scenes, corner-mask pixels and independent SDK shadow references.
The original corner radius is 0 vp at density 3; Default correctly restores square
corners on this profile. Direct main corner mutation returns 1300004. Main shadow
mutation returns **801**, even with the normal permission declared, consistent
with the public permission/device documentation. Child shadows require no such
permission and visibly render on the phone. Final evidence:
`phone/{initial-suite.log,phone-1792079598075348000/}`.

## Generated bindings and host checks

Public signatures are unchanged. All 75 C ABI and 339 binding/reference outputs
are current with the same eight baseline skips. Raw Rust/Dart regeneration has
no diff. Local macOS checks pass: CTest 30/30, Rust workspace, Dart analysis,
C# build (zero warnings/errors), JS 4 pass/1 skip, Python 26 pass/1 skip plus Ruff,
and Go tests (cached). The new Python helper/runner pass lint and formatting; the
shell runner parses. Current PowerShell-runner parser verification is unavailable
because windows-laptop is offline (SSH timeout), and local pwsh is not installed.
Actual OHOS acceptance runs on macbook-pro ARM64; no Windows or x86_64 runtime
acceptance is claimed. Both native HAP architectures compile.

Final current-source computer style **PID 5754: 60 native assertions**, with all
fourteen child scenes and five main shadow/reference scenes passing. Evidence:
`desktop/{final-complete-suite.log,2in1-1792080111855594000/}`. The packaged HAP
contains both ARM64 and x86_64 native libraries and the normal declared permission;
`desktop/hap-manifest-and-architectures.json` records the actual archive contents.

Shared-request regression passes computer application integration **PID 6925: 62**
assertions and five real main/child hide/restore cycles; phone application integration
**PID 7917: 23**, actual launcher badges and SDK references; phone occlusion
**PID 8973: 104** with all twelve native scenes. Final computer general regression
**PID 8148: 201 base + 52 picker + 5 dialog UI + 8 picker UI + 3 actual window
checks = 269**, zero failures. The computer guest is left running with the real
passing acceptance app. Final regression evidence is retained alongside style
results, including `desktop/general-8148/` and each suite's timestamped directory.

Core implementation checkpoint: `c22ec1365` on main. No changes are pushed.
