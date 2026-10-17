# HarmonyOS content aspect constraint checkpoint

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851, API 26 / HarmonyOS
7.0.0.107. ARM64 guests: NativeAPI_Desktop (MateBook Pro, 2in1) and
NativeAPI_Phone (Pura X View). The full API goal remains active.

## Implementation and contract

Window::SetAspectRatio now uses setContentAspectRatio(ratio, false, false)
for positive finite ratios. The API 21+ SDK operation constrains content during
user resizing, without immediately resizing the window or saving the constraint
for later windows. Zero and negative finite values call resetAspectRatio.
Nonfinite values are ignored. The SDK reset also clears any saved SDK ratio;
this side effect is documented in the public availability note.
The older persistent setAspectRatio operation is
not substituted because its lifetime and immediate-resize behavior differ from
the core contract. SDK failures preserve the last successful value.

GetAspectRatio reports the last successful nativeapi constraint, shared by C++,
borrowed wrappers and C handles of the same live window. The SDK has no ratio
getter, so direct host changes are explicitly documented as outside that value.
Native geometry is independently tested, including a direct SDK ratio change
whose actual result differs from the nativeapi getter.

Each window serializes queued requests through real SDK promises. UI calls
submit without blocking the Ability loop; worker calls wait for completion.
The value changes only after SDK success. Destroying windows and environment
cleanup release queued requests through the existing SdkRequest machinery;
requests are rejected once destruction begins. No public signatures changed.

Reference: [public content aspect and reset APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-window-Window.md#setcontentaspectratio21).
Installed SDK declarations were inspected as well.

## Computer acceptance

Final **PID 23526: 52 native assertions, 16 actual UI scenes and 15 real native
mouse edge resizes, zero failures**. All presses start inside the straight right
border, away from transparent rounded corners. Fresh guest PID, UID, native tree
ownership and WMS window identity are checked before each atomic gesture. No
host mouse or keyboard input is used.

Scenes cover square, wide and fractional constraints; programmatic content-size
changes; nonfinite input; SDK rejection of an extreme finite ratio (1300016);
three queued UI requests; C/worker operations; direct host SDK changes; hidden
and normal title bars; zero/negative clearing; maximum-size intersection;
ordinary child refusal (1300004); and closing a child with queued requests.
The immediate queued UI result must still contain three pending operations and
the previous successful ratio, followed by the final 1.25 value after completion.

WMS rectangles and independent SDK drawable rectangles verify actual content
ratios. Examples after a right-edge drag from a 1500×1050 frame:

| Scene | Actual outer frame | Decoration height | Actual content ratio |
| --- | --- | ---: | ---: |
| Square | 1580×1650 | 70 | 1 |
| Wide | 1580×1123 | 70 | 1.500475 |
| Direct SDK ratio 2 | 1580×860 | 70 | 2 |
| Hidden title bar | 1580×1264 | 0 | 1.25 |
| Cleared constraint | 1580×1050 | 70 | 1.612245, unrestricted |
| SDK width cap 1600 | 1600×1136 | 70 | 1.500938 |

Across the constrained scenes, the largest measured ratio error is below 0.001,
consistent with native integer-pixel rounding. Programmatic SetContentSize
does not obey the ratio, and setting a ratio does not itself change the frame.
SDK size limits are restored before the suite finishes.

Window-control regression **PID 26765: 61 native assertions and 18 actual UI
scenes, zero failures**, including hidden-title overrides and maximize/recovery.

Final general/window regression **PID 4109: 269 checks, zero failures**:
201 native general checks, 52 native picker checks, 5 actual dialog scenes,
8 picker UI checks and 3 actual title-drag/maximize/restore checks.

The picker harness now uses a per-run clock token alongside the PID, and reads
the exact current native filename from the log for selection. This fixes the
reboot/PID-reuse collision retained in the preceding checkpoint, while keeping
the fresh-empty-file and original-content assertions. The general run above
verifies both new filenames, actual writes/reads, single/multiple selection,
folder selection and cancellation. Name parsing also checks prior and unique
formats with extensions shown or hidden. Phone picker selection with the new
longer names has not been rerun in this checkpoint and remains a follow-up;
the phone aspect suite is independent of that picker path.

## Phone acceptance and floating-mode limit

**PID 3783: 51 native assertions and 16 actual UI scenes, zero failures**.
The public main-window ratio request succeeds (0), while ordinary child requests
reject with 1300004. C++/borrowed/C values, ordered requests, worker completion,
invalid-value retention, reset and closed wrappers pass. All full-screen main
rectangles remain 1320×2232; no native edge resizing is claimed on this guest.

An independent fifteen-scene SDK mode probe, **PID 3909**, confirms recover()
returns 801 and stage floating-mode selection returns 801. The window stays in
status 1 and retains its full-screen geometry throughout. Some maximize calls
resolve with code 0 while retaining that same geometry; they are not counted as
proof of floating or resizing support. Ratio acceptance therefore establishes
configuration on this phone, with effectiveness in floating mode verified on
the computer. Other phone forms or system modes remain open.

## Reproduction, retained evidence and remaining audits

With one unlocked named disposable guest, from core:

```sh
NATIVEAPI_TEST_SUITE=window-aspect \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  tests/ohos_smoke/run.sh /Applications/DevEco-Studio.app/Contents
```

Evidence lives remotely at `~/Library/Huawei/Logs/window-aspect-validation/` and
locally under ignored `core/build-ohos/verification/window-aspect-validation/`.
It includes normal bundle metadata, SDK/native logs, native ownership trees,
WMS dumps, before/after resize measurements and decoded screenshots.
Final computer: `2in1-1792087925708033000`; phone:
`phone-1792088229764730000`; phone SDK probe:
`phone-1792088314624050000`. Earlier development runs are separately retained.

CTest passes 31/31. All six local macOS binding checks pass (JS and Python each
retain one existing skip). Code generation, raw Rust/Dart generation and final
codegen check pass: 75 C ABI and 339 binding/reference files current, with the
eight baseline skips. Both ARM64 and x86_64 HAP native libraries build; only
ARM64 guests were executed. PowerShell integration is updated but unexecuted
because PowerShell is unavailable locally. Windows runtime is not claimed.

Persistence across process/device restart, additional SDK versions, conflicting
limit changes during a pending ratio operation, external host races and a
separate environment-teardown runtime scenario remain open. So do the other
window capability policies and the rest of the full API ledger.
