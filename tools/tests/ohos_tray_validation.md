# HarmonyOS tray implementation checkpoint

2026-10-15, macbook-pro, DevEco Studio 26.0.0.851. Runtime: ARM64
HarmonyOS 7.0.0.107 / API 26, NativeAPI_Desktop (MateBook Pro / 2in1,
3120×2080 at 304 dpi) and NativeAPI_Phone (Pura X View). Both HAP
architectures compile; x86_64 runtime and windows-laptop OHOS runtime are untested.
This is a checkpoint in the full API ledger, not completion of the whole goal.

## Results

- Computer, final production backend **PID 8024: 40 native assertions, zero failures**.
  The guarded UI suite also verifies six actual icon captures (magenta, cyan,
  template black, restored cyan, burst-final cyan and re-registered cyan), real
  C++/C ABI left-click delivery, real SDK menus and live label updates, C ABI
  hide/re-register, and actual system-tree removal after last-wrapper destruction
  while a native icon update is pending. The original source PixelMaps remain
  usable after repeated replacement. Worker queries/registry/list-handle release
  and actual worker-submitted hover text also pass.
- Phone **PID 3700: 19 native assertions, zero failures**. The same C++ and C ABI
  identities, safe invalid-handle behavior, local properties, manager enumeration
  and worker dispatch pass. Capability is false and registration fails without
  a replacement UI. A separate direct ArkTS reference process **PID 5476** reports
  `SystemCapability.PCService.StatusBarManager` false, independently of the backend.
- Final computer regression **PID 4528: 200 base + 52 picker + 5 dialog UI +
  8 picker UI + 3 desktop-window checks = 268, zero failures**. Actual native
  geometry confirms title dragging, maximize and restore. Compared with the prior
  269-check run, the sole count difference is one coalesced clipboard notification
  assertion; all unique assertion labels are retained.
- Direct desktop SDK comparison **PID 24665** reports system capability true,
  `isStatusBarCapabilitySupported` true and add completion code 0. A guarded click
  on its real system icon returns `data.iconClickType = leftClick`.

The implementation uses the **HMS** public StatusBarManager module, beyond the
OpenHarmony API directory. The kit is `@kit.DeskTopExtensionKit`; migration from
StatusBarExtensionKit is recorded in the [Huawei API differences](https://developer.huawei.com/consumer/en/doc/harmonyos-releases/js-apidiff-desktopextensionkit-6003).
The installed authoritative declarations are
`DevEco-Studio.app/Contents/sdk/default/hms/ets/api/@hms.pcService.statusBarManager.d.ts`.
The [Huawei status-bar sample](https://gitee.com/harmonyos_samples/status-bar-extension-kit-samplecode/blob/master/readme_en.md?skip_mobile=true)
provides the independent application/SDK integration baseline. No private system API,
mock status item or generic popup is counted as a native tray.

## Behavior and restrictions

- Stable process IDs, borrowed opaque model tokens, safe wrapping and manager
  Get/GetAll share identities. The implementation reuses ObjectRegistry for its
  models and counts wrappers explicitly, so manager lookup does not make an icon
  immortal. Only one actual icon can be visible for an application; a second
  registration fails. The manager includes this library's hidden live objects.
- Actual SDK completion changes visibility, image/template and tooltip state.
  Pending requests retain their model safely. Failures preserve applied values;
  an overlong hover-text request is independently rejected by SDK logs. Void
  setters coalesce during pending work; a conservative one-second queued interval
  avoids refresh limits. HMS module references are retained for delayed/worker
  calls, rather than resolving an ArkTS module outside its original call context.
- The service receives separate cloned PixelMaps for white and black themes.
  This isolates caller-owned images from SDK release/replacement. Template maps
  preserve source alpha and generate distinct white/black pixels. Timers only
  submit real requests; they never invent successful state transitions. Timers
  are unreferenced and closed during environment cleanup without JavaScript calls.
- Native right-click menus project grouped separators, icons, selected states,
  two levels, 20 top-level items and 20 children per submenu. Live edits refresh
  the SDK menu. A selection dispatches MenuItemClickedEvent after checking current
  membership/enabled ancestors. It does not synthesize TrayIconRightClickedEvent.
  Disabled entries, mixed states, accelerators, item tooltips, deeper nesting and
  submenu separators lack equivalent SDK fields: initial incompatibility rejects
  registration; an incompatible live tree clears the native menu. The host also appends its own Exit command.
- Only the SDK's left-click event is emitted. The public SDK supplies no bounds
  getter, programmatic menu Open/Close or matching right/double-click and menu
  lifecycle callbacks here. Bounds are empty and Open/Close return false.
  Identifier/title/size/position/content-view properties retain configuration;
  they do not claim custom inline text/views or changes to SDK-owned geometry.
  Unsupported Clicked/DoubleClicked menu triggers are rejected.
- Last-owner destruction requests actual removal. SDK refusal when no window is
  foreground retains the registration for a retry on actual forwarded owning-
  Ability activation. That broader background retry remains unverified.

## Reproduction and evidence

With only the selected disposable NativeAPI guest running and unlocked, from core:

```sh
NATIVEAPI_TEST_SUITE=tray \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The computer verifier requires Pillow. It checks the fixture's PID/UID, the exact
running guest, native window ownership and the icon's actual application
accessTokenId before every press. Its narrow SceneBoard exception admits only
that token's own hover tooltip; unrelated overlays abort input. Mouse moves and
clicks share one uinput invocation to preserve the target coordinates. No host
cursor or keyboard injection is used. The direct SDK page is selected with
`nativeapiSuite=tray-reference`.

Final evidence on macbook-pro:
`~/Library/Huawei/Logs/tray-validation/desktop/{final-verified-suite.log,final-build.log,8024/}`
and `phone/{final-suite.log,final-build.log,3700/,reference-native.log}`.
`desktop/regression.log`, `regression-window-ui.log` and `regression/` retain the
final 268-check regression.
Ignored local copies are under `core/build-ohos/verification/tray-validation/`.
Earlier failed burst runs are retained: they exposed SDK refresh rejection and
ArkTS module resolution outside its original call context. The assertion was
preserved and passes in the final backend.

Public signatures are unchanged. The 75 C ABI and 339 binding/reference files are
current, with the same eight baseline codegen skips. Raw Rust/Dart regeneration
has no diff. Local macOS checks pass: core CTest 30/30, Rust workspace, Dart
analysis, C# build, JS 4 pass/1 skip, Python 26 pass/1 skip, Go tests (cached), and
Python harness lint. The PowerShell runner parses on windows-laptop; that is not
an OHOS runtime acceptance result.

## Remaining work

- Full menu type/boundary/error matrix, nested menu selection and native selected
  icons, alpha edges, desktop appearance changes and source formats/sizes.
- Real no-foreground removal refusal/retry, hide/clear during every pending phase,
  last-owner destruction during add, SDK failures/rate limits, environment teardown,
  late callbacks, multiple contexts/environments and lifetime/worker races.
- Caller-visible feedback for void setter failures and full validation boundaries.
- Status-bar-bound Application Show/Hide now has a separate
  [implementation checkpoint](ohos_app_integration_validation.md). Remaining dock/taskbar
  integrations and all open cases in the coverage ledger remain within the original goal.
