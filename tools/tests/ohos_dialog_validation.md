# HarmonyOS native message dialogs — 2026-10-15

This checkpoint implements MessageDialog through public Native ArkUI controls.
The full HarmonyOS API goal remains open; see [the coverage ledger](ohos_api_coverage.md).
Public signatures are unchanged. The public availability and modal-call contract
are documented in `core/src/message_dialog.h` and regenerated into the API reference.

## Implementation

The dialog uses a native controller, text nodes, buttons, TextInput, Checkbox,
Progress and LoadingProgress. Optional controls can be hidden. Title/message,
input, checkbox and progress can update while displayed; changing progress does
not rewrite the input field or its selection. Buttons/default button/parent are
configured before Open. Labels and input reject embedded NUL, and progress rejects
invalid/non-finite values.

The parent's public `UIContext.runScopedTask` scopes native node creation to its
actual window. An unset parent resolves to the current attached visible window.
Opaque callback tokens protect C++ models from late SDK callbacks. Node/controller
ownership is released after native disappearance, on object destruction, on parent
closure, and during environment cleanup. The environment-cleanup hook is reviewed;
actual environment teardown is not an independent runtime assertion in this suite.

Non-modal Open returns after submission. Modal Open blocks a worker until the
native dismissal callback; the ArkTS UI thread must keep pumping events, so modal
Open there returns false. Application modality disables focus/pointer input on
other attached windows and on windows attached later. Counted leases keep those
windows blocked while overlapping application dialogs remain open; the final
release restores the original policies. Unattached windows from other nativeapi
instances are outside this boundary.

SDK sources: `arkui/native_dialog.h`, `native_node.h` and
`ets/api/@ohos.arkui.UIContext.d.ts` from DevEco's API 26 SDK. The public UI scope
method avoids allocating an opaque native context wrapper with no public release
operation. The upstream [native conversion](https://github.com/openharmony/arkui_ace_engine/blob/master/interfaces/native/node/native_node_napi.cpp)
was inspected for ownership; no private SDK structs are accessed by this backend.

## Verification

Both guests run on macbook-pro, DevEco Studio 26.0.0.851, API 26, ARM64.
Final results (October 15, Asia/Shanghai):

| Guest | Integration suite | Guest dialog UI | Final process/time |
| --- | --- | --- | --- |
| NativeAPI_Phone | 188 passed, 0 failed | 5 passed, 0 failed | PID 6648, 01:00:40 |
| NativeAPI_Desktop | 201 passed, 0 failed | 5 passed, 0 failed | PID 3374, 01:03:16 |

The computer guest additionally passed its 3 native window interaction checks.
The total includes native clipboard callbacks, which can coalesce; repeated runs
can differ slightly in count. A missing PASS marker is always a failure.
The final desktop emulator and passing app remain open; the phone instance is retained.

The HAP also compiles x86_64 libraries; this checkpoint does not claim x86_64 runtime validation.

The fixture retains every earlier service/window assertion and adds native dialog
checks for real primary/secondary/close clicks, actual checkbox state, Unicode
prefilled input, native progress, hidden optional controls, live updates, reopening,
programmatic close and duplicate-close rejection. Worker modal waiting is checked
while the UI loop remains responsive. Destruction, parent closure, creation during
application modality, overlapping modal leases and restoration of prior native
input policies are exercised. Generated C ABI checks configure/open/close the real
native dialog, read owned strings/state, and reject released handles.

The guest UI driver validates the emulator identity, test PID, bundle and native
window ownership before each click. Widget coordinates come from the current
native tree. A final UI tree confirms the PASS report and absence of leftover
dialogs. No host desktop input or keyboard injection is used. The computer guest
also runs the three existing native title-bar drag/maximize/restore checks.

Reproduce on an unlocked isolated NativeAPI phone or computer guest:

```bash
cd core
fastforge --no-version-check host exec macbook-pro -- true
# On macbook-pro, in its synced core mirror:
./tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
# Computer only, after PASS:
python3 tests/ohos_smoke/verify_desktop_ui.py /Applications/DevEco-Studio.app/Contents
```

The runner invokes `verify_dialog_ui.py` automatically. Python 3 is required.
Bash/Python syntax checks passed. The updated Windows runner was parsed by Windows
PowerShell; its runtime suite was not run for this checkpoint.

All six binding/reference generators ran, with no newly skipped symbols. Rust
bindgen and Dart ffigen ran and left their raw ABI outputs unchanged. `./codegen
check` confirmed 339 files current. Host checks passed for Rust workspace, Dart
analysis, C# solution (zero errors/warnings), JS (4 passed, 1 existing desktop
clipboard skip), Python lint/native build/tests (26 passed, 1 existing desktop
clipboard skip), and Go native build/tests. Language runtimes were checked on the
local macOS host, not inside HarmonyOS; the HAP exercises C++ and C ABI there.

Remote evidence: `~/Library/Huawei/Logs/dialog-validation/{phone,desktop}/`, plus
`nativeapi-dialog-{phone,desktop}.log` in the Logs root. Retained evidence includes
build/runtime logs, native widget/ownership snapshots and HAP hashes. Local copies
are under ignored `core/build-ohos/verification/dialogs/`.

Remaining dialog audit: arbitrary long content/tiny windows, native IME editing,
accessibility/theme behavior, failure injection, and actual UI-environment teardown.
File dialogs, notifications, menus, views and the other open ledger groups remain
part of the original goal.

Validated core revision: `8cdebf8`.
