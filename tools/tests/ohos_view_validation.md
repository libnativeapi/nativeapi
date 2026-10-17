# HarmonyOS native view validation

2026-10-15 on macbook-pro (Apple Silicon), DevEco Studio 26.0.0.851,
HarmonyOS 7.0.0.107 / API 26 ARM64. The computer guest is MateBook Pro/2in1,
3120×2080 at 304 dpi; the phone guest is PuraXView, 1320×2232. Only one
isolated NativeAPI guest runs at a time.

Implementation commit: core `aa49297` (not pushed).

The implementation uses public native ArkUI nodes and the host's NodeContent /
ContentSlot. Existing C++ and C signatures are unchanged. Public availability and
limitations are documented in view.h/window.h and regenerated into the API
reference. No SDK-private window/content API or fake native event is used.

The fixture mounts, unmounts and reattaches the slot, then verifies:

- Cached root identity and native window association; unsupported backend rejection.
- Shared ownership, invalid cycles, detachment and child survival after parent destruction.
- Native measured dimensions against logical layout; row flex, spacing, padding,
  hidden children and detached absolute coordinates.
- Native label text including Chinese/emoji, font/colour/alignment and intrinsic measurement.
- Prefilled TextInput and TextArea, preserved View identity and properties across
  native-node replacement, and no changed events for programmatic SetText.
- Native password masking with a separate single-line field. Conflicting secure /
  multiline requests preserve the previous mode.
- Retained PixelMap and DrawableDescriptor, natural image dimensions, contain
  fitting, real Image node appearance, hiding and clearing.
- Focus and native enabled state. A guarded click on the disabled control emits
  no click; subsequent enabled C++ and C ABI clicks arrive exactly once.
- Removal and release of the C handle, native tree cleanup and retained-field use.
- Computer-only: actual window resize changes the root and child layout, then
  restoring window dimensions restores their measured geometry.

Each press checks the fresh UI tree's unique owning bundle/window, WindowManager
PID, process identity and application UID. The named Emulator CLI operates only
inside that disposable guest. No host input or guest typing is used. Full UI
snapshots distinguish the fixture Image from computer title-bar icons. The
screenshots were visually inspected for text, disabled appearance, password
masking and the six-colour image.

The tested runtime returned zero from NODE_POSITION's getter for positioned
children although the UI rendered them at the expected coordinates. The backend
now reads attached coordinates through NodeUtils_GetLayoutPosition and converts
physical pixels using the owning window density. A detached view reports its
last successfully applied logical position. Tests verify real measured geometry,
not only stored setter values.

## Results

| Guest | Final view PID | Native checks | Guarded UI checks |
| --- | ---: | ---: | ---: |
| Computer / 2in1 | 3070 | 32 passed | 3 passed |
| Phone | 3490 | 30 passed | 3 passed |

Zero failures in these final runs. The computer includes the two resize/restore
checks. Earlier successful processes (computer 28050; phone 3876/7924) remain in
the evidence directory but are not added to these totals.

Phone service/window/dialog regression: PID 3985, 189 native + 5 dialog UI checks,
zero failures. No unresolved phone multi-selection/folder picker case was invoked.
Computer regression: PID 3738, 201 native integration + 52 picker native +
5 dialog UI + 8 picker UI + 3 desktop window UI checks = 269 passed, zero failures.
Title dragging, maximizing and restoring still operate in the actual computer guest.
Raw evidence is retained on macbook-pro under
`~/Library/Huawei/Logs/view-validation/{desktop,phone}/`, including per-PID
native logs, ownership snapshots, window tables, layouts, screenshots and builds.
Local ignored copies are in `core/build-ohos/verification/view-validation/`.

Reproduce with `NATIVEAPI_TEST_SUITE=views tests/ohos_smoke/run.sh <DevEcoRoot>`
from core, with the chosen guest unlocked. The Bash runner was executed on macOS;
the PowerShell runner was parsed on windows-laptop but its view suite was not
executed there.

## Checks and remaining audit

All 75 C ABI and 339 binding/reference outputs are current; the same eight known
codegen skips remain. Raw Rust/Dart FFI regeneration produces no changes. On the
local macOS host: Rust workspace check, Dart analysis, C# build (zero warnings /
errors), JS (4 passed, 1 skipped), Python (26 passed, 1 skipped plus Ruff), Go
(cached pass), and core CTest (30 passed) succeeded. These host checks are not
claims that every binding ran in HarmonyOS. Platform files outside OHOS were not
changed.

Open: user-edit/change/submit and IME input, actual read-only edit refusal,
focus/selection transitions during replacement, tooltip display, arbitrary
borrowed-node interoperability, secondary windows, density/theme/font scaling,
API-failure branches and environment teardown with retained objects. Node API
has no native tooltip attribute; a hover/popup adapter is still an implementation
item, not an unsupported-platform classification. Controls' natural sizing uses
the public MeasureUtils text engine with native-style padding/minimum dimensions;
theme-dependent intrinsic sizing still needs an audit. The full API goal remains
active; this is one implementation checkpoint.

The user-staged `.claude/launch.json` is unrelated and remains untouched.
