# HarmonyOS computer emulator acceptance — 2026-10-14

## Application checkpoint — 2026-10-15, 20:35

On macbook-pro ARM64, the final application group passes 45 native assertions in
PID 3028 (real theme pixels, C++/C ABI cancellation/deferred approval and actual
Ability termination) and 25 in PID 3087 (required SDK termination), with 7 guarded
button actions. The final full regression in PID 4114 again passes all 269 checks:
201 integration + 52 picker native + 5 dialog UI + 8 picker UI + 3 desktop-window UI.
Native geometry confirms title dragging/maximize/restore. The passing regression
fixture remains open. See [application validation](ohos_application_validation.md)
for evidence and remaining multi-Ability/failure/teardown/integration work.


## Native menu checkpoint — 2026-10-15, 19:25

The real ArkUI Menu/MenuItem backend passes 40 native + 4 guarded UI checks on
each guest; see [menu validation](ohos_menu_validation.md). The computer uses the
SDK's default native subwindow policy, with ownership verified before each press.
The final computer regression fixture, PID **4596**, passes **201 integration,
52 picker, 5 dialog UI, 8 picker UI and 3 desktop-window UI checks** (269 total),
zero failures. The passing computer emulator and fixture remain open on macbook-pro.
Evidence: `~/Library/Huawei/Logs/menu-validation/desktop/` and ignored
`core/build-ohos/verification/menu-validation/desktop/`.


## Native view checkpoint — 2026-10-15, 18:30

Native containers/labels/buttons/text fields/images now run inside a real ArkUI
ContentSlot; see [view validation](ohos_view_validation.md). Computer view PID 3070
passed 32 native + 3 guarded UI checks, including actual window resize/restore
and root/child relayout. Phone view PID 3490 passed 30 native + 3 UI checks.
The final computer regression fixture, PID **3738**, passed **201 integration,
52 picker, 5 dialog UI, 8 picker UI and 3 desktop-window UI checks** (269 total),
zero failures. The computer emulator and passing fixture remain open on macbook-pro.
Evidence: `~/Library/Huawei/Logs/view-validation/desktop/` and ignored
`core/build-ohos/verification/view-validation/desktop/`.

## Notification cold-launch/refusal extension — 2026-10-15, 17:54

Notification body/button cold launches, Shutdown retention and actual authorization
refusal now pass on both guests; see [notification validation](ohos_notification_validation.md).
The final computer fixture, PID **4013**, passed **201 integration, 52 picker,
5 dialog UI, 8 picker UI and 3 desktop-window UI checks**, zero failures. Native
geometry confirms dragging, maximize and restoration. The passing app and emulator
remain open. Evidence is at `~/Library/Huawei/Logs/notification-lifecycle/desktop/`
on macbook-pro and ignored `core/build-ohos/verification/notification-lifecycle/desktop/`
locally.

## Notification implementation regression — 2026-10-15, 17:22

The final computer build passed **21 notification checks** and real action/body
clicks in process **4185**; see [notification validation](ohos_notification_validation.md).
The restored default fixture, process **4744**, passed **201 integration, 52 picker,
5 dialog UI, 8 picker UI and 3 desktop-window UI checks**, zero failures. Native
geometry confirmed dragging, maximize and restore. The passing app and emulator
remain open. Final evidence is under
`~/Library/Huawei/Logs/notification-validation/desktop-final/` on macbook-pro and
ignored `core/build-ohos/verification/notifications/desktop-final/` locally.

## URL implementation regression — 2026-10-15, 03:40

The current source, including the public-SDK URL implementation, was rebuilt for
the computer guest. Four independent URL launches passed **44 native + 12 UI
checks**; see [URL validation](ohos_url_validation.md). The normal regression
fixture was then restored. Process **5898** passed **201 integration, 52 picker,
5 dialog UI, 8 picker UI and 3 desktop-window UI checks**, zero failures. Dragging,
maximize and restore were verified against actual native geometry. The emulator
and passing app remain open. Evidence is at
`~/Library/Huawei/Logs/url-validation/desktop-final/` on macbook-pro, copied locally
under ignored `core/build-ohos/verification/urls/desktop-final/`.

## Committed-source revalidation — 2026-10-15, 02:51

Core revision **`e07fe6c`** was rebuilt and installed on the macbook-pro computer
guest. Process **3053** passed **201 integration, 52 picker, 5 dialog UI, 8 picker
UI and 3 desktop-window UI checks**, zero failures. The app and computer emulator
remain open. Final logs, guarded ownership/geometry snapshots, HAP hash and screenshot
are saved at `~/Library/Huawei/Logs/picker-validation/desktop-e07fe6c/`, with a local
copy under ignored `core/build-ohos/verification/pickers/desktop-e07fe6c/`.
Phone multi-selection and folder acceptance remain open; see
[picker validation](ohos_picker_validation.md).

## Current-source revalidation — 2026-10-15, 02:21

The computer emulator on `macbook-pro` was cold-booted and the current working
source rebuilt and installed as a desktop-only (`deviceTypes: ["2in1"]`) HAP.
Guest process **3410** passed **201 integration checks, 52 document-picker checks,
5 dialog UI checks, 8 picker UI checks and 3 desktop-window UI checks**, all with
zero failures. This run includes the uncommitted FileDialog implementation; it
does not imply that the pending phone picker tests or every HarmonyOS API passed.

The desktop-window checks use actual compositor geometry and guarded guest input:
title-bar dragging moves the native window, maximize enlarges it, and restore
returns to its preceding floating rectangle. The integration suite additionally
checks native main/child window lifecycle, focus, size constraints, events,
maximize/full-screen distinction and minimize/restore.

The Mac desktop launcher is `~/Desktop/NativeAPI HarmonyOS Desktop.app`. It now
loads `~/Library/Huawei/ohos-env.sh` before starting the emulator. Scenario CLI
commands must also inherit this environment. No host mouse or keyboard injection
was used. The passing app and computer emulator remain open; the separate phone
instance is retained and stopped.

Evidence is saved on macbook-pro at
`~/Library/Huawei/Logs/desktop-acceptance-20261015-0220/`, including build/runtime
logs, fresh UI ownership and geometry snapshots, installed bundle metadata,
HAP SHA-256, launcher source and a final screenshot. A local copy is under ignored
`core/build-ohos/verification/desktop-acceptance-20261015-0220/`.

## Revalidation — 2026-10-15

The installed computer emulator on `macbook-pro` was revalidated at 00:35:43
(Asia/Shanghai), using the previously built desktop HAP from the `0bd817b`
service checkpoint. Guest process 11249 reported **155 passed, zero failed**.
The guarded guest title-bar drag, maximize and restore test then reported
**3 passed, zero failed**. This run reused the existing HAP; it does not validate
the subsequent uncommitted ArkUI/dialog implementation.

The emulator remains running with the passing application visible. Its desktop
launcher is `~/Desktop/NativeAPI HarmonyOS Desktop.app`. Configuration confirms
MateBook Pro / `2in1`, HarmonyOS image 7.0.0.107 / API 26, ARM64, four CPU cores,
4GB RAM and 6GB storage. The phone instance is retained separately.

Evidence, including the HAP SHA-256, installed bundle metadata, native window
geometry, guarded UI ownership snapshots and screenshot, is saved on the host at
`~/Library/Huawei/Logs/desktop-acceptance-20261015/`. A local copy is under ignored
`core/build-ohos/verification/desktop-acceptance-20261015/`.

## Installation and result

`macbook-pro` now has an independent `NativeAPI_Desktop` emulator, created with the
MateBook Pro computer profile: HarmonyOS 7.0.0.107 / API 26, ARM64, 4 CPU cores,
4GB RAM, 6GB storage, 3120×2080 display at 304 dpi. DevEco Studio is 26.0.0.851.
The official 2in1 image was installed under `~/Library/Huawei/Sdk`; instance data
is under `~/.Huawei/Emulator/deployed/NativeAPI_Desktop`.

The desktop-only HAP passed **108 checks with zero failures**, then passed the same
108 checks again after restarting its process. The final rebuilt source passed at
23:08:17 with guest process 19986, followed by another successful interaction run. The guest ArkUI tree contains the
rendered `NATIVEAPI_OHOS_PASS (108 passed, 0 failed)` result. A separate guarded
interaction test passed **3 checks with zero failures** using the actual guest title
bar, maximize button and restore button. Host mouse and keyboard input was not used.

The emulator and passing app remain open. The Mac desktop launcher is
`~/Desktop/NativeAPI HarmonyOS Desktop.app`; it cold-boots with existing data, without
resetting the guest. The earlier phone instance and launcher are retained.

## What ran

- Existing device, display, preferences, clipboard and main/child window integration
  checks, including actual native geometry, focus, cancellable/deferred closing,
  events, invalidation, failed creation and reentrant cleanup.
- Actual main-window movement to logical (120, 120) and resize to 640×480. The native
  compositor reports physical (228, 228), 1216×912 at scale 1.9.
- Native title updates and decorations hidden/restored. Movement/resizing policy
  setters acknowledge disable/enable; this does not claim a negative drag/resize
  gesture test while those policies are disabled.
- Native minimum size 600×400 and maximum size 1200×900 round trips.
- Maximize expands native geometry, reports maximized mode and emits its event.
  Recover returns to the previous floating geometry and emits restoration.
- Full screen covers the real 1642.1×1094.7 logical display, differs from maximize,
  and emits one enter/exit event. Exiting restores the preceding floating or
  maximized mode. Content immersive layout is checked inside full-screen mode.
- Minimize hides the main window and emits its event. Restore returns the desktop
  main window visibly and with focus.
- Guest title-bar drag moves the native rectangle by (240, 160) physical pixels
  without changing its size. Native maximize/restore controls enlarge the window
  and return it to that same moved rectangle. Widget positions come from the guest
  tree; UITest visible-window roots confirm ownership before every input action.

## Fixes and lifecycle boundary

The first desktop run exposed differences that the phone emulator could not cover.
`Maximize()` now explicitly requests the SDK's non-immersive maximize presentation;
the SDK default enters full screen. Desktop full-screen setters/getters/events now
follow the real native window mode and preserve the preceding maximized mode.
`Restore()` uses the desktop SDK restore operation.

The passing desktop HAP has `deviceTypes: ["2in1"]`, verified in its installed bundle
metadata. The runner chooses this build model automatically for a 2in1 guest,
without rewriting the source manifest. A build for a different lifecycle cannot
silently be reused with `--skip-build`.

A mixed `phone`/`tablet`/`2in1` manifest was also exercised: its main window can minimize,
but the Ability becomes background and the public window `restore()` call fails.
The NDK show call returned success without restoring the minimized state. This was
recorded as a limitation, not accepted as restored behavior. Such hosts must first
bring their UIAbility foreground. Huawei documents the [different window/Ability
lifecycles](https://developer.huawei.com/consumer/cn/doc/doccenter-capabilities/window-lifecycle).

System title-bar gestures work in the guest. Nativeapi's explicit `StartDragging()`
and `StartResizing()` methods remain unsupported. Topmost, shadow, privacy mode,
reparenting and opacity were not added to this desktop acceptance. Main-window
closing remains owned by UIAbility. Windows desktop runtime was not run in this task.

## Reproduction and evidence

Sync core from the local checkout through fastforge, then on macbook-pro run:

```bash
cd ~/.fastforge/_work/core-02d614d4e50d
./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents 127.0.0.1:5555 --skip-build
python3 tests/ohos_smoke/verify_desktop_ui.py /Applications/DevEco-Studio.app/Contents
```

The local emulator must be running and unlocked. Its first-boot user screen was
unlocked through that named disposable guest; no host login or user credentials were used.

Remote logs: `~/Library/Huawei/Logs/nativeapi-desktop-smoke.log`,
`nativeapi-desktop-ui.log`, and `desktop-window-validation/` in that Logs directory.
Local copies, screenshot, bundle metadata, widget tree and compositor snapshots live
under ignored `core/build-ohos/verification/desktop-window-management/macbook-pro/`.
The reusable assertions live in `core/tests/ohos_smoke/`.

The HAP compiled both ARM64 and x86_64 native libraries. `./codegen check` passed with
339 generated files current; public C++/C/binding signatures are unchanged. Both
runner scripts passed syntax checks (Bash locally, PowerShell parser on Windows). Other
platform runtime suites were not rerun for this desktop-specific follow-up.

Validated core revision: `baee9f3`.
