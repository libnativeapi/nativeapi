# HarmonyOS independent caption-button policies

2026-10-16, `macbook-pro`, Apple Silicon, DevEco Studio 26.0.0.851,
HarmonyOS 7.0.0.107 / API 26 ARM64. This extends the
[collective-control checkpoint](ohos_window_controls_validation.md).
The [complete API objective](ohos_api_coverage.md) remains active.

## Implemented scope

SetMinimizable, SetMaximizable and SetClosable now apply independent native
caption-button choices through public setWindowTitleButtonVisible. Collective
visibility and title-bar changes preserve the individual choices. Getters retain
successful library applications, independently of compositor layout timing;
SDK failures preserve the prior policies. Desktop primary defaults are enabled
at attachment. Other window profiles report false until a button request succeeds,
and destroyed/empty wrappers report false. Individual SDK button getters do not
exist, so direct embedding-host edits do not rewrite these retained policies.
The collective getter continues to query actual native button area.

The changes preserve programmatic minimize/maximize. Primary Close() retains
its existing unsupported result; actual programmatic child closure still works,
including when the child has no caption support. No public signatures changed.
Public availability notes and generated API reference describe the narrower
OHOS semantics. Other platform implementations are unchanged.

**This is caption-only support, not complete user-action denial.** Real title-bar
double-click still maximizes after SetMaximizable(false). Other system entry
points are unchanged. No complete maximize-policy claim is made by these tests.

Primary references: [Window title-button APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md)
and [WindowStage supported modes](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-WindowStage.md).
The installed SDK declarations were inspected. Supported-mode restrictions also
restrict programmatic maximize, as observed in the earlier independent probe;
they are not used to emulate a user-only restriction.

## Actual runtime qualification

Both installed bundles must be ordinary `normal`, non-system applications.
The fixture uses no shell-granted permissions and runs exactly one named guest.

| Guest / suite | PID / UID | Native checks | Scenes | Actual guest input |
| --- | --- | ---: | ---: | --- |
| Computer caption policies | 11723 / 20020058 | 131 | 32 | One maximize-button click; two double-clicks |
| Computer collective regression | 14697 / 20020058 | 61 | 18 | Eighteen guarded app acknowledgements |
| Phone caption refusal | 3860 / 20020074 | 124 | 32 | No title input; title buttons unsupported |
| Phone collective regression | 6405 / 20020074 | 59 | 18 | Eighteen guarded app acknowledgements |

The policy suite checks C++/C aliases, UI/worker mutations, repeated calls,
independent choices while collectively hidden, title-style restoration, direct
host SDK changes, and empty/closed wrappers. Three property event paths
(object, manager and C) agree on computer counts [3,4,3]; phone counts remain
[0,0,0]. Fresh native trees verify exact button identities and real WMS records
verify window ownership, identity and visibility. SDK results are 0 for computer
main controls, 801 for phone main controls and 1300004 for ordinary children.

The computer suite separately restores the floating rectangle after the
max-hidden double-click, then verifies programmatic maximize with that caption
policy still disabled. Programmatic minimize produces SDK status 3 and actual
WMS invisibility despite the minimize button being disabled. A normal Want
foregrounds the Ability before Restore; this does not prove Restore alone can
foreground an arbitrary background Ability. Programmatic child close removes
its native window while the primary remains alive after its refused Close.

Every press checks the isolated guest name, current app PID/UID, fresh UITest
ownership and same-process visible WMS window. No host input or keyboard keys
are injected. Screenshots are decoded and retained alongside quantitative
assertions. Phone scenes skip programmatic main maximize/minimize and title
input; unchanged phone geometry is not counted as successful desktop behavior.

## Failures retained and corrected scope

The initial lazy policy discovery let the compositor's first usable button
layout change default policy getters after the shared property store was seeded.
The first min-off phase consequently reported event counts [0,1,1] instead of
[1,0,0]. Stable desktop-primary defaults at attachment fix that ordering bug;
completed scenes verify exact transitions rather than treating discovery as an
application mutation.

A 28-scene run passed 117 native checks but did not exercise native double-click.
A strengthened run then disproved complete user-maximize denial: PID 4316 logged
`Disabled maximize policy allowed title double click`. This is retained as a
failed full-policy expectation. The final suite deliberately qualifies only
caption-button control and records the continuing gesture behavior; it does not
convert that failure into a successful denial claim. An earlier driver run also
misclassified the successful UITest text `No Error`; exact success parsing fixed
the test harness. These investigations are not final acceptance runs.

## Reproduction and evidence

From core, with exactly one named unlocked guest:

```sh
NATIVEAPI_TEST_SUITE=window-policies \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
NATIVEAPI_TEST_SUITE=window-controls \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
```

Remote archives: `~/Library/Huawei/Logs/window-policy-validation-20261016/`.
Local ignored copies: `core/build-ohos/verification/window-policy-validation-20261016/`.
Final computer: `2in1-1792143429842054000`. Final phone:
`phone-1792143750390848000`. Earlier desktop investigations are under `diagnostics/`.

After phone acceptance, the archived computer HAP (SHA-256
`97ef0de3b30f2ceb72dfa57c383bf96f5ee486b1f98f609c51f450dc70980f28`)
was reinstalled on NativeAPI_Desktop. Fresh PID 3939 passed all 131 checks,
32 scenes and three title gestures again. The computer is left on its passed
acceptance page. The repetition is retained under
`restored-2in1-1792144048430952000/`, referring to that same archived HAP.

Each final archive retains the matched HAP/build log, both complete suites,
normal bundle metadata and twenty source fingerprints. The HAP contains ARM64
and x86_64 native libraries; only ARM64 runs are claimed. Retrieved hashes and
local/remote source equality are verified. Earlier diagnostic folders are retained
separately; their overwritten HAPs are not claimed as archived baseline binaries.

## Other checks and remaining work

Local macOS core build and CTest pass 35/35. Rust, Dart, C#, JS, Python and Go
binding checks pass. JS has one existing skip and Python one existing skip.
Generation and raw Rust/Dart regeneration pass; codegen check confirms 75 C ABI
files and 339 binding/reference outputs, with seven existing generation skips
and no new omissions. No Windows/Linux/Android/iOS runtime acceptance is claimed.

Complete user-action restrictions, SetFullScreenable, supported decorated child
variants, older SDKs, fullscreen hover/system menus, custom title-bar actions,
failures during teardown and embedding-host policy changes remain open. This
checkpoint does not close the full window audit or the other API modules.

Follow-up: [custom title-bar actions](ohos_title_action_validation.md) implements
PerformTitleBarDoubleClick for explicit host gestures, including eligible children
and SDK split recovery. It does not change this checkpoint's native-system
caption double-click limitation.
