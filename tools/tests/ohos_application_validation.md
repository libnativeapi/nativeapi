# HarmonyOS application lifecycle and appearance validation

2026-10-15, macbook-pro ARM64, DevEco Studio 26.0.0.851,
HarmonyOS 7.0.0.107 / API 26. Isolated PuraXView phone and MateBook Pro/2in1
computer guests; only one guest runs at a time. This is an implementation
checkpoint, not completion of Application or the full nativeapi API surface.

Core commit: `16ddb45` (`Implement HarmonyOS application lifecycle and appearance`).

The previous port reported constant running/visible state and ignored quit and
appearance operations. The port now tracks the owning UIAbility's actual
Create/Foreground/Background/Destroy callbacks, through an explicit internal
host bridge. It emits Started/Activated/Deactivated/Exiting and exposes the state
before delivering the event. One owning UIAbility is tracked per linked core/UI
environment. This does not claim lifecycle ownership of every Ability/process.

## Final acceptance

| Guest | Approved-quit PID | Native assertions | Required-quit PID | Native assertions | Guarded button actions |
| --- | ---: | ---: | ---: | ---: | ---: |
| Phone | 7148 | 49 passed | 7251 | 25 passed | 6 + 1 passed |
| Computer | 3028 | 45 passed | 3087 | 25 passed | 6 + 1 passed |

Zero failures. These counts include real lifecycle callback assertions and repeated
state checks; they are not counts of distinct API methods. Earlier bring-up runs
are retained separately and excluded. Each terminal case starts a fresh fixture
process after proving the old process stopped. The phone approved-quit case also
verifies a real background/foreground round trip in the same process: Settings is
actually visible while the fixture is absent from the native visible-window tree,
then the original fixture returns. C++ and C ABI callbacks observe running=true /
visible=false in the background and true/true after foregrounding.

The final computer regression, PID 4114, passes 201 integration + 52 picker native
+ 5 dialog UI + 8 picker UI + 3 desktop-window UI checks = 269, zero failures.
Actual native geometry confirms title dragging, maximize and restore. The computer
guest and this passing regression fixture remain open; the phone guest is stopped.

The application groups verify:

- Queries and appearance fail closed before host initialization. Actual application
  multi-instance metadata is available after context attachment even before lifecycle
  forwarding. SINGLETON Ability launch mode is not substituted for application policy.
- Actual startup and foreground events reach C++ and generated C ABI listeners on UI;
  running/visible state follows the real host lifecycle, including required destruction.
- Live main-window discovery, stable C++/C ABI identity, explicit primary clearing /
  restoration, and worker state/window queries.
- Run/Run(window) return -1 for the host-owned Ability loop instead of falsely claiming
  that a native nested loop completed. Invalid appearance enums fail in C++ and C ABI.
- C++ Light and C ABI Dark setters change the real UI; a C ABI worker restores System.
  SDK configuration/resource values are compared with nine actual screenshot pixels
  inside a freshly owned native swatch. Both guests render white (255,255,255), black
  (0,0,0), then the initial system white again. Screenshots were also visually inspected.
  This is application color appearance, not display-backlight brightness.
- A C++ quit veto is shared with the C ABI listener and leaves the actual window alive.
  Worker quit creates one shared confirmation; C++ and C ABI deferred votes both hold it.
  Duplicate requests update the code without duplicating confirmation. One accepted
  vote cannot terminate early; a C ABI cancellation wins and late decisions fail.
- Final C ABI worker acceptance resumes the public SDK termination. Only actual
  onDestroy emits Exiting, once in each binding, with the latest approved code 99.
  Independent WindowManagerService records prove the real native window was removed.
- A separate direct SDK terminateSelf call produces a required, non-cancellable request:
  Cancel fails, Defer returns no vote, actual Exiting carries code 0 and the window ends.
- Unintegrated Show/Hide/icon/dock/progress/badge/menu-bar calls return false, without
  simulating successful changes. This negative check is not an unsupported classification.

All presses are confined to the named emulator CLI. Every press uses a fresh native
layout, fixture PID/cmdline/UID and native window ownership. Theme pixels are sampled
only after verifying that their point belongs to the fixture. PNGs are awaited until
the emulator's asynchronous writer produces a decodable file. No host mouse, guest
keyboard, fabricated lifecycle callback or programmatically invoked click handler is
used to satisfy interaction assertions. The production host forwards genuine SDK callbacks.

## Implementation and reproduction

The private `ohos_application.h` bridge is called from the real UIAbility callbacks;
Create follows AttachContext. Destroy releases only the matching context. It invalidates
old generations and pending votes; environment cleanup emits no invented normal exit.
Callbacks retain invalidatable generation tokens, not an unchecked Application pointer.
The shared EventRequest policy handles cancellation/defer/duplicate merging. Approval
submits UIAbilityContext.terminateSelf and holds the request through actual destruction;
an SDK rejection releases it for retry. No process-kill fallback or nested ArkTS loop.

Appearance uses public ApplicationContext.setColorMode; per-Ability overrides take
precedence. Metadata reads ApplicationInfo.multiAppMode.multiAppModeType. Primary
selection is separate from the system window type/WindowStage ownership. The window
query implementation uses registered live windows without consulting lifecycle flags;
a dedicated host-without-lifecycle interoperability run remains open. Source
references: [UIAbilityContext termination](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-ability-kit/js-apis-inner-application-uiAbilityContext.md),
[ApplicationContext color mode](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-ability-kit/js-apis-inner-application-applicationContext.md).

From core, with the selected guest unlocked (the test Python needs Pillow):

```sh
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
NATIVEAPI_TEST_SUITE=application tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

Evidence on macbook-pro:
`~/Library/Huawei/Logs/application-validation/{phone,desktop}/`: final suite logs,
per-PID SDK/native records, ownership trees, window/Ability dumps, RGB samples,
screenshots, build logs and regression evidence. Ignored local copies:
`core/build-ohos/verification/application-validation/`. Final native assertion logs
are under phone/{7148,7251} and desktop/{3028,3087}.

Both HAP architectures compile; runtime evidence is ARM64 only. PowerShell runner
syntax is checked on windows-laptop; no Windows OHOS runtime claim. Public signatures
are unchanged. All 75 C ABI and 339 binding/reference outputs are current with the
same eight baseline skips. Raw Rust/Dart regeneration has no diff. Local macOS checks
pass: core CTest 30/30, Rust workspace, Dart analysis, C# build (zero warnings/errors),
JS 4 pass/1 skip, Python 26 pass/1 skip plus Ruff, and Go (cached pass).

## Remaining work

- Desktop application focus/activation versus Ability foreground semantics;
  multiple owning Abilities/environments, process-wide lifecycle/exit policy and
  same-process Ability replacement/recreation; current support is explicit and limited
  to one owning Ability, not inferred whole-process control.
- Real SDK termination failure/refusal and retry, pending confirmation versus external
  destruction/environment teardown, late callbacks, listener failures and rapid reentry.
- Multi-instance/clone configuration variants; all returned-window list ownership paths,
  secondary/closed primary selections and wider worker/negative-path coverage.
- Per-Ability color overrides, new/secondary window inheritance, theme failure paths,
  content-loading boundaries and environment teardown.
- Status-bar-bound Show/Hide, mission icons and numeric badges have a later
  [implementation checkpoint](ohos_app_integration_validation.md). Dock visibility,
  progress/menu-bar integration and the full failure/teardown matrix remain open.
- All other modules and open cases in the full coverage ledger remain in scope.
