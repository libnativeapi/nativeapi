# HarmonyOS URL dispatch checkpoint — 2026-10-15

UrlOpener now uses the public SDK on HarmonyOS. This checkpoint verifies normal
HTTP/HTTPS dispatch on both guests; it does not close the
[full API audit](ohos_api_coverage.md).
The implementation and runnable fixture are committed in core **`cd82103`**.

## Implementation and host requirements

The public signatures are unchanged. Attach the UIAbilityContext through the OHOS
runtime before querying support. `CanOpen` uses `bundleManager.canOpenLink` after
scheme validation and SDK URL parsing. The host module must declare
`querySchemes: ["http", "https"]`; invalid links, undeclared schemes and missing
handlers return false. Leading/trailing whitespace and scheme case are normalized;
embedded NUL/control characters are rejected.

`Open` invokes `UIAbilityContext.startAbility` with the URL and the public
`ohos.want.action.viewData` action. Production code does not name a browser bundle.
The synchronous native method requires a worker, keeping the ArkTS UI thread free
to process the SDK completion callback. UI-thread calls fail explicitly. Success
means that the system completed launch dispatch, not that a network page loaded.
SDK invocation errors preserve their code/message in the existing invocation-error
result. The callback session owns its state independently of the singleton;
environment cleanup wakes a waiting worker with a failure.

Primary references: public
[UIAbilityContext startAbility documentation](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-ability-kit/js-apis-inner-application-uiAbilityContext.md)
and [bundleManager canOpenLink documentation](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-ability-kit/js-apis-bundleManager.md),
cross-checked against the installed API 26 SDK declarations. The startAbility
callback reports success with BusinessError code zero; submitting a request alone
is not treated as completion.

## Runtime evidence

Both disposable guests run on **macbook-pro**, DevEco Studio **26.0.0.851**,
HarmonyOS **7.0.0.107 / API 26**, ARM64. Only one guest runs at a time.

| Guest | Fresh processes: C++ HTTPS, C HTTP, C++ HTTP, C HTTPS | Results |
| --- | --- | --- |
| NativeAPI_Phone | 26231, 26368, 26761, 27199 | 44 native + 12 guarded UI checks, zero failures |
| NativeAPI_Desktop / MateBook Pro / 2in1 | 4579, 4736, 5006, 5193 | 44 native + 12 guarded UI checks, zero failures |

Each process checks initialized/uninitialized capabilities, real HTTP/HTTPS
handlers, invalid/unsupported schemes and malformed links, C ABI error ownership,
UI-thread rejection, worker queries and the actual launch completion result.
Every launch has a unique URL fragment. The verifier requires that exact URL in
the real browser's native address field, plus matching browser process, focused
window and native ownership. Neither page content nor network availability is a
substitute for the address assertion.

Phone address inspection uses only native browser address/edit/back controls and
the input method's hide control. The disposable phone's input method was configured
once in basic mode with a 26-key layout. The verifier does not accept setup consent
prompts and never types keys. Input-method ownership checks include the exact
extension process and package UID; browser controls require current native window
ownership. No host mouse/keyboard input is injected.

Run this group independently of the default window/service/dialog/picker group:

```sh
NATIVEAPI_TEST_SUITE=url ./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The four URL cases replace the fixture process and foreground the system browser.
The default suite restores the window acceptance fixture afterward. The phone's
base regression after input-method setup passed **189 native + 5 dialog UI checks**
in process **27578**; that run deliberately did not invoke the unresolved phone
multi-file/folder acceptance paths.

The final computer build then passed the complete existing regression group in
process **5898**: **201 integration, 52 picker, 5 dialog UI, 8 picker UI and 3
desktop-window UI checks**, zero failures. The last three assertions compare real
native window geometry after title-bar dragging, maximize and restore. The passing
fixture and computer emulator remain open. This is coverage of the existing test
group, not a claim that every API or window property is complete.

Evidence on macbook-pro is under `~/Library/Huawei/Logs/url-validation/phone/` and
`~/Library/Huawei/Logs/url-validation/desktop-final/`: exact URL/process manifests,
native reports, UI/ownership snapshots, build logs and HAP SHA-256. Local copies
are stored under ignored `core/build-ohos/verification/urls/`.

## Generation and regression checks

All generated C ABI/binding/reference files are current: `./codegen check` verified
75 C ABI and 339 binding/reference files. Rust bindgen and Dart ffigen ran with no
raw ABI changes; no additional symbols were skipped.

Local macOS checks passed: Rust workspace check; Dart analysis; C# build with zero
errors/warnings; JS native build/typecheck and 4 tests (1 skipped); Python native
build/lint and 26 tests (1 skipped); Go native build/tests; and the existing core
URL validation test (1/1). These are host regressions, not runtime coverage of
every generated language on HarmonyOS. The Windows runner passed PowerShell syntax
parsing; Windows emulator execution was not part of this checkpoint.

## Open audit cases

SDK permission/background launch denial, a separate undeclared-querySchemes build,
environment destruction during a pending launch, and broader Unicode/extreme URI
cases have not been independently exercised. Cleanup/error handling is implemented
but is not claimed as separately runtime-verified. Phone file-picker multiple
selection and folder confirmation also remain open in the
[picker report](ohos_picker_validation.md). Other API modules retain their existing
open status in the coverage ledger.
