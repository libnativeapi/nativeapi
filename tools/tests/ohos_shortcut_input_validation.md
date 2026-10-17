# HarmonyOS real shortcut input — 2026-10-16

The human explicitly approved Ctrl+Alt+F9 input only inside the isolated
NativeAPI_Phone and NativeAPI_Desktop test guests, with the test process and
native focus checked. This is the requested exception to the GUI skill's default
no-keyboard rule. No host keystroke or other guest/combination is used.
The broader HarmonyOS API objective remains active.

Core runner/guard checkpoint: `e4c70db`; the production global-hotkey backend is
unchanged by this input-only checkpoint.

## Acceptance

macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26 ARM64:

| Guest | Input PID | Registration assertions | Actual chords | Business callbacks / activation events | Direct SDK callbacks | Result |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| NativeAPI_Phone | 8204 | 67 | 9 | 6 / 6 | 9 | 60 native input assertions, 0 failed |
| NativeAPI_Desktop / 2in1 | 3556 | 67 | 9 | 6 / 6 | 9 | 60 native input assertions, 0 failed |

Four initial foreground inputs cover every left/right Ctrl and Alt pairing.
Each produces exactly one business callback, one public ShortcutActivatedEvent
and one separately registered direct-NDK reference callback. The fifth input
focuses the independently packaged SDK-only peer app (different PID/UID), and
still reaches the native global shortcut. The source's actual Window.IsFocused
is false in its callback. The runner observes delivery through logs before
sending any Want that would bring the source back to the foreground.

Two inputs disable the manager and the held shortcut (through its C alias),
respectively. The SDK reference count advances to six and seven, proving keys
arrived, while business/activation counts stay at five. Reenabling restores
delivery. The eighth input invokes a callback that unregisters itself on UI:
both business and public activation counts become six, with no registrations
left. The ninth input still reaches the SDK reference, while the ended native
registration produces no callback/event. All native callbacks verify UI delivery.
The normal registration suites separately recheck actual resource ownership,
cross-app conflicts and release/reuse, passing 67 assertions on each guest.

The direct reference is a separate public-NDK monitor in the source process,
not a second language binding or a fabricated activation. The peer is used to
prove background delivery; its packaged HAP contains no native shared libraries.
The existing production global-shortcut backend required no change for these
successful input scenes. C++ registration/callbacks and C alias enabled-state
control are exercised; a separately C-registered actual callback remains an audit.

## Input guards and evidence

The runner requires `--allow-guest-keyboard`; normal smoke/registration runners
never add it. It accepts exactly the two authorized instance names, confirms a
loopback emulator model and the single running instance, verifies bundle PID/UID
from /proc and bundle metadata, then checks the actual focused WindowManagerService
window belongs to that test process immediately before input. Only the source
fixture and its SDK-only peer can be selected. A bounded guest uinput command
presses/releases Ctrl, Alt and F9, with unconditional additional releases in
finally. The installed SDK confirms key codes 2072/2073 (Ctrl), 2045/2046 (Alt),
2098 (F9). This does not route input through the macOS keyboard or its foreground app.

Remote archives:
`~/Library/Huawei/Logs/shortcut-input-validation-20261016/{phone-8204,desktop-3556}/`.
Ignored local copies are under
`core/build-ohos/verification/shortcut-input-validation-20261016/`.
Each contains actual per-command counters, per-chord native focus dumps/logs,
source/peer HAPs and SHA-256, production/fixture/runner snapshots, and result JSON.
The desktop also records explicit per-chord guest, bundle, PID/UID, focused-window
ID and key-code JSON. Phone guards execute the same checks before input; its
earlier runner snapshot predates that additional JSON logging and formatting.

After the normal registration fixture has built/installed both HAPs, an explicitly
authorized run is:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d/tests/ohos_smoke
"$HOME/Library/Huawei/test-tools/bin/python" -u run_shortcut_input_suite.py \
  "$DEVECO_STUDIO_HOME" --allow-guest-keyboard
```

Runner lint and formatting pass. Production symbols/signatures and generated
bindings are unchanged by this input checkpoint. The accompanying system-close
checkpoint records the current CTest 33/33 and all six language-binding checks.
Final computer normal regression (PID 5943) passes all 269 native/picker/dialog/
window UI checks, with zero failures; its exact HAP and evidence are archived by
the [system-close checkpoint](ohos_native_close_validation.md).

Application-local registration must use a real window-local observer and remains
unimplemented; globally reserving then filtering would interfere with other apps.
Full keyboard monitoring, release events, physical/layout variants, independently
C-registered callback delivery, concurrent queued unregistration/disable, SDK
failure injection and complete environment/process teardown remain open. These
results close the previously pending real global-key input scenarios, not the
entire shortcut module or the full OHOS goal.
