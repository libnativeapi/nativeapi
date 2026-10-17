# HarmonyOS document picker implementation and open validation — 2026-10-15

This checkpoint remains incomplete on the phone. It is part of the full HarmonyOS
API goal, not a replacement for that goal. A public SDK control returning one file
does not establish that multi-file selection is impossible on the device.

## Implementation and computer evidence

Core revision `e07fe6c` uses the public `@ohos.file.picker.DocumentViewPicker`
with the attached UIAbilityContext and visible nativeapi parent window. `Open()`
waits on a worker; the UI thread continues pumping the actual picker. Configuration
is immutable and results remain absent while pending. Original authorized URIs are
returned without stripping schemes, decoding them into POSIX paths, or copying
files into the application's sandbox.

On macbook-pro's ARM64 computer guest, process **3410** passed **52 native checks**
and **8 guarded picker UI checks**, zero failures, at 02:21. These cover saving two
different file types with Unicode/space/#/% names, writing and reading actual bytes,
opening the same file again, selecting both original files together with their
contents intact, selecting the containing folder, user cancellation and generated
C ABI result/ownership paths. See [computer acceptance](ohos_desktop_window_validation.md).

After the final core commit, computer process **3053** repeated the complete run
at 02:51: **201 integration + 52 picker + 5 dialog UI + 8 picker UI + 3 desktop UI
checks**, zero failures. This verifies revision `e07fe6c`, including the test
host's separate reference/cancellation paths leaving its default full suite intact.
The passing computer application remains visible.

The SDK has no public programmatic close operation; `Close()` returns false while
the real native picker remains pending. Native user cancellation is handled
separately. A literal comma within one extension cannot be expressed in the SDK's
comma-separated filter syntax and fails explicitly before showing the UI.

## Phone investigation

The same ARM64 phone guest successfully saved `.txt` and `.md` documents, obtained
their exact authorized URIs, wrote/read their actual contents using the public
file API, and opened the original `.txt` document again. Process **3377** recorded
these positive native and C ABI checks; its full suite subsequently failed on
multi-selection. It must not be reported as a passing full phone suite.

The phone save sheet retains its last folder and exposes folder rows as either
`Row symbol` or `Row image`. The UI driver now handles the actual saved-folder
heading and waits for the current browser navigation state, rather than assuming
that every invocation starts at the storage root.

Selecting file A from the phone's Browse > My phone > Download page immediately
returns one URI. The expected two-file assertion remains a failure. Experiments
with explicit selection counts (500, 10000 and INT32_MAX), SDK option constructors
and integer creation did not resolve that behavior; those speculative production
changes and diagnostic logging were removed. The implementation retains the
system's default multi-selection limit. Long-pressing the file label/icon exposed
a provider subwindow; the owner guard stopped subsequent input instead of treating
that window as the application's modal extension.

An independent ArkTS reference path now calls the public SDK directly, bypassing
nativeapi. Process **21895**, using `new DocumentViewPicker(context)` and
`maxSelectNumber = 10`, also returned **one URI** from that Browse interaction.
Its rendered `SDK_REFERENCE_COUNT 1`, exact URI, JS log and ownership snapshots
are retained. This narrows the investigation to the SDK/provider interaction and
remaining UI paths; it does not classify the public multi-file API as unsupported.

Process **24297** additionally reported
`canIUse('SystemCapability.FileManagement.UserFileService.FolderSelection') == true`.
Direct SDK folder mode hid ordinary files inside Download, but the explored page
did not expose a folder-confirmation control. Folder acceptance remains open.

A separate cancellation scenario subsequently passed on phone process **30130**
at 02:46:28: **10 native checks and 3 guarded UI checks**, zero failures. It verifies
the real C++ and generated C ABI user-cancellation results, absent paths/errors,
pending-state behavior and released-handle rejection. It emits
`NATIVEAPI_PICKER_CANCEL_PASS` / `NATIVEAPI_PICKER_CANCEL_UI_PASS`; these distinct
markers cannot satisfy the normal full-suite PASS requirement. The UI driver now
waits for the native phase transition after one close press, preventing a second
press from reaching the next test's newly opened picker.

## Reproduction and evidence

The normal runner still requires the complete native and guarded UI suites to pass:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=picker ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME"
```

For the independent SDK comparison, install the same test HAP, then start its own
Ability with `--ps nativeapiSuite reference` (multi-file reference) or
`--ps nativeapiSuite reference-folder`. These diagnostic paths do not call
nativeapi's picker implementation and are not substitutes for its acceptance suite.

The isolated native cancellation scenario uses `nativeapiSuite=picker-cancel` and
`verify_picker_ui.py ... --pid <fresh-pid> --cancel-only`. It runs the normal native
and C ABI cancellation stages with a separate summary, leaving the complete suite's
multi-file and folder requirements intact.

Computer evidence: `~/Library/Huawei/Logs/desktop-acceptance-20261015-0220/`.
Final committed-source computer evidence:
`~/Library/Huawei/Logs/picker-validation/desktop-e07fe6c/`.
Phone evidence: `~/Library/Huawei/Logs/picker-validation/phone-investigation-20261015/`.
Local copies are under ignored `core/build-ohos/verification/`.

Remaining work includes the phone interactions above, provider count limits,
filter/name boundaries, environment teardown and permission-grant lifetimes,
and interoperability of selected URIs with Image/Clipboard. Shared-code regression
evidence already includes **28 Linux file-dialog checks** in a private Xvfb server,
zero failures. Code generation and binding checks do not prove these runtime cases.

Binding verification for this checkpoint: Rust workspace check, Dart analysis,
C# build, JS type/native tests (4 passed, 1 skipped), Python native build/lint/tests
(26 passed, 1 skipped), and Go native build/tests passed on the local Mac. Raw Rust
and Dart FFI were regenerated without changes. HarmonyOS builds compile both ARM64
and x86_64 libraries; runtime evidence here is macbook-pro ARM64 only. Windows
PowerShell runner parsing passed; Windows runtime was not rerun for this checkpoint.

Public source references: [picker API and options](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-core-file-kit/js-apis-file-picker.md),
[SDK picker implementation](https://github.com/openharmony/filemanagement_user_file_service/blob/master/interfaces/kits/picker/picker.js),
[document URI access](https://github.com/openharmony/docs/blob/master/en/application-dev/file-management/user-file-uri-intro.md).

## 2026-10-16: API 26 reinvestigation and release API 24 comparison

The new phone process **3560** successfully saved both current-run nonce filenames,
verified their real bytes and C ABI URIs, and reopened the original document. In
Browse > My phone > Download, one guarded file press immediately completed the
multi-file request with `count=1`: **33 passed, 1 failed**. No early confirmation
press was sent. The exact-two-original-files requirement remains intact.

The direct SDK probes recorded actual serialized options and did not call nativeapi:
**9180** used maxSelectNumber=10 and a Download start URI; **18382** omitted file
filters; **19941** used DOCUMENT aggregation with no filter. The tested Recent
pages remained empty. Folder probes **9423** and **20103** omitted file filters;
root/Download navigation exposed no acceptance control. Long presses opened
provider subwindows whose parent IDs were verified in WMS (33 → app window 32,
36 → app window 35); they exposed no actionable children in the captured trees.
The provider logged Recent/database/resource errors, including an undefined
`searchMemoryId`. These logs are observations, not a proven root cause or a public
SDK unsupported classification. A separate system FileManager preview visibly
rendered the test document's original `NativeAPI A 中文🙂` bytes; this did not
establish successful picker multi-selection or folder acceptance.

An additional official release phone image was installed and created as
**NativeAPI_Phone24** on macbook-pro: HarmonyOS 6.1.1/API 24, software 6.1.0.126,
ARM64, Pura X View, 4 GB. It preserves the original API 26 phone instance and data.
The explicit `release24` fixture product keeps target API 26 and declares minimum
6.1.1(24); its packed manifest was verified (`minAPIVersion=60101024`). Installation,
native-library loading and the actual test page succeeded. A static header scan
matched all 144 referenced OH_/HMS_ imports; it is not proof that every library API
works on API 24. The comparison used guest-only input guarded by fresh native
modal/window/process ownership; no host mouse or keyboard input was sent.

On that release image, process **6055** again passed save A/B, exact URI/C ABI and
actual byte checks, and original-file reopening. One file press in multi-file
Browse returned `count=1`, producing **33 passed, 1 failed**. Independent SDK
folder process **7598** entered the storage root and Download with FOLDER mode,
maxSelectNumber=1 and no file filter; no folder-confirm control or accepted URI
was observed. These are additional reproductions, not a passing full phone suite
or a claim of complete API 24 compatibility.

Test-only changes expose start-URI/filter/aggregation options and structured SDK
errors, retain genuine reference pending/result markers in ownership guards, and
add the explicit release24 build/instance variant. Production picker behavior and
all original full-suite assertions remain unchanged.

Evidence: `~/Library/Huawei/Logs/picker-reinvestigation-20261016/` on macbook-pro,
including native/SDK logs, serialized options, original UI/WMS captures, packed
bundle metadata, build logs and the exact guest-only comparison scripts. Local
copy: ignored `core/build-ohos/verification/picker-reinvestigation-20261016/`.
Phone multi-selection, folder acceptance and the wider picker audit remain open.

After these test-only changes, the normal API 26 computer product was rebuilt and
installed afresh. Process **3144** passed **201 integration + 52 native picker +
5 dialog UI + 8 picker UI + 3 desktop-window UI checks = 269**, zero failures,
at 03:14 on 2026-10-16. Real multi-file/folder selection and native title-bar
movement/maximize/restore passed again. The computer emulator is left running on
its passing fixture. The release comparison instance is stopped; its data is kept.
Final evidence is under the archive's `desktop-final/` directory. Python syntax,
Ruff E9/F and whitespace checks also pass. No public headers, production APIs or
generated bindings changed in this diagnostic checkpoint.

The diagnostic fixture is committed as core `8fbede9`; production implementation
remains the preceding `5bf4fed` snapshot. No changes were pushed.
