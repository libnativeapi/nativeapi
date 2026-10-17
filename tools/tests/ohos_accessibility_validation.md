# HarmonyOS accessibility state validation

2026-10-15, macbook-pro ARM64; DevEco Studio 26.0.0.851, HarmonyOS
7.0.0.107 / API 26. One named disposable guest runs at a time.

Core commit: `ef506c9` (`Query real HarmonyOS accessibility state`).

The previous backend set a local flag in `Enable()` and returned that flag from
`IsEnabled()`, claiming system enablement without an actual system operation.
The backend now calls the public `accessibility.isOpenAccessibilitySync()` on each
query. UI calls run directly in the initialized ArkTS environment; worker calls
marshal through the same environment. No AbilityContext is required. Before
initialization, on environment loss or an SDK error, the query returns false.

`Enable()` is a documented no-op for ordinary applications. The SDK operation
that activates an extension, `accessibility.config.enableAbility`, is a system API
requiring `ohos.permission.WRITE_ACCESSIBILITY_CONFIG`; it also requires a concrete
extension identity/capability list absent from this generic API. The library does
not invoke private interfaces, request system privileges or return a fabricated
local state. This restriction applies to activation, not to the supported public
state query. Sources: [public state query](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-accessibility-kit/js-apis-accessibility.md#accessibilityisopenaccessibilitysync10),
[system-only activation](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-accessibility-kit/js-apis-accessibility-config-sys.md#enableability).

## Final runtime evidence

| Guest | PID | First run ID | Live requery ID | Result per run |
| --- | ---: | --- | --- | --- |
| Phone / PuraXView | 9085 | 1792064960322001000 | 1792065008759010000 | 12 native checks passed |
| Computer / MateBook Pro, 2in1 | 3332 | 1792065073729530000 | 1792065076238868000 | 12 native checks passed |

Zero failures. Each live requery is a fresh Ability Want and native test invocation
in the same process, using a unique marker to exclude old PASS records. These are
12 distinct checks repeated twice per guest, not 24 distinct scenarios. Each run
also compares the SDK/native result with the actual AccessibilityManagerService
`accessible` state. All four final runs observe false and an empty enabled-extension
list, matching system `accessible: 0`.

The checks cover C++ and generated C ABI UI-thread queries, worker-thread queries,
pre-initialization refusal, repeated C++/C ABI Enable on both threads, and the
unchanged real state after those calls. An independent ArkTS SDK state query and
its enabled-extension list supply the reference. No input or permission grants
are needed by the acceptance runner. The installed final fixture has no test
accessibility extension and declares no new permissions.

Evidence on macbook-pro:
`~/Library/Huawei/Logs/accessibility-validation/{phone,desktop}/`, including final
suite logs, live requery logs, native per-run records, system state and process
identity. Ignored local copy: `core/build-ohos/verification/accessibility-validation/`.
The computer guest remains running with the passing fixture.

## Reproduction and compatibility

From core, with the selected guest running:

```sh
NATIVEAPI_TEST_SUITE=accessibility tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
python3 tests/ohos_smoke/run_accessibility_suite.py /Applications/DevEco-Studio.app/Contents
```

The second command rechecks the current fixture process. No cursor or keyboard
input is injected. Both HAP architectures compile; runtime evidence is ARM64 only.
The updated PowerShell runner parses on windows-laptop; no Windows OHOS runtime
claim is made.

The existing signatures and generated binding code remain unchanged; documentation
and the generated API reference describe the real platform semantics. Raw Rust/Dart
regeneration produces no changes. Local macOS checks pass: core CTest 30/30,
Rust workspace, Dart analysis, C# build (zero warnings/errors), JS 4 pass/1 skip,
Python 26 pass/1 skip plus Ruff, and Go (cached pass). Generation retains the same
eight known skips.

## Open evidence

Enabled-state and false → true → false transitions are **not verified**. A
non-intercepting test extension was temporarily installed while investigating the
system settings, but the tested computer settings and phone accessibility pages
did not expose its management entry. No extension was enabled and no system flag
was overwritten. The unused extension was removed from the final fixture. This is
a remaining emulator/runtime investigation, not evidence that ordinary extensions
or true-state queries are unsupported.

SDK failure and pending worker/environment teardown scenarios also remain open.
The complete API goal and other module audits remain active in the coverage ledger.
