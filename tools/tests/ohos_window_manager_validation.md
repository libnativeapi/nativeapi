# HarmonyOS window manager queries — 2026-10-16

GetWindowAtPoint now uses the public SDK's actual application window stack,
including focus-independent child z-levels and SDK raises. Both ordinary named
guests pass 1,188 native assertions over nine real window scenes. Get, GetAll,
GetCurrent, C ownership, exclusions and destroyed identities are covered alongside
the point query. This checkpoint does not complete the full HarmonyOS API goal
or the remaining WindowManager audit.

## Reproduced failure and resulting behavior

The original query sorted only observed show/focus operations. The computer
baseline's main window had actual SDK ID 73; children were 74 and 75. After focus
returned to the main window, GetCurrent correctly returned 73, but the old point
query also returned 73. The independent SDK stack at the same real point was
`[75, 74, 73]`. Focus and frontmost window were different. The baseline used the
original WindowManager query body, preserved with its raw logs and nine scenes.

The public [window SDK reference](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkui/arkts-apis-window-f.md)
defines getWindowsByCoordinate as a promise returning this application's visible
windows in stacking order, with a display ID selecting the display. The installed
API 26 declarations and both actual guests agree with this behavior.

Workers now submit a fresh query after invocation and wait for its real SDK
completion. UI callers submit without blocking and use the last completed stack;
before a successful result, the existing observed-order fallback is retained.
SDK rejection keeps the last completed order or that initial fallback. This UI
behavior is explicit in the public header and generated reference: the first UI
read after an external layer change can be stale. A completed worker refresh
updates the subsequent UI read.

Requests and caches are grouped by the SDK display ID of every attached window,
not only the primary window. UI requests coalesce per display; worker calls do
not reuse an earlier pending UI query. Per-display completion sequences prevent
older results from replacing newer ones. Environment identity, actual attached
window membership and cleanup protect retained data. Removed display mappings
and caches are pruned. Multi-display coordinate behavior still needs real runtime
evidence; grouped implementation is not a multi-display acceptance claim.

GetAll continues to enumerate live managed windows, including hidden ones. Point
queries use only the completed SDK-visible set and the current window bounds.
Nonfinite points return no hit. The public lookup catches native exceptions and
returns nullptr; failed UI dispatch cannot return a partially reordered vector.
SDK numeric arguments are checked before publishing a pending UI request. Lookup and enumeration still drop destroyed
windows while retained wrappers keep their identity. No public signature, C ABI
symbol or handle type was added. The five other platform implementations are
unchanged.

## Final named guest results

Host: macbook-pro, Apple Silicon; DevEco Studio 26.0.0.851, HarmonyOS
7.0.0.107/API 26, ARM64. One cold-booted guest runs at a time. The tested HAP is
ordinary dev.nativeapi.smoke; metadata/storage regressions independently verify
its normal privilege and absence of device-name mutation permission.

| Guest | Manager PID / UID | Scenes | Native checks | Worker samples | SDK-compared scenes | Initial UI results changed after refresh |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| NativeAPI_Desktop | 15551 / 20020058 | 9 | 1,188 | 72 | 9 | 4 |
| NativeAPI_Phone | 3649 / 20020074 | 9 | 1,188 | 72 | 9 | 6 |

All final checks have zero failures. All SDK query and action result codes are
zero on both guests; no unsupported case is counted as a passing scene. Each
scene requires exactly 132 native assertions, eight samples from four real C++
threads, exact C++/C equality, a coherent worker result and the matching refreshed
UI result. The runner additionally requires at least two actual stale-initial-UI
to-fresh-worker changes, making nonblocking completion observable.

The nine scenes are: baseline overlapping children, focus main, A higher than B,
B higher than A, equal child levels plus direct SDK raise of A, hide B, show B,
direct SDK destruction of B, and nativeapi close of A. Child geometry and the
query point come from actual SDK rectangles/density. Direct SDK point queries
supply the expected ordered IDs for no exclusion and each of main/A/B excluded.
Phone SDK visibility can exclude a covered child; registry enumeration still
retains that live window, so visible-list size is not substituted for ownership.

Assertions also check canonical C++ identity, C list/handle ownership, caller edits
to snapshots, invalid IDs, and nonfinite coordinates. After destruction, retained
wrappers have nonzero stable IDs but neither C++ Get nor the C lookup can retrieve
them. Hidden windows remain managed until actual destruction. Current-window
identity agrees with independent SDK focus, with the documented primary fallback.

The scenario manipulates actual SDK windows and takes live queries; it does not
mock the SDK, patch its module, or replace production request callbacks. No mouse
or keyboard input is sent by this scenario. Guest unlocks use the separately
guarded emulator helper.

## Test export repair and regressions

The preceding Preferences fixture change added an entry to a descriptor array
but left its registration count at one. This hid the DeviceInfo test export,
not the production C++/C DeviceInfo implementation. The count now follows the
actual array length. Both complete suites pass against each final installed HAP:

| Guest | DeviceInfo reader PIDs | DeviceInfo checks / controlled faults | Preferences reader PIDs | Preferences checks |
| --- | --- | ---: | --- | ---: |
| Desktop | 15615, 16042 | 1,062 / 78 | 16429, 16633, 16919 | 215 |
| Phone | 3766, 4178 | 1,062 / 78 | 4607, 4831, 5129 | 215 |

Both device names are restored to `emulator`; the isolated preference scopes are
cleared, including persisted-clear checks after real process restarts. The
regressions demonstrate that both exports remain callable, including real name
changes, C++/C workers, isolated SDK errors and SDK/native preferences exchange.

Local macOS CTest passes 33/33. Rust workspace check, Dart analysis, C# build,
JS native/type/tests (four pass, one existing skip), Python native/lint/tests
(26 pass, one existing skip), and Go native/tests pass. C ABI and all six binding
outputs, Rust bindgen and Dart ffigen are current. Codegen check passes with the
same eight baseline unsupported-symbol skips. Fixture Python syntax/selected
E/F/W/B/UP/I lint and shell syntax checks pass. These host checks are not evidence
for all six language runtimes inside OHOS; the actual guest scope is C++/C and
the independent ArkTS SDK.

## Evidence and remaining scope

Remote archives are under `~/Library/Huawei/Logs/window-manager-validation-20261016/`
on macbook-pro, copied to local `core/build-ohos/verification/` with the same suffix:

- `desktop-15551`: manager root `2in1-1792127169407332000` and all five metadata/
  storage regression process roots.
- `phone-3649`: manager root `phone-1792127295419864000` and all five regression roots.
- `baseline-original-query`: original query source, build/failure logs and root
  `2in1-1792124840833908000`.
- `host-checks`: local verification logs.

Each final case includes raw logs, per-scene SDK/native results, bundle metadata,
checkpoint summaries, exact HAP, HAP hash and source hashes. Earlier archived runs
are preliminary; these final directories include the retained-closed-ID lookup
assertions and failure protection.

Desktop HAP SHA-256:
`c4e7a290f6d3931474385eb7808466bc839d49aee88cb95e1dc99b587143525a`.
Phone HAP SHA-256:
`33ae2df689130972b7f1fab752dc670646de321de476db6afd5ad5e122f6fce1`.
Both final production sources match the worktree:
ohos_window.cpp `ff7b2d80b77a089ff8c7d69bf34a726f5a2871314c75a7e2753983eb06237114`,
window_manager_ohos.cpp `44bc93d47038f24aeae24bf5525d4c852468824b00ba87ae6c84f0681c8a159b`.

Actual multi-display/hotplug coordinates, foreign-application coverage, unattached
SDK windows, mask/clipping/edge points, SDK failures and pending-request environment
teardown, including SDK ID reuse and live environment replacement, remain open.
The eight legacy swizzle-related methods are retained and
unchanged; this checkpoint does not classify their no-ops as completed. Their
existing retirement/design issue and remaining manager event/ownership audits
stay in the full goal. There is no new general picker, window dragging or global
keyboard-input regression claim.

After final phone acceptance, NativeAPI_Desktop was cold-booted and its exact
archived HAP reinstalled. PID 3381/UID 20020058 again passes all nine scenes and
1,188 assertions; it is left running with only the main test window. This extra
cold-boot run is excluded from the two main table rows. Its summary and raw-log
location are recorded in `desktop-left-running.json`.
