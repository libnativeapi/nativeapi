# HarmonyOS application visibility, mission icon and badge checkpoint

2026-10-15, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107
(API 26). Runtime guests are ARM64 NativeAPI_Desktop (MateBook Pro / 2in1)
and NativeAPI_Phone (Pura X View). This is a checkpoint in the full API ledger;
multiple contexts, failure/teardown and other modules remain open.

## Accepted behavior

- Computer final **PID 21197: 62 native assertions, zero failures**. Actual SDK
  whole-Ability hide removes both main and child windows from the visible system
  tree; WindowManagerService reports their z-orders as -1. Five cycles restore
  them through C++/C application calls, activating Window::Show, external same-
  process launcher activation and a real status-icon click. The same window IDs
  and native rectangles survive every cycle. C++/C ABI activation events agree,
  and event listeners observe the updated visible state.
- Two actual mission-window icon replacements render magenta and cyan pixels in
  the native title bar. Calls cover C++ UI and C ABI worker paths, a Unicode/#/%
  image path and an explicitly cleared Application primary-window selection.
  Empty, missing and nonsquare images fail. Cloned PixelMaps isolate SDK ownership.
- Desktop SceneBoard Dock nodes display **9**, **99+**, then no badge. Independent
  public getBadgeNumber reads report 9, 100 and 0. Notification settings enable
  badge numbers on this image. No notification was published to create the badge.
- Phone final **PID 3638: 23 native assertions, zero failures**. The actual home
  launcher nodes display **9**, **99+**, then no badge after clearing. C++ UI/C ABI
  worker/native worker calls agree with independent SDK reads. Foreground-to-home
  preview and same-process relaunch cover three real background/foreground pairs.
- On phone, independent public hideAbility, showAbility and setMissionWindowIcon
  reject with **801**. C++/C ABI worker calls return false for these unsupported
  desktop operations. Invalid/overflowing/nondecimal/NUL badge labels fail.

The direct SDK desktop reference **PID 14150** establishes that hideAbility and
showAbility succeed with a real status item while neither emits UIAbility
onBackground/onForeground. Per-window isWindowShowing remains true even when the
whole SDK window group is hidden. The backend records only successful SDK hiding,
combines it with owning lifecycle state, and reconciles external restoration from
the primary window's public windowVisibilityChange event. False visibility alone
is not treated as application hiding because occlusion can also produce it.

A failed Dock restore attempt is retained: hideAbility removes the running Dock
item on this image. No nonexistent Dock click is claimed. Normal launcher activation
(`aa start` of the same EntryAbility, with the same PID) and the actual status item
are separately verified restore paths.

## API and SDK boundaries

- Application Show/Hide require forwarded owning-Ability lifecycle, initialized
  context and a desktop status-bar capability. The application must already have
  a real status registration, whether created by nativeapi or the host. No loop
  over individual windows substitutes for application hiding.
- Activating Window::Show restores the owning hidden group. Non-activating show
  preserves SDK focusOnShow=false; it does not force application activation.
  Unrelated per-window Hide does not mark Application hidden.
- SetIcon calls public UIAbilityContext.setMissionWindowIcon, not the legacy
  private setMissionIcon. It changes the owning window/mission/snapshot icon,
  not the installed launcher asset. Images must decode as square RGBA within
  128 MB. No privileged SET_ABILITY_INSTANCE_INFO request is made.
- SetBadgeLabel calls public notificationManager.setBadgeNumber. Nonnegative
  ASCII decimal counts through INT32_MAX are accepted; empty/0 clears, and the
  convenience label 99+ maps to count 100. Counts above 99 render as 99+ subject
  to system notification/badge settings. No permission prompt is auto-launched.
- These SDK operations are asynchronous: UI calls return submission status;
  worker calls wait for actual SDK completion. SDK errors are logged and report
  false to workers. N-API roots protect pending arguments/callbacks, generation
  checks reject stale owning-Ability updates, and environment cleanup releases
  workers without invoking JavaScript.

Current installed declarations are the authoritative SDK reference:
`openharmony/ets/api/application/UIAbilityContext.d.ts`,
`openharmony/ets/api/@ohos.notificationManager.d.ts`, and
`openharmony/ets/api/@ohos.window.d.ts` under DevEco's default SDK.
See also [Huawei UIAbilityContext reference](https://developer.huawei.com/consumer/en/doc/harmonyos-references-V14/js-apis-inner-application-uiabilitycontext-V14)
and [Huawei PC status-bar integration](https://developer.huawei.com/consumer/cn/doc/doccenter-industry-solutions/pc_status_bar-0000002551100435).

## Reproduction and evidence

From core, with exactly the selected named disposable guest running and unlocked:

```sh
NATIVEAPI_TEST_SUITE=integrations \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The harness checks fresh PID/UID, exact guest and native window ownership before
input. The narrow SceneBoard status-item action verifies the actual app token and
system window at the point. No host cursor or keyboard input is used. Evidence
includes native logs, owner records, native trees, window dumps and icon pixel
samples. Final remote evidence:
`~/Library/Huawei/Logs/app-integration-validation/desktop/{final-suite.log,21197/,2in1-1792075042351347000/}`
and `phone/{initial-suite.log,3638/,phone-1792075270376241000/}`.
Ignored local copies live in `core/build-ohos/verification/app-integration-validation/`.
Earlier failed hide/getter and Dock-target attempts remain useful failure evidence.

## Remaining work

- SDK denial/refusal, missing real status registration, rapid opposite/reentrant
  requests, pending-request destruction and environment teardown, stale callbacks,
  multiple owning contexts, inactive restoration and occlusion interaction.
- Full image formats/size/alpha limits and payload ownership matrix; system badge
  settings disabled, additional count boundaries and SDK denial/persistence cases.
- Dock visibility, application progress/menu bar and every open API/parameter/
  lifecycle/binding case in the coverage ledger. These are not closed by this run.

## Regression and generated bindings

Application lifecycle/theme/termination regression passes on both guests: computer
full PID 23552 **44** assertions and required PID 23606 **24**; phone full PID 4906
**49** and required PID 5174 **25**. Each guest also completes seven guarded app
UI actions and actual native window removal. The desktop count is two fewer than
the earlier application checkpoint because the old unsupported Show/Hide assertion
is removed once from each fresh desktop process; the new integration suite tests
those operations positively. Phone retains the unsupported assertion.

Public signatures are unchanged. All 75 C ABI and 339 binding/reference outputs
are current, with the same eight baseline skips; raw Rust and Dart regeneration
has no diff. Local macOS checks pass: CTest 30/30, Rust workspace, Dart analysis,
C# build (zero warnings/errors), JS 4 pass/1 skip, Python 26 pass/1 skip plus Ruff,
and Go tests (cached). New Python harnesses pass lint and formatting checks; the
shell runner parses and the PowerShell runner parses on windows-laptop. Both HAP
architectures compile; no x86_64 runtime or Windows OHOS runtime result is claimed.

Tray regression also passes: computer PID 3452, **40 native assertions** plus six
actual icon captures, clicks/menus/live updates and removal; phone PID 6375,
**19 assertions**. Final tray-suite evidence is retained alongside this report's
application integration evidence on macbook-pro.

Final computer general regression **PID 6066** passes **201 base + 52 picker +
5 dialog UI + 8 picker UI + 3 actual window drag/maximize/restore checks = 269**,
zero failures. This is one more than the preceding 268-check tray checkpoint
because the coalesced clipboard notification count varies; unique assertion
labels remain covered. Evidence: `desktop/{general-regression.log,window-regression.log,general-6066/}`.
The computer guest is left running with the real passing acceptance app.

Core implementation checkpoint: `85b6314` on main. No changes are pushed.
