# HarmonyOS programmatic drag sources — 2026-10-16

This checkpoint replaces the OHOS DragSource stub with public native ArkUI drag
actions on macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26
ARM64. The broader OHOS API goal remains active.

Implementation commit: core `718c92b`.

The implementation creates an ephemeral native node inside the source window's
public UIContext scope, obtains its drag action and disposes the node. It does
not replace ArkTS content or require a mounted NativeAPI content root. The SDK
receives immutable UDMF text/file records and a retained PixelMap preview. A
compact default tile supplies the mandatory preview when no image was requested.
File paths use the existing validated public URI conversion helper.

SDK status notifications can arrive on a worker. Borrowed notification objects
are consumed there; guarded completion is posted to the initialized UI loop.
Each request has an independent token and owned resources. The public source may
be released on UI or a worker while the OS finishes its drag; its SDK payload and
preview remain alive, and dead sources receive no completion. The registration
is retired before user completion, so listeners can destroy their source safely.
Environment cancellation also uses the emitter's lifetime guard; complete real
environment teardown and cross-environment reuse are not separately exercised.

A second source cannot disturb an ongoing NativeAPI SDK session. The NDK
StartDrag wrapper can return zero after the engine refuses a request, so an
immediate ENDED without STARTED is rejected rather than leaving IsDragging true.
No-pointer, no-data, None/Move/Link preferences, embedded-NUL text and missing-file
requests are checked independently of successful data delivery.

## Public capability queries and SDK limits

Two additive static methods exist on all six native backends and all bindings:

- `DragSource::IsPositionSupported()`: true on macOS/Windows/Linux; false on
  Android/iOS/OHOS. A direct independent SDK action confirms that OHOS source
  completion reports zero coordinates even for a non-origin destination. Its
  end positions are an explicitly unavailable origin fallback, not measured
  coordinates. Ordinary window-local touch monitoring also did not receive the
  release consumed by the system drag; it is not used to invent an endpoint.
- `DragSource::IsOperationSelectionSupported()`: true on macOS/Windows/Linux;
  false on Android/iOS/OHOS. The public NDK offers no source operation mask.
  Only the default Copy preference may initiate this backend; explicit Move/Link
  preferences are refused. The SDK receiver can nevertheless choose Move, and
  ended events preserve its actual Copy/Move/None result. The backend never
  deletes the original files. Clients needing a fixed source offer must check
  the capability before relying on SetDragOperation.

The existing event payload layout is unchanged. New C symbols and language
methods are additive. Source properties retain their requested values, including
preferences that cannot initiate an OHOS source. Header availability notes and
the generated API reference describe the partial capabilities.

Authoritative references: installed API 26 `arkui/drag_and_drop.h` and the public
[NDK adapter](https://github.com/openharmony/arkui_ace_engine/blob/master/interfaces/native/event/drag_and_drop_impl.cpp),
[engine drag adapter](https://github.com/openharmony/arkui_ace_engine/blob/master/frameworks/core/interfaces/native/node/drag_adapter_impl.cpp),
and [pointer validation](https://github.com/openharmony/arkui_ace_engine/blob/master/adapter/ohos/entrance/ace_container.cpp).
The adapter copies result/behavior into source notifications, omits coordinates,
and validates the current pressed pointer before asking the system to drag.

## Real guest acceptance

The destination is an independent ArkTS SDK component. It reads the actual UDMF
records and both files through the SDK file APIs, checking exact UTF-8 bytes.
No NativeAPI destination or fabricated source event supplies the expected data.

The suites cover C++ and C text/mixed/file-only payloads; actual Copy/Move/refusal
feedback; leave/cancel; source and concurrent-instance conflicts; default/custom
previews; property/image-handle changes after submission; idle/invalid requests;
destruction from completion and while dragging on both UI and worker threads.
Two preview cases compare real guest pixels before and during the atomic gesture,
proving the original magenta preview remains visible after clearing the property.
Each gesture revalidates the named guest, bundle PID/UID and fresh native window
ownership over its path, then always completes its touch/mouse release.

| Guest | PID / UID | Real gestures | Native assertions | Result |
| --- | --- | --- | --- | --- |
| NativeAPI_Desktop / 2in1 | 9589 / 20020058 | 27 | 211 | 0 failed |
| NativeAPI_Phone | 4172 / 20020074 | 25 | 191 | 0 failed |

Each suite includes an independent direct-SDK reference drag proving the missing
end coordinates. The desktop adds C++/C real mouse drags to the phone's touch
scenes. The worker cases retain data delivery while suppressing completion to
already-released public sources. Thread and state checks use actual callbacks.

## Verification and reproduction

On macbook-pro with exactly one named phone/computer guest running:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  NATIVEAPI_TEST_SUITE=drag-source \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

The PowerShell runner accepts the same selector but was not run on Windows.
Only disposable loopback guests are accepted; no host input or keyboard input is
sent. Full codegen writes 75 C ABI and 339 binding/reference files with the same
eight baseline skips. Rust bindgen and Dart ffigen expose both new symbols.

Local macOS verification: core CTest 33/33; Rust workspace check; Dart analyze;
C# solution build (0 warnings/errors); JS native/type/test checks (4 pass, 1
existing skip); Python native/lint/tests (26 pass, 1 skip); Go native build/tests. These compile/check the macOS binding backends; HarmonyOS runtime
acceptance above exercises C++ and C ABI, not six separate language runtimes.
Final desktop normal-suite regression (PID 3276) passes 269 checks: 201 native,
52 picker, 5 dialog UI, 8 picker UI and 3 actual window drag/maximize/restore
checks, with zero failures. Its archive contains 86 fresh evidence files and the
installed HAP with SHA-256.

Remote evidence: `~/Library/Huawei/Logs/source-validation-20261016/`, with native
logs, ownership trees/window dumps, before/held/after screenshots, exact source,
and installed unsigned HAP plus SHA-256. Final runs are selected by the PIDs in
the table; earlier investigative runs are retained separately. Local ignored
copies live at `core/build-ohos/verification/source-validation-20261016/`.

Other-app grants/revocation and file-manager input, folders, multiple pointer IDs,
other device/SDK versions, secondary windows, unacknowledged SDK failures, and
complete worker/environment/process teardown remain open. The capability flags
record the verified SDK restrictions; they do not close those remaining audits.
