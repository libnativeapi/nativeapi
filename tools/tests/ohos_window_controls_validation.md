# HarmonyOS native window control buttons checkpoint

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851 / API 26, HarmonyOS
7.0.0.107. Both disposable guests are ARM64: NativeAPI_Desktop (MateBook Pro,
2in1) and NativeAPI_Phone (Pura X View). The full API goal remains active.

## Behavior and implementation

SetWindowControlButtonsVisible uses the public setWindowTitleButtonVisible API.
IsWindowControlButtonsVisible reads the actual getTitleButtonRect area; it does
not return the last requested flag. A nonempty externally selected subset also
counts as visible. Unsupported, foreign, default and closed windows return false.
The public signatures and other platform implementations are unchanged.

The SDK decor getter cannot stand in for button visibility: on this computer,
setWindowDecorVisible(false) leaves the three real native buttons visible.
SetTitleBarStyle now explicitly applies the button visibility implied by the
core contract after the decor operation succeeds. Hidden hides the buttons;
Normal restores them. Explicitly showing controls after Hidden remains supported.

Actual rendered layout changes produce deduplicated WindowControlButtonsVisible
property events through C++ object, manager and C listeners. Empty button rows
do not reliably deliver a usable title-button rectangle event, so the backend
also observes frameMetricsMeasured and reads only this property after layout.
The initial frame hook is retried once UIContent exists, and seeds its first
usable layout. Removing listeners preserves observation snapshots; destroying
the window or environment releases the native hooks.

Primary references: [Window title-button and frame APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md)
and [WindowStage supported modes](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-WindowStage.md).
Installed SDK declarations were also inspected.

## Actual acceptance

Computer final run, **PID 28140: 61 native assertions and 18 actual UI scenes,
zero failures**. The direct SDK succeeds for the freeform main window (0) and
rejects the ordinary child (1300004). Independent native tree assertions count
three, zero or one actual title buttons, and distinguish close-only from
maximize-only subsets. WindowManagerService and SDK rectangles retain identity
and geometry, except for the intentional programmatic maximize and recovery.

The scenes cover C++/C and worker/UI requests, repeated hides, direct external SDK
changes, partial-button layouts, hidden-title overrides, removing/readding all
three listener paths, ordinary children and retained closed wrappers. The three
event paths agree on eleven post-baseline visibility changes; repeated requests
and true-to-true subset changes do not create duplicate visibility events.
Programmatic maximize and recovery still work with every native button hidden.

Phone final run, **PID 3797: 59 native assertions and 18 actual UI scenes,
zero failures**. The direct SDK returns 801 for main title-button requests and
1300004 for the ordinary child. All scenes retain zero actual buttons, false
C++/C getters and zero visibility-change events. Maximize/recovery calls are
skipped on this unsupported full-screen main window; those scenes verify its
unchanged geometry. The child is actually removed before testing retained wrappers.

Computer general regression, **PID 6351: 268 checks, zero failures**: 200 native
general checks, 52 native picker checks, 5 actual dialog scenes, 8 picker UI checks
and 3 actual desktop drag/maximize/restore checks. All unique general-check labels
match the preceding 201-check run; the clipboard UI callback coalesced from eight
to seven notifications. The retained label comparison verifies that exact difference.

An earlier general run reused PID 3053 after cold boot and hit an existing test
filename. The overwrite prompt was owner-guarded, but the fixture then correctly
failed its fresh-empty-file assertion because that document already had contents.
That run is retained as failed; the complete fresh-process run above passed
without manual interaction. Making picker test filenames unique across guest
reboots was subsequently addressed by the [content-aspect checkpoint](ohos_window_aspect_validation.md)
with per-run names and exact-name parsing, independent of window-button support.

## Separate capability-policy investigation

An independent direct SDK probe completed fifteen computer scenes, **PID 28188**.
Hiding only the maximize button leaves programmatic maximize/recovery working.
Changing WindowStage support to FLOATING-only succeeds (0), but then programmatic
maximize and immersive maximize reject with 1300002. It therefore does not
directly implement the core maximize-button policy. Restoring the three declared
SDK modes [0,1,2] rejects with 1300016 in this probe; the reason remains open.
A new process starts with its ordinary supported modes.

SetMinimizable, SetMaximizable/IsMaximizable, SetClosable, SetFullScreenable,
custom-title double-click policy and arbitrary-edge resize remain open. This
checkpoint implements collective button visibility, without claiming those
capability policies are complete or impossible.
The title-button setter starts at API 14; frameMetricsMeasured starts at API 22.
Older SDK event behavior, system full-screen hover transitions and the remaining
parameter/lifecycle audits are not closed by these API 26 results.

## Reproduction and evidence

With exactly one named unlocked guest, from core:

```sh
NATIVEAPI_TEST_SUITE=window-controls \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The fixture builds a normal application, guards every input by fresh guest
PID/UID/window ownership, and retains native logs, bundle metadata, native trees,
WMS rectangles, button counts and decoded screenshots. No host mouse or keyboard
input is sent. The separate run_control_probe.py records observations and SDK
errors rather than asserting nativeapi support for capability policies.

Remote evidence is under `~/Library/Huawei/Logs/window-controls-validation/`;
the local ignored copy is `core/build-ohos/verification/window-controls-validation/`.
Final computer control scenes: `2in1-1792085585006164000`; final direct SDK probe:
`2in1-1792085713392014000`. Earlier failed investigations remain separately
retained, including the initial layout-baseline bug and incorrect decor gate.
Final phone: `phone-1792086596609431000`. The first phone hot-boot attempt stalled
in HDC installation; logs retained repeated file-transfer failures. After reconnect
and a cold-boot instance appeared, installation succeeded. An incomplete subsequent
run failed to obtain a UITest tree while a separate layout analysis was active;
the complete final run used the fixture alone. Neither attempt is counted as passing.

## Local checks

CTest passes 31/31. Rust, Dart, C#, JS, Python and Go binding checks pass on the
local macOS host; JS has one existing skip and Python one existing skip.
Code generation, raw Rust/Dart regeneration and codegen check pass: 75 C ABI
files and 339 binding/reference files current, with the eight baseline skips.
The smoke HAP builds ARM64 and x86_64; runtime acceptance here is ARM64 only.
The PowerShell suite branch was updated, but neither parsed nor executed because
PowerShell is unavailable locally. Windows runtime acceptance is not claimed.

Follow-up: [independent caption policies](ohos_window_policy_validation.md)
implements the three button choices and their composition. Its actual title
input also confirms that hiding maximize does not suppress native double-click;
complete user-maximize denial remains open. The limitations above describe this
historical collective-visibility checkpoint.
