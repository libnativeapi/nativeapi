# HarmonyOS Preferences — 2026-10-16

Preferences now preserves Storage::Remove's absent-key result and shares each
scope's SDK handle across nativeapi instances. Both ordinary phone/computer
fixtures pass 215 assertions over eight rounds, including independent public
SDK interoperability, concurrent lifetime, storage limits and real process
restarts. This checkpoint does not complete the full HarmonyOS API objective.

## Problems reproduced and changes

The original backend passed the NDK Delete result directly to callers. On the
computer guest, baseline PID 8035 reproduced two failures: deleting a missing
key returned true, and a second deletion after successful C deletion also
returned true. The shared Storage contract requires false for absence.

The official [NDK implementation](https://github.com/openharmony/distributeddatamgr_preferences/blob/master/frameworks/ndk/src/oh_preferences.cpp)
also closes by removing the per-file SDK cache. Independently closing one
nativeapi instance can therefore affect another instance using that same file.
The replacement keeps one SDK handle and mutex per exact scope. Set/Delete/Clear
and their flushes run under that mutex; Remove checks membership before deleting.
Only the final nativeapi owner closes the handle. A registry lock serializes
acquisition with the complete final close, preventing an old close from evicting
a newly opened store. Unused entries are removed rather than retained forever.

File names, byte encoding, the NDK storage-engine selection and existing data
locations are preserved. No migration, new permission or public signature is
introduced. The constructor documentation now matches all six existing platform
implementations: the default scope is the literal `default`.

The SDK reference originally used the Ability/module context and saw a different
store. The [NDK source](https://github.com/openharmony/distributeddatamgr_preferences/blob/master/frameworks/ndk/src/oh_preferences.cpp)
uses the application context's preferences directory. The independent ArkTS
reference now uses getApplicationContext, the exact encoded file name and GSKV
on these GSKV-capable guests. The [NDK options source](https://github.com/openharmony/distributeddatamgr_preferences/blob/master/frameworks/ndk/src/oh_preferences_option.cpp)
selects GSKV when available and XML otherwise. Merely matching logical names
across different context directories or storage engines is not interoperability.
Production retains this existing SDK selection policy.

## Final guest evidence

Host: macbook-pro, Apple Silicon, DevEco Studio 26.0.0.851; HarmonyOS
7.0.0.107/API 26, ARM64. One cold-booted named guest runs at a time. Both readers
are dev.nativeapi.smoke, non-system applications with normal privilege level.
No keyboard or mouse input is sent by the Preferences workflow. Guest unlocking
uses the separately guarded emulator unlock helper.

| Guest | Three reader PIDs | UID | Rounds | Assertions | Failures |
| --- | --- | ---: | ---: | ---: | ---: |
| NativeAPI_Desktop | 16173, 16919, 17201 | 20020058 | 8 | 215 | 0 |
| NativeAPI_Phone | 3394, 3486, 3766 | 20020074 | 8 | 215 | 0 |

The runner requires exact per-round assertion counts and exact SDK maps:

| Round | Native assertions | Evidence |
| --- | ---: | --- |
| seed | 30 | Small real UI C++/C operations, all worker storage methods, constructor metadata, ownership, scope isolation and invalid NUL inputs. |
| sdk-write | 0 | Independent public ArkTS SDK puts and flushes a fourth Unicode value. |
| verify | 10 | C++ and C read that SDK value and the complete persistent map. |
| concurrent | 138 | Eight competing C++/C removals yield exactly one success; eight workers write/read/remove 48 disjoint keys; 32 unanchored concurrent open/write/remove/close sequences pass. |
| limits | 12 | 1024-byte key and 16 MiB value roundtrip; oversized inputs fail without losing the prior value; boundary data is removed. |
| verify after first actual process restart | 10 | All four exact strings remain, including the independently written SDK value. |
| clear | 8 | C clear is visible through live C++ instances, and empty clear/missing deletion retain their specified results. |
| empty after second actual process restart | 7 | Native and fresh SDK maps remain empty. |

Both constructors and all inherited storage methods are exercised. Default-scope
construction checks metadata only and never mutates that scope. Mutations use a
unique bounded test scope, cleared at the end. Scope variants containing path
traversal characters, Unicode, a slash/hex-like spelling and an embedded NUL
retain distinct identities and clear independently inside the application sandbox.
Embedded NUL keys/values fail without truncation or changing existing contents.

Strings include Chinese/emoji, the empty value, a real newline/tab, equals and
a literal backslash-n sequence. C++/C enumeration matches exact key/value maps;
mutating an owning C map does not affect stored data. Returned strings, lists and
maps are released with their ABI helpers. Released C handles fail safely. The UI
case frees a C instance while its C++ peer remains live and successfully writes
again, directly exercising shared-handle lifetime.

All worker operations run on actual native threads, and N-API completion returns
to the owning UI loop. The concurrent round uses a second scope with no permanent
anchor so final close and new acquisition can overlap. The two restarts are real
force-stop/new-process launches with different PIDs, not object reconstruction.
Every command checks the current bundle/process/UID and installed normal profile.

## Build and evidence records

Local macOS CTest passes 33/33. Rust workspace check, Dart analysis, C# build,
JS native/type/tests (four pass, one existing skip), Python native/lint/tests
(26 pass, one existing skip), and Go native/tests pass. C ABI/all six generated
outputs, Rust bindgen and Dart ffigen are current. Codegen check passes with the
same eight baseline unsupported-symbol skips; fixture Python E/F/W/B/UP/I lint,
syntax checking and shell syntax checking pass. No generated method or symbol
was added or removed. The five other platform implementations are unchanged.
These host checks do not establish six-language runtime behavior inside OHOS;
the real guest checks cover C++/C plus the independent ArkTS SDK.

Remote archives are under `~/Library/Huawei/Logs/preferences-validation-20261016/`
on macbook-pro, copied to local `core/build-ohos/verification/` with the same suffix:

- `desktop-16173-16919-17201`, roots `2in1-1792123541629500000`,
  `2in1-1792123546242791000`, `2in1-1792123547803859000`.
- `phone-3394-3486-3766`, roots `phone-1792123658431828000`,
  `phone-1792123663359099000`, `phone-1792123664873502000`.
- `host-checks`, plus the original `preferences-desktop-baseline-8035.log`.

Each final archive includes all three process roots, per-round native/SDK records,
raw logs, actual bundle metadata, build logs, exact HAP, source hashes and HAP
hash. The desktop final HAP SHA-256 is
`8f954e429f940b806ae8111f95d0a6313095fa38cb539d125b93804455ea6134`;
phone is `c1d97981cf64d50c7acb1330aa720d3e4d22635e5d9517a3df1414db02d39eb2`.
The OHOS production backend SHA-256 is
`209cbc415040e7c9ac9a44391ca4c3f560eb55071748473b6b351f7b2ba31d5a`.
Only explanatory README text changed after the source snapshots.

Actual storage failures/full disk, unsupported/failed SDK open, external SDK
handles held concurrently with nativeapi, multiple linked core instances,
cross-process writes, cross-application sandbox isolation, foreign SDK value
types, other ABIs and older/XML-only images remain open. Engine fallback is
retained from the SDK but is not proved by GSKV guest results. There is no new
window/picker/input regression claim. The full HarmonyOS API goal stays active.

After phone acceptance, the computer guest was cold-booted again and the exact
archived desktop HAP reinstalled. PID 3350/UID 20020058 passes seven additional
native assertions and a fresh SDK empty-map comparison against the original
cleared test scope, proving that clear also survives the guest cold boot. It is
left running. This extra read is excluded from the 215 assertions above;
`desktop-left-running.json` records its raw-log location.
