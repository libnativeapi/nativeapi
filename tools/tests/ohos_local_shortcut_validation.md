# HarmonyOS application-local shortcuts — 2026-10-16

Application scope now uses normal ArkUI component input through the canonical
NativeShortcutHost companion, with event-time modifier state. It never registers
or probes global hotkeys. A different UID can own all four Ctrl/Alt variants
while this application's local registration exists, and local registration also
succeeds after that other application already owns the global chord.
The complete HarmonyOS API objective remains active.

## Acceptance

macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26, ARM64 guests:

| Guest | Local source / peer PID | Source / peer UID | Real local-scenario chords | Native assertions | Global regression PID / assertions |
| --- | --- | --- | ---: | ---: | --- |
| NativeAPI_Desktop / 2in1 | 7204 / 7773 | 20020058 / 20020061 | 16 | 102 | 7859 / 60 |
| NativeAPI_Phone | 3731 / 4204 | 20020074 / 20020075 | 16 | 102 | 4287 / 60 |

Both guests pass with zero failed assertions. The normal global registration
suite separately passes 67 assertions on each. Local scenarios end with eight
business callbacks, nine public activation events, five successful registrations
and five removals, zero remaining registrations, and exactly one C callback
user-data release. The ninth activation event deliberately unregisters its
shortcut, suppressing the business callback. Later finally cleanup adds two
successful cleanup assertions beyond the reported 102.

The sixteen real Ctrl+Alt+F9 chords cover all four left/right Ctrl/Alt pairings;
normal peer foreground input without any global reservation; peer-owned global
activation; disabled manager and disabled C alias; host detachment/remount;
callback self-removal and subsequent silence; C-registered actual callback and
user-data lifetime; worker registration; and activation-listener removal before
the business callback. Disabled scenes require a positive component key-event
log with the correct F9/modifier snapshot and consumed=false, not only an unchanged
business counter. Normal background input requires a positive real peer component
F9 event while the source remains unchanged, before any source Want changes focus.
The peer is independently packaged, has a distinct UID and contains no native .so.

The existing global scenario still passes nine actual chords, six business/public
activations and nine independent NDK-reference callbacks on each guest, including
actual background delivery and SDK-positive disabled/unregistered negative cases.
It uses the same extracted guarded input helper as the local scenario.

## Implementation and SDK basis

Shared ShortcutManager registration retains an atomic in-process duplicate check,
but only Global scope calls the platform's global availability probe. Actual
platform registration validates local scope, canonical aliases and host presence.
A controlled shared seam test refuses all global probes and verifies successful
local registration, local duplicate refusal and platform-failure rollback without
any global probe. No public C++ signatures or C ABI symbols change; the OHOS
availability comment and generated API reference are updated.

The private AttachLocalKeyHost/DetachLocalKeyHost/DispatchLocalKey seam is bridged
by the embedding N-API module. The canonical ETS adapter reads normal component
KeyEvent.getModifierKeyState for Ctrl/Alt/Shift and metaKey for rejecting unsupported
Meta chords. The root forwards onKeyPreIme and disposes its host on disappearance.
A mounted host is required; absent-host local registration fails. Exactly one
host is permitted per attached SDK window, and a remount receives a new token.
Native-only component hosts can forward the equivalent real snapshot through
this private seam, but that integration is not yet separately verified.

API 26 UIContext.addLocalInputEventMonitor was investigated and deliberately not
used: its raw wrappers cannot provide valid metaKey/getModifierKeyState at that
pre-dispatch stage. This limitation is documented in the installed SDK
ets/component/common.d.ts and the [official input monitor reference](https://developer.huawei.com/consumer/cn/doc/doccenter-capabilities/api/ts-inputeventmonitor).
Normal component KeyEvent documents the modifier snapshot in the installed SDK
common.d.ts; the four actual hardware-side variants confirm it in the guest.
Tracking modifier presses alone would lose correctness across focus changes.

Callbacks queue to UI and recheck registration activity, manager/shortcut enabled
state, the non-reused host token, native window lifetime and actual SDK focus.
Normal repeated key-downs are suppressed until key-up. SDK focus changes clear
held-key state; native destruction/environment cleanup forget the window's hosts.
No UI dispatch is awaited while holding the shared manager mutex during worker
local registration/removal. The registry outlives C++ singleton teardown so late
SDK environment cleanup cannot access a destroyed registry. Full teardown and
repeat/focus timing are still separate runtime audits.

Global IsAvailable remains a global-reservation query. Another application's
global handler may consume input before a focused local component receives it;
local scope does not override system/other-app global priority. Components that
consume an event before it reaches the configured root retain priority. Meta/Fn/
lock modifiers and release events remain unsupported. Bare-key registration is
implemented but no bare-key input is injected under this chord-only authorization.

## Authorization, reproduction and evidence

The human explicitly approved only Ctrl+Alt+F9 inside NativeAPI_Phone and
NativeAPI_Desktop. Both runners require --allow-guest-keyboard and reject other
instance names. Before every chord the shared helper verifies the one running
emulator instance, source/peer bundle, process command line, UID and the actual
WindowManagerService focused window owner. It injects only a bounded guest
uinput chord and releases all keys again in finally. No macOS keystrokes or
NativeAPI_Phone24 input are used. Normal run.sh/run.ps1 registration sends no keys.

After normal registration builds and installs the two fixtures:

```sh
python run_local_shortcut_suite.py "$DEVECO_STUDIO_HOME" --allow-guest-keyboard
python run_shortcut_input_suite.py "$DEVECO_STUDIO_HOME" --allow-guest-keyboard
```

Remote archives: ~/Library/Huawei/Logs/local-shortcut-validation-20261016/.
Ignored local copies: core/build-ohos/verification/local-shortcut-validation-20261016/.
Each desktop/phone input/global directory contains per-command counters/logs,
per-chord focus dumps and owner JSON, source snapshots and SHA-256 manifest,
source/SDK-peer HAPs and their SHA-256. Snapshots and HAP hashes are checked after
copying back. The desktop direct-SDK component probe is preliminary evidence only;
the complete final local scenario above is authoritative.

## Host checks and remaining audits

Local macOS CTest: 33/33. Rust workspace check, Dart analysis, C# solution build
(0 warnings/errors), JS native/type build/tests (4 passed, 1 baseline skip), Python
native/lint/tests (26 passed, 1 baseline skip), and Go native build/tests all pass.
Full codegen emits 75 C ABI and 339 binding/reference files with the same eight
baseline skips. Raw bindgen/ffigen produce no API changes; codegen check passes.
These are host-language checks, not OHOS runtime verification of every binding.
Windows/Linux/Android/iOS runtimes and PowerShell runner were not run here.

Additional windows and native-only hosts, held-key focus/remount transitions,
repeat timing, extra modifiers/layouts, queue cancellation under teardown,
concurrency, global C-registered actual input and full KeyboardMonitor remain
open. This checkpoint does not close the module or full HarmonyOS goal.

Final normal computer regression: PID **3436**, **269 checks**, zero failures:
201 native integration + 52 native picker + 5 dialog UI + 8 picker UI + 3 real
window drag/maximize/restore checks. Archive: desktop-regression-3436 under the
same evidence base. Its HAP SHA-256 is
2ba55d33da68810fe560ecd64601c9e74e125ddba22829f134b86d5b7cd33df2.
Only fresh current-process picker, dialog and window evidence is copied. The
computer guest and passing app remain running; phone images/data are retained.

Core checkpoint: `ece3e1b4a`. The workspace sync includes the core pointer, generated
API reference, binding changelogs and these validation reports. Nothing is pushed.
