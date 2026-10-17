# HarmonyOS native drop destinations — 2026-10-16

This checkpoint implements DropTarget through the public native ArkUI drag/drop
API on macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26 ARM64.
Implementation: core `eebbfa770`; committed locally, not pushed.
It does not complete the full OHOS API goal.

The host mounts the NativeAPI content root using NodeContent/ContentSlot before
adding a listener. The first listener registers native enter/move/leave/drop
handlers and supported UDMF types. Only one destination owns a root. Removing the
last listener removes only these handlers/receiver; an environment cleanup hook
also releases registrations before native views are disposed.

The public SDK exposes a suggested Copy/Move operation rather than an offered
operation mask. This backend currently accepts Copy and refuses None/Move/Link.
Arbitrary ArkTS-only/Flutter content is not automatically wrapped. Children with
their own destinations take precedence; that overlap case remains untested.

Dropped data is read from public UDMF objects. File URI entries can be secondary
entries of a general.file record, so every record is inspected rather than gated
by the top-level data type. Returned paths go through the same validated,
readability-checked URI conversion as the clipboard. Physical window-relative
positions are converted into logical content coordinates using live drawable
bounds and window scale.

Native ArkUI broadcasts a node's events to all its registered receivers. Testing
found and fixed a crash where the existing View receiver interpreted a drop's
numeric target ID as a text change. View and DropTarget now filter event types
before interpreting their module-local tokens/IDs. Shared enter/move methods
snapshot their result before synchronous user callbacks, and DropTarget shuts
down listeners before releasing its platform state. The real fixture destroys a
target from its enter callback without a stale delivery or process crash.

## Actual acceptance

The source is an ArkTS SDK Text component, independent of the NativeAPI backend;
no native drag event is fabricated. Thirteen real touch gestures per guest cover:

- C++ and C ABI Unicode text, mixed text/two files, and files without text.
- Exact contents of both files, including Chinese, spaces and URI punctuation.
- Native enter/move/drop delivery, UI thread and actual window identity.
- Logical drop coordinates checked against physical gesture endpoints and SDK
  drawable geometry; the point is not inferred from an image.
- Enter/move/exit without a drop by moving into the app's own bottom padding.
- None/Move/Link refusing every event for an incoming SDK Copy drag.
- Duplicate destination refusal without disrupting the original destination.
- Last-listener removal, re-registration, destruction during enter, C ownership
  handoff and no further delivery after removal.

Every gesture validates the named disposable guest, bundle PID/UID and fresh
native window ownership across the gesture path. An atomic guest touch-drag
releases the contact; no host desktop input or keyboard input is sent. C empty
text is checked using the existing null-pointer-for-empty-string ABI convention.

| Guest | PID / UID | Real gestures | Native assertions | Result |
| --- | --- | --- | --- | --- |
| NativeAPI_Desktop / 2in1 | 8948 / 20020058 | 13 | 1083 | 0 failed |
| NativeAPI_Phone | 3409 / 20020074 | 13 | 1044 | 0 failed |

Assertion totals include per-event identity/thread/coordinate checks and vary
with the number of native move callbacks; the scenario count is the stable
coverage measure. The SDK source reports success for the six accepted payload
scenes and completion/refusal for the negative scenes.

Native-view regression: computer PID 11742 passed 32 native assertions plus
three guarded UI scenes; phone PID 5893 passed 30 native assertions plus three
UI scenes. Both retain normal focus/click, native control replacement and tree
cleanup after receiver isolation. Final desktop normal-suite regression (PID 3433) passed 269 checks: 201 native
integration, 52 native picker, five dialog UI, eight picker UI and three real
window drag/maximize/restore assertions. NativeAPI_Desktop remains running; the
phone is stopped with its data preserved.

## Build and downstream checks

No public signature changed; six-platform definitions remain present. Header
availability notes and the generated website reference describe the OHOS subset.
Full codegen regenerated 75 C ABI and 339 binding/reference files; the same eight
baseline parser skips remain. Codegen check, Rust bindgen and Dart ffigen passed;
raw binding outputs are unchanged.

On the local macOS host: core CTest 33/33; Rust workspace check; Dart analyze;
C# solution build (0 warnings/errors); JS native build, TypeScript check and tests
(4 pass, 1 existing skip); Python native build/lint and tests (26 pass, 1 skip);
Go shared-library build and tests passed. These are macOS binding checks, not
separate HarmonyOS runtime acceptance of each language. Windows/Linux/Android/iOS
native builds were not run for this checkpoint.

## Reproduce and evidence

With the phone or computer guest running on macbook-pro:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  NATIVEAPI_TEST_SUITE=drop-target \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

The Windows runner accepts the same `NATIVEAPI_TEST_SUITE=drop-target` selector;
it was updated but not exercised in this checkpoint. The HAP validator checks
the device-specific lifecycle before installation.

Remote evidence is preserved under
`~/Library/Huawei/Logs/drop-validation-20261016/`, including native logs, SDK
source outcomes, UI ownership trees, window dumps, screenshots, exact source and
the installed unsigned test HAP with SHA-256. Local ignored copies live under
`core/build-ohos/verification/drop-validation-20261016/`.

Authoritative references: installed API 26 `arkui/drag_and_drop.h`,
`database/udmf/{udmf,uds,utd}.h`, the
[public native implementation](https://github.com/openharmony/arkui_ace_engine/blob/master/interfaces/native/event/drag_and_drop_impl.cpp),
and the official
[uniform data structures](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkdata/js-apis-data-uniformDataStruct.md).

Cross-application file grants/revocation, folders, external file-manager input,
source-side DragSource, WindowDragSession, arbitrary non-native content, child
precedence, unsupported/malformed payloads, multi-window and worker/environment
teardown remain open. The cleanup hook is implemented; environment destruction
has not received a separate runtime acceptance assertion. Pending keyboard-input
permission remains separate; this checkpoint sends no keys.
