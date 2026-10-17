# HarmonyOS native window dragging checkpoint

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851 / API 26, HarmonyOS 7.0.0.107.
Actual guests: ARM64 NativeAPI_Desktop (MateBook Pro / 2in1) and NativeAPI_Phone
(Pura X View). The full API goal remains active.

## Implemented behavior

Window::StartDragging now calls the public SDK startMoving method on the owning
live window. The call happens synchronously through the existing UI dispatch
inside the host's active ArkTS onTouch down/move callback. The SDK owns movement
and ends it when the pointer is released; no library timer or fake window-position
simulation implements the drag. The void operation adds no local success flag.
Destroying, closed, foreign and empty handles are safely ignored.

A freeform computer main window and a native child can move. A full-screen phone
main window is rejected by the SDK, while the native phone child can move.
Disabling native title dragging does not disable an explicit startMoving request;
the fixture keeps that title policy disabled to exclude automatic title movement.
Public signatures are unchanged; the other platform definitions remain in place.

Primary reference: [public startMoving API](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#startmoving14).
The installed SDK agrees: callers must have an active touch-down/move callback;
non-freeform main windows can return 801 or 1300004.

## Actual acceptance

The new window-drag suite creates a real undecorated magenta child and a separate
main-window content drag pad. Eight scenes compare C++, C, direct SDK and no-op
control handlers in both windows. Every gesture starts from fresh native tree,
window-service PID/window identity and UID ownership evidence. The named guest
receives atomic gestures; the host cursor and keyboard are never used.

The independent WindowManagerService rectangle must follow the touch displacement
with stable size, stop after release and remain unchanged for a click without
movement. All positive gestures also require real native move events. C++ object,
manager and C subscriptions must agree, and C++/C getters must agree after input.
Retained closed wrappers are tested after destruction and all observers removed.

Computer first touch run **PID 17370: 41 native assertions, 8 actual scenes, zero
failures**. Each successful C++/C/SDK gesture adds ten observed move events in
this run: child counts 1→11→21→31; main counts 0→10→20→30. These observations
are checked independently against retained logs as well as the suite's parity
assertions. SDK main/child reference requests return 0.

Final computer touch/mouse run **PID 20756: 45 native assertions, 8 scenes,
8 touch gestures, 8 mouse gestures and 8 click-only checks, zero failures**.
Both pointer sources exercise six positive movements and two no-op controls
through the same C++/C/SDK/control modes. The new runner
requires actual move-event increases after each positive scene. The retained
`gesture-summary.json` labels each independently checked touch and mouse rectangle;
original runtime per-gesture files and raw WMS evidence remain intact.

Phone **PID 18132: 41 native assertions, 8 actual scenes, zero failures**. Child
C++/C/SDK gestures move the real native child. Main C++/C/reference/control scenes
leave the full-screen main rectangle unchanged. The direct SDK main request
returns **1300004**, including the click-only check; the child returns 0.

## Independent input regions and other open work

A valid one-rectangle setTouchableAreas call returns **201** for both main and
child windows on both tested guests. The retained bundle-service metadata proves
normal APL, isSystemApp=false and no requested SET_WINDOW_TOUCH_AREAS permission.
The source manifest is not modified to grant it, and no shell permission grant,
privilege change or forged approval profile is used for this API.

The [official restricted-permission definition](https://github.com/openharmony/docs/blob/master/en/application-dev/security/AccessToken/restricted-permissions.md#ohospermissionset_window_touch_areas)
classifies the permission as system_basic/system_grant, PC/2in1, since API 26.
It is a restricted permission, not an ordinary declaration-only grant. The public
SDK also caps the region at ten rectangles and invalidates it after rectangle
changes. SetInputShape therefore remains unsupported for the current ordinary
application; the public availability note now explains the restriction.
Approved restricted-ACL applications, exact polygon-to-rectangle limits and their
restoration behavior remain open investigations. This checkpoint does not label
those scenarios impossible or complete.

Custom-title double-click is still open. Its maximize-enable policy must be
implemented coherently with SetMaximizable/IsMaximizable before claiming support;
those methods remain stubs. No nonfunctional action gated by an always-false flag
is committed. Starting resize from an arbitrary selected edge is also open; merely
enabling native resize handles does not implement that operation.

## Reproduction, evidence and checks

With one named unlocked disposable guest, from core:

```sh
NATIVEAPI_TEST_SUITE=window-drag \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

Evidence is retained remotely under
`~/Library/Huawei/Logs/window-drag-validation/{desktop,phone}/` and copied locally
to ignored `core/build-ohos/verification/window-drag-validation/`. It contains
native logs, normal bundle metadata, per-gesture owner and rectangle results,
WMS/layout dumps and decoded screenshots. Initial computer result directory:
`2in1-1792082791939616000`; final computer touch/mouse:
`2in1-1792083223826537000`; phone: `phone-1792083024328485000`.

Computer general/window regression **PID 23904** passes **269 checks**: 201 base,
52 picker, 5 dialog UI, 8 picker UI and 3 actual title-bar drag/maximize/restore
checks, zero failures. Its normal result window remains visible in NativeAPI_Desktop.

macOS CTest passes 31/31. The generator, raw Rust bindgen and Dart ffigen steps
succeed; check reports 75 C ABI and 339 binding/reference files current, with the
same eight baseline skips. Local Rust, Dart, C#, JS, Python and Go checks pass;
JS has 4 passed/1 skipped and Python 26 passed/1 skipped. These local language
checks do not prove every binding's runtime integration inside HarmonyOS.
The HAP builds ARM64 and x86_64; runtime acceptance here is ARM64 only. The added
PowerShell mode is not run or parser-checked (no local PowerShell). Other device
versions, pointer-source variants, races and broader lifecycle/binding/window
capability audits remain open.
