# HarmonyOS cursor query checkpoint — 2026-10-16

`DisplayManager::GetCursorPosition()` now queries the public Input Kit
`OH_Input_GetPointerLocation` instead of always returning the origin. It reads
current display metadata, converts local physical pixels to logical coordinates,
and accounts for the primary and secondary display origins. Secondary-display
position lookup is optional at runtime (API 26); missing geometry or any SDK
failure returns the documented `(0,0)` default. No pointer position is cached and
no extra input-device permission is requested. C ABI and all existing signatures
remain unchanged. Public platform notes and the generated API reference are current.

## Actual emulator evidence

Host: macbook-pro ARM64, DevEco 26.0.0.851. Both guests run HarmonyOS
7.0.0.107/API 26; the release API 24 guest is not part of this checkpoint.

| Guest | Process | Explicit scenario | Result |
| --- | ---: | --- | --- |
| `NativeAPI_Desktop` | 24347 | `--expect-pointer absent` | 38 checks passed |
| `NativeAPI_Phone` | 3369 | `--expect-pointer absent` | 38 checks passed |

Each scenario uses three real owned button presses, independently delivered ArkUI
click coordinates and fresh native window/process/UID ownership. Desktop presses
use one complete guest mouse click; phone presses use the named guest's touch
input. Direct SDK calls before/after each sample and on a worker return
`INPUT_DEVICE_NO_POINTER` (3900010). C++ and generated C ABI, on both UI and worker
threads, agree on safe defaults. The window then really loses focus (the native
live query reports false), all four paths again match the SDK, and focus is
restored with a new sample. Actual event positions and focus transitions are
checked separately from the query's default values.

This is **not positive cursor-position verification**. Host mouse hover also left
the guest SDK reporting no pointer. The missing-pointer result takes precedence
in the tested unfocused state, so that state does not prove the separate
`INPUT_APP_NOT_FOCUSED` authorization-denial branch. The test's explicit `present`
scenario still requires three distinct nonzero positions, SDK agreement with real
press/event coordinates, rejection after focus loss, and recovery. It cannot be
satisfied by an ABSENT_PASS marker. Positive pointer hardware, authorization
refusal with a pointer, multiple displays/mixed density and unavailable-position
fallback remain open. Display enumeration/hotplug/configuration events and the
rest of the Display/DisplayManager audit also remain open.

## Reproduction and artifacts

After building and installing the normal HAP, with exactly one unlocked named
NativeAPI guest running:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d/tests/ohos_smoke
~/Library/Huawei/test-tools/bin/python run_cursor_suite.py "$DEVECO_STUDIO_HOME" \
  --expect-pointer absent
```

Use `--expect-pointer present` only in an environment with a pointer device;
it keeps the stronger success/authorization assertions. The default full smoke
suite is unchanged. The independent cursor scenario retains actual SDK codes,
C++/C/worker results, event coordinates, native focus and ownership snapshots,
packed-bundle metadata and decoded screenshots.

Remote evidence: `~/Library/Huawei/Logs/cursor-validation-20261016/`.
Local copy: ignored `core/build-ohos/verification/cursor-validation-20261016/`.

## Source and binding verification

The public SDK [Input Kit reference](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md#oh_input_getpointerlocation)
documents the pointer query, focused-app authorization and missing-pointer result.
The installed native header agrees and names `libohinput.so`; the core target now
links that library. The [Huawei input FAQ](https://developer.huawei.com/consumer/cn/doc/doccenter-dev-faq/faqs-input-3)
also describes emulator mouse capability limits; observed runtime codes above
are the evidence for these specific guests.

Code generation: 75 C ABI files and 339 binding/reference files current, eight
baseline skipped symbols and no new skips. Rust bindgen and Dart ffigen produce
unchanged raw FFI. Local macOS core CTest: 31/31. Rust workspace check, Dart
analysis, C# build, JS type/native build/tests (4 passed, 1 baseline skip), Python
native build/lint/tests (26 passed, 1 baseline skip), and Go native build/tests
pass. Those are macOS binding checks, not HarmonyOS binding-runtime coverage.

Final normal computer regression after the implementation: process **3085** passed
**201 integration + 52 native picker + 5 dialog UI + 8 picker UI + 3 desktop-window
UI checks = 269**, zero failures, at 03:39. Real title-bar movement, maximize and
restore passed. The computer guest remains running on its passing fixture; the
phone guest is stopped with its data preserved. The final archive includes the
full regression logs and ownership snapshots under `desktop-final/`.

Core implementation and fixture commit: `179492d`. Changes are local; nothing
has been pushed. This checkpoint does not close the full HarmonyOS API goal.
