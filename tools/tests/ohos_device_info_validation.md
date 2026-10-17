# HarmonyOS device information — 2026-10-16

All seven DeviceInfo getters now have ordinary-app C++/C UI and worker evidence
on both named API 26 guests. GetName reads the current user-assigned Settings
name when a host/context is attached, with the documented market-name fallback.
Missing OS distribution names remain empty; architecture describes the library's
compile target independently of the kernel machine. This checkpoint does not
complete the full HarmonyOS API objective.

## Implementation and SDK boundary

The public [Settings API](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-basic-services-kit/js-apis-settings.md)
provides general.DEVICE_NAME and getValueSync(context, key, default). The installed
SDK declares no permission for this read. The normal dev.nativeapi.smoke reader
declares no MANAGE_SETTINGS and is verified as a non-system application. Reads
marshal to the attached UI environment; pre-host/detached reads and SDK failure,
missing or empty names fall back to the public NDK market name.

The other fields use the public [deviceinfo NDK](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-basic-services-kit/capi-deviceinfo-h.md),
uname release and compile-target architecture. No new public signatures or C ABI
symbols were added. Header documentation records the actual six-platform behavior;
the five other platform implementations are unchanged. The iOS architecture note
was corrected against its existing compiler-target implementation.

Only the separate dev.nativeapi.deviceinfo.writer fixture writes the device name,
through public setValueSync. It declares only MANAGE_SETTINGS and uses the SDK
system_basic/hos_system_app debug profile for the specific emulator UDID. This
privilege belongs to the writer, not the normal reader or production library.
Profiles are short-lived, signed and verified using SDK public sample keys copied
to ignored output. No personal certificate or system permission database is used.

## Final real guest results

Host: macbook-pro, Apple Silicon; DevEco Studio 26.0.0.851, HarmonyOS image
7.0.0.107/API 26, ARM64. One cold-booted guest runs at a time.

| Guest | Reader PIDs / UID | Writer PIDs / UID | Native checks | Controlled faults | Worker samples |
| --- | --- | --- | ---: | ---: | ---: |
| NativeAPI_Desktop | 14914, 15081 / 20020058 | 14969, 15342 / 20020063 | 1,062 | 78 | 120 |
| NativeAPI_Phone | 3482, 3889 / 20020074 | 3697, 4086 / 20020077 | 1,062 | 78 | 120 |

All final cases have zero failures. Six phases each run 177 native assertions and
20 concurrent snapshots: initial read, saved original, first name, second name,
actual reader process restart while the second name remains set, and restoration
after an actual writer process restart. Controlled faults run once per reader
process (39 × 2); the native batch asserts that they passed. An additional finally
restoration/read batch is excluded from these totals.

Both guests start and finish with Settings name `emulator`. The two real test
names are `NativeAPI·设备甲🙂` and `NativeAPI·Device-B-é`; C++/C/worker values
match the independent SDK at every phase. Market name and model remain
`emulator`, manufacturer `HUAWEI`, distribution name `HarmonyOS`, distribution
version `7.0.0`, full OS name `OpenHarmony-7.0.0.105`, kernel release
`5.10.210`, and process architecture `aarch64`.

The writer flushes the saved original to its own SDK Preferences before mutation.
Its data survives application updates and process termination. The successful
restoration comes from a new writer PID, followed by another idempotent restore.
The helper refuses to overwrite an unrelated external device-name change. Every
writer reply is matched to its nonce and checked against the actual bundle,
process UID and installed permission/profile metadata. No mouse or keyboard
input is sent by this DeviceInfo workflow.

## What the assertions establish

- All seven UI getters agree between C++ and C. The first five match independent
  ArkTS SDK reads, kernel release matches guest uname, and architecture matches
  the actual loaded core library's ELF64 image (machine 183/AArch64), including
  process pointer width. The SDK is sampled before and after each batch.
- Pre-host C++/C reads use market name even when the current user name differs.
  A real owning-context detach also restores this fallback; all six other fields
  remain valid. Reattachment immediately restores the live user name.
- Four real C++ worker threads each repeat five C++/C snapshots while the main
  event loop remains available for dispatch. Completion returns on the UI thread.
- Mutating a caller-owned C string does not change the next allocation or C++
  value. Returned C strings are freed through the generated ABI helper.
- A separately namespaced target compiles the actual backend with private
  NDK/uname inputs. It covers null/empty/changed values, held string ownership,
  distribution-version fallback, missing distribution names, uname failure and
  a kernel machine deliberately different from the actual process ABI.
- The same isolated backend uses a private SDK-shaped module with the real
  N-API engine: user-name changes, empty results, thrown Settings calls, loader
  exceptions, wrong result types, undefined modules and missing methods all
  preserve the specified result/fallback without residual JavaScript exceptions.
  These substitutions never enter production or mutate the actual SDK module.

Shell access to another application's /proc/PID/exe is denied on these images.
The verifier therefore reads its own loaded native library's ELF identity via
the OS loader. This is process ABI evidence, not an inference from uname or the
device's supported ABI list.

## Build, compatibility and evidence

Local macOS CTest passes 33/33. Rust workspace checking, Dart analysis, C# build,
JS native/type/tests (four pass, one existing skip), Python native/lint/tests
(26 pass, one existing skip), and Go native/tests pass. C ABI and all six language
outputs were regenerated, including Rust bindgen and Dart ffigen; the generated
API reference documents the changes. Codegen check passes with the same eight
baseline unsupported-symbol skips. Python fixture syntax and selected
E/F/W/B/UP/I lint checks pass. The broader default Ruff rules report existing-style
issues in these scripts (dictionary style, executable shebangs and explicit
subprocess check), and are not claimed as passing.

The shared fixture signer preserves sign_keyboard_acl.py as its default keyboard
entry point. A real archived keyboard HAP was re-signed through that unchanged
interface and cryptographically verified successfully; no new keyboard input
test is claimed for this refactor. The keyboard runtime backend is unchanged.
Other operating-system runtimes and the six language runtimes inside OHOS were
not exercised by this checkpoint; the real guest evidence is C++/C plus ArkTS SDK.

Final archives are under `~/Library/Huawei/Logs/device-info-validation-20261016/`
on macbook-pro and copied under the same suffix in local
`core/build-ohos/verification/`. Final directories:

- `desktop-14914-15081`: suite roots `2in1-1792122447627415000` and
  `2in1-1792122452316498000`.
- `phone-3482-3889`: suite roots `phone-1792122565897908000` and
  `phone-1792122570656506000`.
- `host-checks`: local verification logs and signer compatibility result.

Archives contain per-phase SDK/native records, process metadata, writer journals,
raw logs, original/restored summaries, manifests/tokens, HAPs, signed public debug
profiles, source hashes and artifact hashes. Sample-key keystores are excluded.
Earlier 27-fault runs remain separate preliminary evidence; the figures above
refer to the final 39-fault version.

| Final HAP | SHA-256 |
| --- | --- |
| Desktop ordinary reader | `a57ae22d0f43f91c3243915356ab13fde94f50840c55cd0cb30dbed4e7651735` |
| Desktop Settings writer | `9e7afc5823d2e0a092af77a948962ba00a4dba0a5f74ab2fa815aa2d06bca035` |
| Phone ordinary reader | `3ea4ae3fd825391c94cac27910aa90192dcc6cf86b3dbcbe93a82d165c279328` |
| Phone Settings writer | `412b0ae8890940cea5fd4d7675759c5d8c6cf180a11f00c9c485ada8160ddec6` |

The production OHOS implementation SHA-256 is
`f5732a66b7193d5fd3bf506064e60ec5651e08e3cd06bd2ca63b56c0e0557180`.
The only subsequent fixture change is README explanation, not executable source.

Actual Settings service failure/revocation, environment teardown while workers
are pending, older SDK images, other process ABIs on real guests, and broader
module audits remain open. Controlled errors do not establish actual service
failure behavior. This is no new window/picker/input regression claim; their
previous checkpoint reports remain authoritative. The full HarmonyOS goal stays
active.

After phone restoration, NativeAPI_Desktop was cold-booted again and its exact
archived ordinary HAP reinstalled. Reader PID 3837/UID 20020058 passes another
177 native checks plus 39 controlled faults and reads the restored `emulator`
name. It is left running; this extra read is excluded from the six-phase totals.
`desktop-left-running.json` records the result and raw-log location.
