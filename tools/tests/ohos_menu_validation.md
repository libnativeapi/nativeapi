# HarmonyOS native menu validation

2026-10-15, macbook-pro Apple Silicon; DevEco Studio 26.0.0.851 and
HarmonyOS 7.0.0.107 / API 26 ARM64. Isolated guests: MateBook Pro/2in1
(computer, 3120×2080 at 304 dpi) and PuraXView (phone, 1320×2232).
One guest runs at a time. No host input or guest typing is injected.

Core commit: `8e1adb1` (`Implement HarmonyOS native menus and lifecycle`).

This is a native Menu/MenuItem implementation checkpoint, not full API completion.
The existing public C++ and C signatures are unchanged. The public native node
API does not expose generic Menu nodes, so the library's canonical ArkTS companion
uses the public UIContext/ComponentContent/PromptAction menu APIs. It presents real
Menu/MenuItem/MenuItem submenu components, without substituting an action sheet
or a dialog. The SDK's default computer subwindow and phone in-window policy is
preserved. The smoke fixture copies the same canonical library adapter at build
time; it has no alternate menu implementation.

## Runtime evidence

| Guest | Final menu PID | Native checks | Guarded UI checks |
| --- | ---: | ---: | ---: |
| Computer / 2in1 | 3716 | 40 passed | 4 passed |
| Phone | 4147 | 40 passed | 4 passed |

Zero failures in these final runs. Older successful runs remain in the evidence
folders but are not added to these totals. Earlier failed bring-up trials are
excluded from acceptance.

Computer regression PID 4596: 201 integration + 52 picker native + 5 dialog UI +
8 picker UI + 3 desktop-window UI checks = 269 passed, zero failures. Native
geometry still confirms title dragging, maximize and restore. Phone regression
PID 13591: 189 integration + 5 dialog UI checks passed; the unresolved phone
multi-selection/folder picker scenario was not invoked. The computer emulator and
passing fixture remain open.

Native logs,
per-press PID/UID/window ownership, selected/disabled UI states, full native trees,
screenshots and build logs are kept on macbook-pro in
`~/Library/Huawei/Logs/menu-validation/{desktop,phone}/`. Ignored local copies:
`core/build-ohos/verification/menu-validation/`.

The fixture verifies:

- Stable menu/item identities, collection insertion/enumeration/removal/clear,
  duplicate/null/index guards, separators, opaque-token wrapping and cycle guards.
- Normal, checkbox, radio and submenu configuration, radio mutual exclusion,
  rejected radio Mixed state, disabled state, icon and accelerator metadata.
- Native UI Menu/MenuItem nodes, PNG icon, mixed-state dash, Radio A unselected /
  Radio B selected, disabled input refusal, and actual selected checkbox state
  after the application's update. UI assertions read the system tree, not getters.
- Absolute and C ABI relative-rectangle/offset anchors. Requested logical screen
  positions are compared with native FrameNode positions and actual menu geometry
  using the guest's density. Boundary clamping is permitted only when the requested
  menu would extend beyond the native viewport.
- Live label/removal changes in the displayed native menu, actual submenu opening
  and item selection, paired menu and owning-item submenu lifecycle events.
- Explicit Close on a hidden submenu (false) and a visible submenu (closes its
  native popup hierarchy). Native lifecycle events arrive once.
- C ABI menu/item construction, direct C ABI Open with PositioningStrategy,
  native lifecycle and item clicks, owned handle release.
- Pending presentation cancellation without fabricated Opened/Closed events,
  programmatic root closure, already-closed false, object destruction and no late
  delivery; the final native tree contains no residual MenuItem nodes.

Each press uses a fresh UI layout and verifies the fixture's current process,
application UID and the native owning window. All input is confined to the named
NativeAPI emulator through its CLI. Screenshots supplement native-tree assertions.

The host waits for the observed anchor's actual position and measured dimensions
before opening. FrameNode positions are vp while measured size is px. Builders
keep their component receiver for deferred submenus; changed item data updates
native nodes through ComponentContent and keyed item reconstruction. Sessions
retain the original host reference through closure, callbacks use invalidatable
integer tokens and guarded emitters, and content disposal checks presentation
identity so a delayed callback cannot dispose a newer presentation with the same
menu ID. SDK disappearance drives lifecycle; confirmed window destruction also
invalidates its presentations. Host/page disposal cancels pending work.

## Reproduction and host checks

From core, with the selected guest unlocked:

```sh
NATIVEAPI_TEST_SUITE=menus tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

The standard runner validates package and phone/computer lifecycle before install.
Its PowerShell variant was parsed on windows-laptop; menu runtime acceptance was
performed only on macbook-pro ARM64. Build output also compiles x86_64, without
claiming an x86_64 runtime pass.

All 75 C ABI and 339 binding/reference outputs are current, with the same eight
known generation skips. Raw Rust/Dart regeneration produces no diff. On local
macOS: Rust workspace check, Dart analysis, C# build (zero warnings/errors), JS
(4 pass, 1 skip), Python (26 pass, 1 skip plus Ruff), Go (cached pass), and core
CTest (30 pass) succeeded. The common Menu backend dispatch excludes OHOS from
the generic stub definitions; other platform implementations are unchanged.

## Open work

- Cursor-position anchoring: the port currently rejects it; investigate a correct
  public current-pointer query, rather than using a stale event or fabricated zero.
- Actual keyboard and tooltip/hover interaction. The companion maps public character
  and FunctionKey shortcuts with Ctrl/Shift/Alt; remaining key/modifier semantics
  and any wider accelerator scope still need investigation and runtime evidence.
- The full 12-placement matrix, window-relative anchors, edge cases, multiple displays,
  secondary windows, multiple/replaced hosts, page transitions and env teardown.
- Long/empty/invalid content, type transitions, radio-group boundaries, remaining
  live property combinations, SDK error/rejection paths, and rapid reentrant operations.

The root API coverage ledger retains these cases and all remaining modules. The
original goal remains active. The unrelated user-staged `.claude/launch.json` is
preserved.
