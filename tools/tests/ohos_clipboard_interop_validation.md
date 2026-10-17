# HarmonyOS clipboard interoperability — 2026-10-16

Clipboard URI reads now verify actual file type and app readability before
returning a local path. Deleted files and genuine app-UID read denial no longer
produce misleading usable paths. For foreign authorities, lexical normalization
and realpath checks prevent dot segments or symlinks from bypassing the private
sandbox-root check. Valid SDK mappings to readable authorized foreign paths
remain supported. Public API and generated ABI signatures are unchanged.

## Independent application evidence

Host: macbook-pro ARM64, DevEco Studio 26.0.0.851. Both named disposable guests
run HarmonyOS 7.0.0.107/API 26. API 24 is not part of this checkpoint.

| Guest | Native process / peer | Application UIDs | Result |
| --- | --- | --- | --- |
| NativeAPI_Desktop | 19276 / 19488 | 20020058 / 20020061 | 34 native assertions + 17 validated commands, zero failures |
| NativeAPI_Phone | 3936 / 4064 | 20020074 / 20020075 | 34 native assertions + 17 validated commands, zero failures |

Each final state records three C++ and three C change notifications and inactive
monitoring. A further external publication after the final unsubscribe leaves
both event counts unchanged. Cleanup is an additional idempotent command.

The new `core/tests/ohos_clipboard_peer` is a separate application named
`dev.nativeapi.clipboard.peer`. It imports only public ArkTS Pasteboard, File and
Image SDKs; its packed HAP is checked to contain no native shared libraries.
The driver verifies different application UIDs, exact process command lines and
fresh request IDs. Each read follows an actual foreground handoff. Debug
READ_PASTEBOARD grants apply only to these two isolated emulator fixtures.

Each application creates files at the same private logical paths, including
Unicode, spaces, plus/hash/percent characters, but with different UTF-8 contents.
The test compares the original peer URI paths against the consumer's actual
UIAbilityContext.filesDir paths, then requires two real readable authorized
paths and exact producer bytes in order. Returning an empty file list cannot
pass. On the tested guests, the SDK maps external file URIs under
`/storage/Users/currentUser/appdata/el2/base/dev.nativeapi.clipboard.peer/…`.
The consumer must never confuse them with its own `/data/storage/…` files.

The scenario covers independent SDK publication into C++/C/worker reads, file-only
reads, C++ and generated C publication back into the independent SDK, external
change events, final observer removal and foreign clear. Text and HTML match
exactly; all 24 bytes of a 3x2 RGBA image match in both directions. SDK pixel
format is inspected and normalized explicitly; the producer declares RGBA input
rather than relying on the SDK's BGRA source-buffer default. C read delivery
leases must be active and completed once, with a later state snapshot checking
all native assertions after completion. Source fixtures are different processes
and UIDs throughout the transfer.

Eight direct FileUri assertions check an existing own readable file, foreign
underlying-file separation, dot-segment separation, a real URI before unlink,
rejection after unlink, a real URI before chmod, actual app-UID read denial and
remote-URL rejection. A foreign URI may correctly resolve to an authorized
external file, so the separation assertion compares device/inode identity;
it does not incorrectly require every external URI to be rejected.

No host pointer or keyboard input is used. The original independent producer
and receiver remain installed with their data preserved. Clipboard content is
cleared by the independent peer before completion. The final cleanup removes
native observers even when a stronger payload assertion fails.

## Reproduction and evidence

With one unlocked NativeAPI_Phone or NativeAPI_Desktop running:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=clipboard \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

The normal runner builds and installs the main fixture and the separate SDK peer.
`--skip-build` requires both HAPs already built. Process identity, command JSON,
real file bytes, native/peer logs, packed-bundle metadata and screenshots are kept
under `test-results/clipboard-suite/`. PowerShell has a matching branch, but its
runtime is untested in this macOS-host checkpoint.

Read-only codegen/check passes 75 C ABI and 339 binding/reference files with the
eight baseline skipped symbols. Python drivers pass compilation and Ruff E9/F;
Bash syntax passes. Production changes are OHOS-only and are compiled and run
inside both real emulator environments; this checkpoint does not claim new
Windows/Linux/Android/iOS native runtime coverage.

Remote evidence: `~/Library/Huawei/Logs/clipboard-interop-20261016/`.
Local copy: ignored `core/build-ohos/verification/clipboard-interop-20261016/`.

Final normal computer regression, PID **3446**, passes **201 integration + 52
native picker + 5 dialog UI + 8 picker UI + 3 desktop-window UI = 269 checks**,
zero failures. Real title-bar movement, maximize and restore pass. The computer
guest remains running on its passing fixture; the phone is stopped and both
guests preserve their data. `desktop-final/` contains current-run logs, fresh
UI ownership/geometry records and the final native/peer HAP hashes.

## Remaining coverage

Follow-up: [clipboard boundary acceptance](ohos_clipboard_boundaries_validation.md)
now covers directories, permission refusal/restoration, concurrent changes and
real environment teardown on both guests. The paragraph below describes the
original checkpoint, before that follow-up.

File-manager/directory interop, URI grant revocation and producer restart/uninstall,
permission refusal for clipboard content itself, concurrent change during reads,
queued read/environment teardown and broader codec boundaries remain open.
This checkpoint proves the listed positive flows and file-access rejection;
it does not complete the whole Clipboard module or HarmonyOS API goal.

Core implementation and fixture commit: `4b8b8ab`. Changes are local; nothing
has been pushed. The full HarmonyOS API goal remains active.
