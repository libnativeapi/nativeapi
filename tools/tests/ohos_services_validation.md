# HarmonyOS service implementation checkpoint — 2026-10-15

This is a checkpoint toward implementing every feasible HarmonyOS API. It does
not close the full API goal; the [coverage ledger](ohos_api_coverage.md) records the
remaining modules, semantic checks and interoperability work.

## Implemented changes

- AppInfo reads current bundle metadata through public bundleManager APIs. The
  localized application name resolves the attached UIAbilityContext label resource.
  Calls from workers marshal to the owning ArkTS UI environment. The host attaches
  its context explicitly in `onCreate`; no platform types enter public headers.
- Image uses the public Image NDK: source format and native pixels, first-frame
  decode, PNG data-URI encoding, file encoding, retained copy ownership and safe
  moved-from access. PNG/JPEG/WebP codecs were exercised. An internal PixelMap
  encoder lets clipboard reads materialize independent images without exposing a
  new public factory or retaining buffers owned by the native clipboard.
- SecureStorage uses the system ASSET encrypted store, with namespace-scoped
  enumeration/deletion and no plaintext fallback. Empty values have a format byte;
  values are limited to 1023 bytes, namespaced aliases to 256 bytes. Metadata is
  unencrypted; values are encrypted. First unlock is required; app uninstall removes
  assets. Missing removals return false, and invalid inputs preserve prior values.
- Clipboard supports native text/HTML, PixelMaps and readable sandbox file/directory
  URI records in the same data package. Path conversion and validation happen before
  publication, and a foreign private sandbox URI is never mistaken for the same
  path inside this app. Unmappable/non-local file URIs are ignored on reads.
- Checked internal N-API/environment/context helpers support further SDK-backed
  modules. Context references are released during environment cleanup.

## Runtime evidence

Both guests run on `macbook-pro`, DevEco Studio 26.0.0.851 / SDK API 26, ARM64:

| Guest | Image | Final integration results |
| --- | --- | --- |
| NativeAPI_Phone | HarmonyOS 7.0.0.107 | 143 passed / 0 failed twice, final PID 17999 at 23:41:26 on October 14 |
| NativeAPI_Desktop (MateBook Pro) | HarmonyOS 7.0.0.107 | 156 passed / 0 failed, then 155 passed / 0 failed, final PID 7807 at 23:53:10 on October 14 |

The total includes assertions in native clipboard callbacks, whose notifications
can coalesce; each final run had zero failures. A missing PASS marker is failure.
The computer guest additionally passed the existing three guarded guest UI checks
(actual title-bar drag, maximize and restoration of native geometry).

New assertions cover:

- Installed name, bundle identifier, version/build strings, worker access and the
  generated C ABI.
- Actual 3×2 RGBA bytes, invalid base64/padding/data, PNG/base64/file pixel round trips,
  format detection despite a renamed file, JPEG/WebP codecs, copy/move ownership,
  invalid paths/codecs and a generated image handle.
- Real ASSET availability, Unicode/empty/embedded-NUL values, overwrite, scope
  isolation, all storage collection operations including more than 40 entries,
  1023-byte boundary, invalid aliases, instance reopen, process restart persistence,
  worker reads, deletion semantics and generated C ABI calls.
- Native clipboard image pixel equality and independent ownership after Clear;
  ordered file paths containing Unicode, whitespace, `+`, `#` and `%`; all four
  formats returned together; failed file-list conversion retaining all prior formats.
- All prior foundation/window cases are retained.

The tests use the normal app sandbox and the isolated emulator's native services.
The runner grants READ_PASTEBOARD only to its disposable debug HAP. No host desktop
mouse/keyboard events are injected. The app and computer emulator remain visible.

## Important limits / work still open

Cross-app clipboard file reception/authorization has not been exercised with a
separate receiving application. Remote clipboard change signals are not subscribed.
Locked-state ASSET failures and actual environment teardown are not separate runtime
assertions. Broader codec/large-image cases also remain part of the open audit.

Application lifecycle, URL dispatch, notifications, dialogs, menus, views, input,
drag/drop, tray, launch-at-login and remaining window/display capabilities are not
made complete by this checkpoint. Existing stubs are not treated as proof of an
unavailable public SDK feature. Other emulator architectures were compiled into
the HAP but not run for this checkpoint.

## Generation and host checks

Public signatures are unchanged. Public availability documentation and all six
binding references were regenerated. No new skipped symbols. Rust bindgen and Dart
ffigen ran; their raw ABI outputs stayed unchanged. `./codegen check` confirmed all
339 files current.

- Rust workspace check: passed.
- Dart analyze: passed.
- C# solution build: zero errors/warnings.
- JS typecheck/tests: 4 passed, 1 desktop clipboard test skipped.
- Python lint/native build/tests: passed; 26 tests passed, 1 desktop clipboard test skipped.
- Go native build/tests: passed.

These binding checks ran on the local macOS host; they do not claim language runtime
execution inside HarmonyOS. HAP tests exercise C++ and the generated C ABI there.

Reproduce on an unlocked local guest with `core/tests/ohos_smoke/run.sh`; it chooses
the computer-only manifest model for a 2in1 guest. Use `--skip-build` for the second
process-persistence run. See the fixture README for the exact host setup.

Remote evidence: `~/Library/Huawei/Logs/services-validation/{phone,desktop}/`, plus
`nativeapi-services-*.log` in the same Logs root. Local copies live under ignored
`core/build-ohos/verification/services/`. Reusable assertions are in
`core/tests/ohos_smoke/entry/src/main/cpp/service_tests.h` and `smoke.cpp`.

Validated core implementation: `0bd817b`.
