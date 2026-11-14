# Protocol handler migration verification (#27)

The [migration guide](https://github.com/leanflutter/protocol_handler/blob/main/MIGRATION.md)
maps every 0.2.x public capability to the NativeAPI-backed 0.3.x facade and the
direct NativeAPI API. The facade remains necessary for Flutter host integration.

## Scope

`verify_protocol_handler.py` builds the real plugin example with a debug-only
[test entry point](fixtures/protocol_handler_flutter.dart). The entry point reads
the public Dart API and exposes its observations on loopback port 49227. The runner
sends URLs through each OS; it never calls the native delivery bridge to manufacture
an incoming event, and sends no desktop mouse or keyboard input. iOS 27.2 displays
a system confirmation before opening the custom scheme; the optional XCTest helper
accepts only the alert naming `Protocol Handler Example` in the dedicated simulator.

Each platform runs these 14 checks:

1. Ordinary launch has no initial URL.
2. Registration of the declared `myprotocol` scheme succeeds.
3. Invalid scheme registration returns false.
4. A real URL cold-launches the process and retains its entire encoded query.
5. An early URL is present on the first read, or a late URL reaches both consumers.
6. The launch URL is not duplicated in the listener or stream.
7. Mutating the `listeners` snapshot does not detach the actual listener.
8. A URL opened while running arrives once in both the listener and stream.
9. Warm delivery keeps the same process and original initial URL.
10. Opening the same URL again produces a second activation.
11. Removing the object listener stops its callbacks while the stream still works;
    `hasListeners` is false and the listener count is zero.
12. Cancelling the stream as well stops all Dart deliveries.
13. Two URLs opened without subscribers are replayed once, in order, on resubscription.
14. The initial URL remains unchanged after resubscription.

The cold URL contains `%20`, a percent-encoded emoji and two query parameters;
the warm URL contains percent-encoded Chinese text. Counts and exact strings,
rather than screenshots, are asserted.

## Results

Verified on 2026-11-14 with Flutter 3.50.0 / Dart 3.14.0, the local `nativeapi` and
`cnativeapi` 0.6.0 packages, and the NativeAPI-backed `protocol_handler` 0.3.0
plus the iOS delegate-lifetime fix and documentation/example changes accompanying
this report. The core was `f77602ae`; its protocol implementation is unchanged
from the original `024ce5df` implementation. These checks cover source on `main`,
not a new pub.dev release.

| Platform | Environment and delivery route | Result |
| --- | --- | --- |
| macOS | Local arm64 Mac, macOS 27.2 (26B123), sandboxed debug app; `open -g -a <bundle> <url>` sends the Apple event to the exact test bundle | 14/14 passed; launch URL arrived after the first null read and was also emitted once |
| Windows | `windows-laptop`, Windows 11 x64 build 26200.9974, interactive session 1; shell protocol activation launches the registered EXE and the second-instance helper forwards later URLs | 14/14 passed; launch URL available on first read; original HKCU scheme registration restored |
| iOS | Disposable iPhone 18 Pro simulator, iOS 27.2; `simctl openurl` through the scene-based Flutter runner, with the XCTest helper accepting the named test-app confirmation | 14/14 passed after the delegate-lifetime fix; launch URL available on first read |
| Android | Disposable arm64 Pixel 9 emulator, Android 16 / API 36; `am start` with ACTION_VIEW and BROWSABLE through the real activity/plugin | 14/14 passed; launch URL available on first read; later intents delivered to the same activity/process |

`flutter analyze` and all three package tests passed on macOS and Windows.
`core/tests/protocol_handler_test.cpp`, linked to the built macOS native library,
also passed: late launch delivery, bounded 32-item queue overflow and ordering,
listener removal/re-attachment and scheme validation. Unlike the OS-delivery
checks, this unit test intentionally uses the host bridge.

### Regression found and fixed

The first iOS run reproduced an actual failure: after confirming the system URL
prompt the process launched, but both `initialAtRead` and `initial` stayed null
and the listener/stream arrays remained empty. Flutter stores application and
scene lifecycle delegates weakly, and this plugin has no method channel retaining
its instance. `ProtocolHandlerPlugin.register` now calls
`registrar.publish(instance)` before registering its delegates, retaining it for
the engine lifetime ([plugin fix `73321dd`](https://github.com/leanflutter/protocol_handler/commit/73321dd)). Rebuilding and repeating the same OS-delivery test passed all
14 checks, including cold and subsequent links. The fix is under `Unreleased` in
the plugin changelog; 0.3.0 from pub.dev does not include it.

## Repeat the checks

Run from the workspace root. The build helper temporarily overrides `nativeapi`
and `cnativeapi` in **both** package and example, and copies the fixture to the
example's `lib/`. It restores/removes those files in `finally`; do not commit the
overrides. Build one local platform at a time. Port 49227 must be free.

```sh
python3 tools/gui/verify_protocol_handler.py macos --build

# Use dedicated, booted simulators/emulators; these commands install the example.
python3 tools/gui/verify_protocol_handler.py ios --build --device <simulator-uuid> \
  --confirm-ios-prompts
python3 tools/gui/verify_protocol_handler.py android --build --device <adb-serial>

fastforge --no-version-check host doctor windows-laptop
fastforge --no-version-check host exec windows-laptop -- \
  python tools/gui/verify_protocol_handler.py windows --build-only
fastforge --no-version-check host exec windows-laptop --no-sync -- \
  powershell -NoProfile -ExecutionPolicy Bypass \
  -File tools/gui/verify_protocol_handler_windows.ps1
```

Omit `--build` to reuse a built fixture. `--confirm-ios-prompts` requires XcodeGen;
it builds a small XCTest helper and accepts only the named test-app URL alert in
the specified simulator. It never moves the Mac's cursor or types keys. On runtimes
without this system prompt the flag is optional. iOS helper logs/results are kept
beside the runtime JSON. Do not count a successful `simctl openurl` exit code alone
as delivery: iOS can return zero while awaiting confirmation.

Windows runtime checks run through a
temporary scheduled task in the logged-on desktop session, not SSH session 0;
the task is removed afterwards. The runner backs up and restores the existing
`HKCU\Software\Classes\myprotocol` key. Runtime JSON evidence is written under
`tools/gui/output/protocol-handler-<platform>/results.json`; the Windows wrapper
also retains stdout, stderr and its desktop session/exit code in `output/`.

For the core queue test, compile `core/tests/protocol_handler_test.cpp` against
the built core (the CMake target is `protocol_handler_test`), then run it. Its
success marker is `protocol_handler_test: PASS`.

## Coverage boundaries

- Mobile runs use simulators/emulators, not physical iOS/Android devices.
- iOS exercises the scene-based Flutter runner; custom app delegates must preserve
  Flutter lifecycle forwarding and need their own integration checks.
- Windows exercises an unpackaged application. MSIX `uap:Protocol` configuration
  and the intentionally false runtime `register()` result are documented, but
  installing/signing and activating a packaged MSIX is not covered by this run.
- macOS selects the exact application with `open -a`; it verifies Apple-event
  delivery and declaration validation, not changing the user's default handler.
- Android process-death/Activity restoration, HTTPS App Links/Universal Links,
  Linux support and protocol unregistration are not claimed by these checks.
