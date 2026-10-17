# HarmonyOS foundation backend validation — 2026-10-14

## Environment

- Remote host: `windows-laptop`, Windows 11 x86_64, Intel i7-11370H, 16GB RAM.
- Existing DevEco Studio 26.0.0.821 and its API 26 SDK were reused.
- Emulator: HarmonyOS 7.0.0.106, x86_64, `NativeAPI_Phone`, 2GB RAM.
- Images: `E:\Huawei\Sdk`; instance data: `E:\Huawei\Emulators\NativeAPI_Phone`.
- The emulator is also listed in the existing DevEco deployment directory through
  a junction to E:, retaining the previously configured emulators.
- Desktop launch shortcut: `NativeAPI HarmonyOS`.
- User tool paths and SDK/Java/Hvigor environment variables are configured.

## Result

The final debug HAP reported **46 passing checks and zero failures**, including a
second process run confirming persisted preferences. The HAP remains installed as
`dev.nativeapi.smoke`; its runner is `core/tests/ohos_smoke/run.ps1`.

The test compared nativeapi device/display values with the actual SDK, called the
generated C ABI, checked preference scope isolation and persistence, and exercised
clipboard Unicode, text+HTML, deferred reads, worker-thread writes, change events,
pre-initialization failure and listener cleanup. READ_PASTEBOARD was granted only to
the debug HAP inside the disposable emulator; real applications must obtain user
permission normally.

ARM64/API 23: the core and native integration module compiled and linked successfully
on windows-laptop. This was a cross-build, **not an ARM64 device runtime test**.

## Apple Silicon emulator — macbook-pro

- Host: `macbook-pro`, macOS 27.0.1, ARM64, 16GB RAM; hardware virtualization enabled.
- DevEco Studio 26.0.0.851 was downloaded on the local Mac, transferred over SSH,
  and installed at `/Applications/DevEco-Studio.app`. Both archive copies matched
  SHA-256 `86289edeb2e8e30eb1adf80cba0fc55f0e57cdd4f6dc4ec9c59985a0bc4b8524`.
  The disk image checksum and the application's Huawei Developer ID signature
  (`TZEA3TN37Q`) passed verification.
- Emulator 26.0.0.402; HarmonyOS 7.0.0.107 / API 26 phone image, ARM64.
- Instance: `NativeAPI_Phone`, 4 CPU cores, 4GB RAM, 6GB storage.
- Images: `~/Library/Huawei/Sdk`; instance: `~/.Huawei/Emulator/deployed/NativeAPI_Phone`.
- Desktop launcher: `NativeAPI HarmonyOS.app`, successfully used to start the emulator.
- Tools are configured in `~/Library/Huawei/ohos-env.sh`, sourced by both `.zprofile`
  and `.zshrc`; fastforge detects the `ohos` platform, Hvigor and the bundled Java.
- The HAP was built on macbook-pro with ARM64 and x86_64 native modules. The Unix
  runner (`core/tests/ohos_smoke/run.sh`) completed twice in separate app processes:
  **46 passing checks, zero failures each time**, including preference persistence.
- The test result remains visible in the running emulator. Host logs are under
  `~/Library/Huawei/Logs`; fixture logs are under `tests/ohos_smoke/test-results/`
  in the fastforge core mirror.

The first launch attempt occurred during the emulator's initial locked-screen
startup and failed explicitly. After the launcher home screen was ready, both runs
passed. No host mouse or keyboard input was injected for these emulator tests.

The test runners now store results outside Hvigor's build directory because a fresh
build can remove that directory. The updated Windows runner was also rechecked:
**46 passing checks, zero failures**.

## Workspace checks on macOS

| Check | Result |
| --- | --- |
| Native C++ library build | Passed |
| `./codegen check` | Passed; no new skipped symbols |
| Rust raw FFI + Dart raw FFI regeneration | Passed; no signature changes |
| Rust workspace check | Passed |
| Dart analyze | Passed |
| C# solution build | Passed, zero warnings/errors |
| JS type check + tests | Passed; 4 tests passed, 1 desktop clipboard case skipped |
| Python native build + tests | Passed; 26 tests passed, 1 desktop clipboard case skipped |
| Go native build + tests | Passed |
| Python static checks | Four pre-existing I001 import-order findings in untouched test files; no fixes made |

## Scope

Implemented: DeviceInfo, Display/DisplayManager queries, Preferences, text/HTML
Clipboard with local monitoring, and N-API UI-thread dispatch. No public signatures
changed; existing generated language APIs call the updated core backend.

Not covered by this phase: Window/Ability lifecycle and creation, menus/tray, dialogs,
native views, secure storage, accessibility, global shortcuts, clipboard image/file
conversion and display hotplug events. SDK requirements,
initialization and supported formats are documented in `core/src/platform/ohos/README.md`.

Window management was implemented afterward and is covered by
[the window validation report](ohos_window_validation.md).
