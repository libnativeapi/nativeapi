# HarmonyOS public global shortcuts — 2026-10-16

The OHOS ShortcutManager backend now registers real public NDK hotkeys instead
of returning false for every registration. It parses the existing accelerator
vocabulary into native key codes, rejects unrepresentable modifier combinations,
and registers each left/right Ctrl/Alt/Shift variant. Native registration failures
roll back completed variants. Failed native cleanup retains ownership for retry;
public unregistration failure preserves the identity and callback until a later
successful retry. The SDK service and callback registry are initialized while
the manager singleton constructs, keeping final unregistration inside their
lifetimes.

SDK callbacks copy registration ownership and dispatch through the active main
scheduler, with emitter and per-registration guards. Activation emits its event
on UI before invoking the callback, rechecking removal/disabled state afterwards.
Removing an event listener does not unregister the separately owned callback.
Native hotkeys report activation only; no release event is fabricated.

Availability normalizes aliases/order and performs a short-lived native probe
with no user callback. This checks system reservations and other applications,
which a registry-only or built-in-system-key list cannot prove. Another process
can still race a later Register call, whose SDK result remains authoritative.
No public signatures changed. The internal backend preflight has a default
implementation preserving the existing registry-only behavior on other platforms.

## Authoritative SDK references

The installed Input Kit headers and the [public NDK reference](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md)
provide Create/DestroyHotkey, Set/GetPreKeys, Set/GetFinalKey, Add/RemoveHotkeyMonitor
and the SDK conflict results, available since API 14. Global hotkey subscription
does not require the separate full-keyboard INPUT_MONITORING permission. The
[InputConsumer reference](https://raw.githubusercontent.com/openharmony/docs/master/en/application-dev/reference/apis-input-kit/js-apis-inputconsumer.md)
describes the independent ArkTS subscription and occupied-by-other error.

GetPreKeys takes caller-owned pointers to individual integer outputs rather
than an allocated flat array. The [NDK implementation](https://github.com/openharmony/multimodalinput_input/blob/master/frameworks/native/input/oh_input_manager.cpp)
and its SDK tests were inspected, and twelve native object round trips verify
the mapping against the installed SDK. Four output pointers cover the documented
maximum modifier count. Both modifier sides are registered, rather than assuming
left codes also match right keys.

## Registration evidence

Host: macbook-pro ARM64, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107/API 26
phone and computer guests. API 24 is not part of this checkpoint.

| Guest | Native / peer process | Native / peer UID | Result |
| --- | --- | --- | --- |
| NativeAPI_Desktop | 4575 / 4797 | 20020058 / 20020061 | 67 native assertions, 13 test commands, zero failures |
| NativeAPI_Phone | 15057 / 15207 | 20020074 / 20020075 | 67 native assertions, 13 test commands, zero failures |

Both final snapshots show three registered and three unregistered lifecycle
events, eleven rejected registrations, zero remaining registrations and zero
activation/SDK input callbacks. Two further idempotent cleanup commands pass.

The independent `dev.nativeapi.clipboard.peer` fixture now also imports the
public ArkTS InputConsumer module. It packages no native shared libraries and
has a different UID/PID. Tests reserve Ctrl+Alt+F9 on each side and require error
4200003 from the other process. After native release the peer acquires all four
variants; the native availability/registration paths reject that occupancy.
After peer release the native backend must register and release them again.
No keyboard input is needed to prove actual resource ownership and reuse.

Other assertions cover C++/C/worker registration, metadata, held identities,
Get/GetAll/GetByScope, alias/order conflicts, rejected syntax/modifiers, enabled
state, ended callback invalidation and asynchronously drained lifecycle events.
A successful run ends with no native manager registration or peer reservation.
The result marker is specifically NATIVEAPI_SHORTCUT_REGISTRATION_PASS and
requires zero actual activation callbacks.

## Reproduction

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=shortcuts \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

The normal runner builds both fixtures and validates their identities. It accepts
only one unlocked named disposable NativeAPI guest. PowerShell has a matching
entry point, but its runtime is not tested in this macOS-host checkpoint.

## Shared regression and downstream checks

Core CTest passes 33/33, including an isolated fake-backend failure regression.
Refused unregistration and UnregisterAll preserve both registry lookups and
callbacks for retry. A successful retry clears retained callbacks and identities.
With no dispatcher, invalid/unavailable/native-refused registration callbacks may
re-enter manager queries without deadlocking; failure events release the manager
mutex before the generic dispatcher's synchronous fallback. These are shared
algorithm tests, not SDK fault-injection evidence.

Full codegen/check passes 75 C ABI and 339 binding/reference files, with the same
eight baseline skipped symbols. Rust bindgen and Dart ffigen leave raw FFI current.
The public platform notes and generated references are updated. Rust workspace
check, Dart analysis, C# build, JS type/native build/tests (4 passed, 1 baseline
skip), Python native build/lint/tests (26 passed, 1 baseline skip) and Go native
build/tests pass locally on macOS. Other native platforms retain their existing
backend behavior and are not newly verified on physical Windows/Linux/mobile
hosts by these macOS checks.

Remote evidence: `~/Library/Huawei/Logs/shortcuts-validation-20261016/`.
Local copy: ignored `core/build-ohos/verification/shortcuts-validation-20261016/`.
The latest registration runs are `desktop-final-registration/` and `phone/`.

Final normal computer regression, PID **9231**, passes **201 integration + 52
native picker + 5 dialog UI + 8 picker UI + 3 desktop-window UI = 269 checks**,
zero failures. Actual movement, maximize and restore pass. The computer guest
remains running on its passing fixture; the phone is stopped and its data is
preserved. `desktop-regression/` archives current-run logs, fresh ownership/
geometry records and the native/peer HAP hashes.

## Remaining scope and input verification

The human approved the isolated guest-keyboard exception on 2026-10-16. The
separate [actual input acceptance](ohos_shortcut_input_validation.md) now passes
on NativeAPI_Phone (PID 8204) and NativeAPI_Desktop (PID 3556): each has nine actual
Ctrl+Alt+F9 inputs, six business callbacks/activation events, nine direct SDK
callbacks and sixty native assertions, zero failures. Foreground/background,
disabled manager/shortcut, self-removal and no delivery after removal are proven.
The opt-in scenario verifies exact guest instance/PID/UID/native focus, releases
the full chord in finally and checks SDK delivery during negative cases. Normal
registration runners still send no keys. The source snapshot and full evidence
are linked in that acceptance report. Its INPUT_PASS is separate from the older
registration-only PASS above.

Application-local scope was open at this global-registration checkpoint; the
follow-up linked below now implements a genuinely local component host without
reserving other applications' global chords. Full keyboard monitoring, permission policy,
release delivery, layouts/physical input variants, SDK fault injection and late
SDK/teardown races also remain open. Neither the full module nor the overall
HarmonyOS API goal is complete.

Core implementation and fixtures: `e5a63f4`. Changes are local; nothing has been
pushed. The full HarmonyOS API goal remains active.

The later [application-local shortcut checkpoint](ohos_local_shortcut_validation.md)
adds host-based Application scope. The original rejection above was observed
without a mounted local key host; it remains the expected absent-host behavior.
With the canonical host, both API 26 guests now pass 102 assertions and sixteen
real input chords, including local registration while a different UID owns the
global chord. Global C-registered actual input and remaining audits stay open.
