# HarmonyOS foundation/value API validation

Checkpoint date: 2026-10-16. This is one checkpoint in the continuing
[full API coverage ledger](ohos_api_coverage.md), not completion of the overall goal.

Three shared implementation defects are repaired:

- `Color::FromHex` returns transparent for null, malformed lengths and non-hex
  characters without throwing. Direct C++ and the C ABI now agree. The parser
  accepts the existing RGB/RGBA/RRGGBB/RRGGBBAA forms, with optional `#`, and
  inspects at most nine characters after the prefix.
- C++ → C conversion of flags enums preserves combined values. Previously a
  Ctrl+Alt mask of 6 became None (0), since only individually named variants
  survived the generated switch. The generator recognizes power-of-two flags;
  ordinary enums keep their previous fallback. All 256 keyboard modifier masks
  are checked for exact C struct roundtrip, including owned string release.
- Window-relative `PositioningStrategy` values retain `std::shared_ptr<Window>`.
  Copies/moves preserve the wrapper; the new exported `GetRelativeWindow()`
  returns a shared reference, and its C getter returns an independently owning
  window handle. C++ callers now use `Relative(window)` instead of
  `Relative(*window)`. Null windows yield an empty relative rectangle and no
  window reference. The C factory's existing symbol/signature is unchanged.

The baseline core `8f206d360` was compiled in an isolated probe: null/empty/invalid
colors threw in all three selected cases, and Ctrl+Alt converted from 6 to 0.
The same mask converts to 6 after the repair. This probe never dereferenced a
freed window or sent input. Raw results are in `host-checks/baseline.json`.

## Real emulator results

Host: `macbook-pro`, Apple Silicon. DevEco Studio 26.0.0.851, HarmonyOS
7.0.0.107 / API 26, ARM64. One named emulator ran at a time. Both HAPs were
checked for their actual phone/computer lifecycle manifest before installation;
installed bundle metadata confirmed an ordinary app (`normal`, not system-app).

| Guest | App PID / UID | UI checks | Worker checks | Window checks | Independent SDK scenes |
| --- | --- | ---: | ---: | ---: | ---: |
| NativeAPI_Desktop | 26942 / 20020058 | 162 | 162 | 14 | 2 |
| NativeAPI_Phone | 3957 / 20020074 | 162 | 162 | 14 | 2 |

The UI and worker execute the same portable C++/C test body. The extra guest
check confirms the actual UI/independent-worker thread, so host CTest reports
161 checks for that body. Covered contracts:

- Color constructors, all four hex formats, case/hash/alpha/default alpha,
  18 invalid/null input cases, both integer packing orders and all nine constants.
- Fractional signed point/size/rectangle conversions, both inset factories and
  edge ordering; all 12 placement enum mappings.
- Accelerator default/explicit construction, empty values, string order,
  equality/inequality, C strings, all three C++ modifier operators and every
  eight-bit modifier combination. These are values; no shortcut is registered
  and no keys are sent by this suite.
- Polygon constructor/add/clear/count/get, fractional/inclusive limits, invalid
  coordinates preserving previous data, exact 4096-vertex limit, copies/moves,
  C getters and stale/cross-type handle rejection.
- Shadow constructor and all six accessors, defaults, inclusive finite bounds,
  NaN/infinity/out-of-range refusal preserving previous values, copy isolation,
  C getters and stale handles. This does not establish native shadow rendering.
- Absolute/cursor/rectangle/null-window strategy factories and getters,
  offset preservation, copy/move independence and stale C handles.
- All eight EventRequest/EventDecision consumer methods through C++/C: required
  requests, synchronous veto, deferred acceptance/veto, dropped votes, producer
  invalidation, late replies, C ownership, 16 concurrent voters and reentrant
  completion outside the mutex. The private producer protocol is test setup;
  actual OS close integration remains documented in its separate checkpoint.

The fixture creates a real SDK subwindow. ArkTS independently moves/resizes it
first to `(180,300,540,420)` then `(270,450,630,480)` in physical pixels. In both
scenes, the original C++ strategy, its copied/moved value and the C strategy
match the SDK's content rectangle, using `windowRect + drawableRect` origin
and the owning display's `densityPixels`. Original C window handles were freed
before these observations. Maximum permitted comparison error: 0.002 logical
pixels (native JSON serialization precision).

| Guest | SDK content rectangle, first scene | Second scene |
| --- | --- | --- |
| Desktop | (94.7368433,157.8947388,284.2105299,221.0526344) | (142.1052649,236.8421082,331.5789515,252.6315821) |
| Phone | (60,100,180,140) | (90,150,210,160) |

After actual SDK destruction, both C++ and C content rectangles are empty,
lookup no longer returns the child, and the owning C getter still safely holds
its invalidated wrapper after all strategy values/handles are released.
Releasing that getter drops exactly its own shared reference. Settled SDK
promise callbacks may retain additional references until JS garbage collection;
this suite does not equate native destruction with immediate C++ destruction.
An isolated OHOS wrapper with no registry/promise references separately proves
that strategies keep the source alive, the owning C getter can be its sole
remaining owner, and its final release destroys the wrapper.

No host keyboard/mouse input was used. The pre-existing guarded guest unlock
helper was used only within the isolated named emulators.

## Artifacts and host checks

Remote archive: `~/Library/Huawei/Logs/foundation-validation-20261016/`.
Local copy: `core/build-ohos/verification/foundation-validation-20261016/`.
Each archive includes HAP, build log, exact source fingerprints, bundle metadata,
PID/UID, raw own-process logs, independent SDK scene JSON and summary:

- Desktop: `2in1-1792132354575166000`, HAP SHA-256
  `2a507f50619ec41e9aa86cdc4aa765f9c0b1bdfc350c8839692655a738bfb702`.
- Phone: `phone-1792132507841628000`, HAP SHA-256
  `580c480f616011de0e796b3b9f1fc17f8e1467ee73f8bf801814960e857f413e`.
- `host-checks/`: baseline results, CTest, generator and binding verification.
- `desktop-left-running/`: original verified desktop HAP reinstalled after phone
  acceptance; test page rerun successfully and left visible, PID 3078. This
  convenience rerun is excluded from the table totals.

Both local HAP hashes and all archived compiled-source fingerprints were checked
against the retrieved packages/current source. Both ARM64 and x86_64 HAP libraries
compiled; guest runtime evidence is ARM64 only.

Host macOS checks: CTest **34/34**, including the 161-check shared value suite;
C ABI generator **5/5**; codegen **339 outputs current**, with seven existing
skips (the relative-window getter is no longer skipped). Rust workspace check,
Dart analysis, C# solution build (0 warnings/errors), JS native build/typecheck
and tests (4 passed, 1 existing skip), Python native build/lint/tests (26 passed,
1 existing skip), and Go native build/tests all passed. Raw Rust/Dart FFI was
regenerated. The getter was inspected in all six generated bindings and website
reference. Runtime guest tests here use C++/C and ArkTS, not six language runtimes.

The implementation is shared by all six core platforms. macOS and OHOS compile
coverage is recorded above; the Linux menu caller was updated to retain the
shared result but was not compiled on Linux in this checkpoint. Windows,
Android and iOS were not newly built. No new platform stub was introduced.

Native menu placements, cursor availability, extra modifier display names,
remaining keyboard events, shadow application/rendering and embedding/teardown
scenarios retain their existing open ledger entries. Passing value contracts
is not a claim that those native capabilities have been completed.
