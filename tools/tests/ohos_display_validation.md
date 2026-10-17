# HarmonyOS live display observers — 2026-10-16

The OHOS DisplayManager now observes public NDK display configuration, available
area, addition and removal callbacks. Native IPC callbacks schedule work on the
owning main dispatcher. Each subscription period owns its cancellation state;
removing the final listener unregisters successful registrations and invalidates
already queued work. Independent announced identities preserve add/remove diffs
when ordinary GetAll/GetPrimary queries run before a queued native notification.
Registration indices are optional values, so SDK index zero is valid. Registration
and unregistration failures are logged with their actual SDK result.

The shared enumeration seam now distinguishes failure from a successful empty
topology. Failed/partial enumeration returns no query result while retaining
cached identities and suppressing false disconnects. A standalone controlled-seam
test exercises failure, recovery with the same object/id, a successful empty
snapshot and reconnection with a new identity. This is a shared algorithm test,
not native SDK fault injection. All six platform implementations use the revised
private seam; public signatures and generated ABI signatures are unchanged.

## Actual guest evidence

Host: macbook-pro ARM64; DevEco Studio 26.0.0.851. The two named guests run
HarmonyOS 7.0.0.107/API 26, one at a time, preserving their data. API 24 is not
part of this checkpoint.

| Guest | Process | Actual changes | Result |
| --- | ---: | --- | --- |
| NativeAPI_Desktop | 16505 | 8 available-area transitions; 1 old notification held | 444 native checks, zero failures |
| NativeAPI_Phone | 3476 | 8 rotations, 8 configuration + 8 area notifications; 2 old notifications held | 437 native checks, zero failures |

Computer final nativeapi counts: base/typed/async C each 6, synchronous C 5.
Phone counts: base/typed/async C each 12, synchronous C 10. The deliberately
unsubscribed transition and cancelled queue account for suppressed events;
the C-only removal accounts for the additional synchronous-C difference.
Both final snapshots have zero observers and zero pending notifications.

The computer guest does not support the Emulator rotation scenario. It does
produce real SDK available-area changes when its own main window enters and
leaves full screen: `[0,0,3120,1955]` becomes `[0,0,3120,2080]`, then returns.
That actual geometry change is required for a positive pass.

Both scenarios compare independent direct SDK listeners against nativeapi C++
base events, typed events, synchronous C events and asynchronous C delivery
leases. Each held C++ object and generated C handle must preserve the connected
primary identity. Live size/density, orientation, refresh rate, copied name,
primary flag and work area agree with separate SDK queries; C++/C positions
agree and unavailable bit depth remains zero. Tests remove just the C observer,
remove all observers on a worker while another SDK transition occurs, subscribe
again on a worker, cancel queued callbacks and verify fresh observers still work.
No add/remove callback is fabricated and no hotplug success is inferred from
successful registration.

The queued-cancellation scene uses a fixture-only dispatcher installed once at
module startup, before subscriptions. Its normal path forwards directly to the
real OHOS NAPI scheduler. During this one scene it retains nativeapi tasks while
the UI remains free to finish an actual window/rotation transition. Both an
independent SDK callback delta and a positive production pending-notification
count are required before replacing subscriptions. Old queued tasks are then
drained on UI, without delivering to either old or replacement subscribers.
This tests cancellation with real SDK notices; it does not claim a blocked UI
can finish a window transition. A previous computer attempt that blocked UI
failed its pending-work assertion and was replaced by this explicit scheduler.

Commands are allowlisted Want parameters inside the verified disposable test
Ability, identified by PID/UID and a unique request nonce. No host pointer or
keyboard input is used. Each transition stores SDK JSON, native logs, display
and window service dumps and screenshots. Original geometry is restored in a
finally block, and final observer counts must be zero.

## Reproduction

With one unlocked NativeAPI_Phone or NativeAPI_Desktop running:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=displays \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

The runner selects `rotation` for phone and `fullscreen` for computer using the
actual guest type. `NATIVEAPI_DISPLAY_MODE=quiet` is a separate registration-only
scenario, with a distinct result marker; it cannot satisfy either positive suite.
PowerShell has the same selection branch; its runtime is untested this checkpoint.

## Source and binding verification

Installed public SDK headers: `window_manager/oh_display_manager.h`, the
DisplayChange/Add/Remove listener APIs and AvailableAreaChange listener API.
The existing OHOS target already links the native display manager library.
The public DisplayManager notes and generated API reference describe event
availability and main-dispatcher requirements. The platform seam spec records
failure-versus-empty and the independent announced snapshot.

Local macOS CTest passes 32/32, including the new enumeration-failure regression.
Full codegen/check passes 75 C ABI and 339 binding/reference files, with the same
eight baseline skipped symbols. Rust bindgen and Dart ffigen leave raw FFI
unchanged. Rust workspace check, Dart analysis, C# build, JS type/native
build/tests (4 passed, 1 baseline skip), Python native build/lint/tests (26 passed,
1 baseline skip) and Go native build/tests pass. These binding checks run on
macOS, not inside HarmonyOS. Windows/Linux/Android/iOS seam changes are inspected
but their native builds and runtimes were not exercised this checkpoint.

Remote evidence: `~/Library/Huawei/Logs/display-validation-20261016/`.
Local copy: ignored `core/build-ohos/verification/display-validation-20261016/`.
The current successful runs are under `desktop/` and `phone-final/`.

Final normal computer regression (PID **3196**) passes **201 integration + 52
native picker + 5 dialog UI + 8 picker UI + 3 desktop-window UI = 269 checks**,
zero failures. Actual title-bar movement, maximize and restore pass. The computer
guest remains running on its passing fixture; the phone is stopped with its data
preserved. Full regression logs and ownership/geometry records are archived under
`desktop-final/`.

## Open coverage

Actual display add/remove/hotplug, multiple displays and mixed density,
non-primary origin/orientation/identity behavior, native-ID reuse and retained
removed wrappers remain open. Native SDK registration/enumeration/unregistration
fault injection, callback/re-subscribe races and environment teardown also need
further evidence. The previous cursor missing-pointer checkpoint remains valid;
positive pointer positions and focused-pointer authorization denial stay open.
No full Display/DisplayManager or whole HarmonyOS API completion is claimed.

Core implementation and fixture commit: `f0adb42`. Changes are local; nothing
has been pushed. The full HarmonyOS API goal remains active.
