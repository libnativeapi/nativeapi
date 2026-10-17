# HarmonyOS native-view hover tooltip checkpoint

2026-10-16, macbook-pro, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26,
ARM64 NativeAPI_Phone and NativeAPI_Desktop. The complete API goal remains open.

## Implemented behavior

View.SetTooltip previously only stored its optional text. It now displays real
hover tips for native View/Label/Button/TextField/ImageView nodes through the
optional canonical NativeTooltipHost companion supplied to AttachViewContent.
Public C++/C/binding signatures are unchanged. The companion is library source,
copied into the fixture by its build configuration, rather than a test-only popup.
Other platform implementations are unchanged.

Native NODE_ON_HOVER events work independently of business-event subscriptions.
After 700 ms, the companion opens a public SDK popup anchored to the actual native
node's unique ID. No invisible input overlay is added. The popup is nonmodal,
maskless, nonfocusable and uses noninteractive text content. Pending and visible
presentations are invalidated on exit, clearing/null/empty text, disabled/hidden
ancestors, removal, destruction, native-node replacement, ContentSlot unmount or
host replacement. Native predicates validate the live owning window, ancestors,
hover and node; the SDK companion also checks actual attachment/visibility.
Presentation/closure promises retain content until disposal. Tokens are monotonic,
and validation callbacks resolve the live registry instead of retaining a raw
View pointer. Environment cleanup avoids executing companion JavaScript.

TextInput to TextArea replacement retains View identity and tooltip configuration.
Events from old native handles are ignored even when their View token survives.
The new node may receive a fresh actual hover event while the pointer remains
stationary; the test distinguishes its new text and native ID from the old tip.
SDK-generated hover after native layout changes is legitimate input, rather than
an indication that an old tooltip survived.

The node-ID helper dynamically finds the API 20+ native query; older systems can
use the public NODE_UNIQUE_ID attribute. Both independent paths agree for a real
button and input in each API 26 guest. Missing-symbol behavior on an older image
is not claimed as runtime-verified. The public popup APIs start at API 18. Hosts
without a companion retain their existing ContentSlot contract and stored tooltip
values; repeated content/host attachment is idempotent and a new host replaces
only the companion.

Primary references: [public popup presentation and closure](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/arkts-apis-uicontext-promptaction.md#openpopup18),
[native node identities](https://github.com/openharmony/docs/blob/master/en/application-dev/ui/ndk-node-query-operate.md),
[native event/node APIs](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-arkui/capi-native-node-h.md),
and [guest mouse injection coordinates](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/dfx/uinput.md).
The installed public SDK declarations were also inspected. No additional permission
or system-app signing is needed for this tooltip implementation.

## Actual guest acceptance

Each final run requires exactly eight native C++/C assertions, 24 real popup scenes,
zero failures and exact single-popup text/geometry. Unicode/emoji text is also
visually inspected in captured compositor screenshots. Tests check updates while
hovering, C null/empty clearing, real underlying clicks, visibility and enabled
ancestors, pending removal/destruction, remove/readd, new TextArea nodes, content
detach/reattach and replacement of the host while a popup is visible.

All 24 scene snapshots retain the same real WindowManager focus owned by the
fixture. Both underlying controls receive exactly one real click. Pending disable,
remove, detach and destruction must arrive before the actual recorded hover
request reaches 700 ms; absence is checked after that deadline. A label without
business listeners proves that tooltip delivery does not depend on subscribing
to unrelated native events.

The runner derives every mouse target from a fresh native UI tree and validates
fixture PID/UID, actual owning system window and exactly one named guest. It
checks the guest's lack of a physical pointer device before injection. Public
uinput supplies guest-only mouse motion; no host mouse input or keyboard input
is used. Test-only Want commands change properties without moving the pointer.
The current guest service still reports no physical pointer; these observations
do not establish a positive DisplayManager cursor query.

| Guest | Tooltip acceptance | Existing views without companion | Parent regression |
| --- | --- | --- | --- |
| NativeAPI_Phone | PID 3580, UID 20020074; 8 native + 24 real scenes | PID 7023; 30 native + 3 guarded UI checks | PID 7119; 420 checks, SDK reparent refusal 801 |
| NativeAPI_Desktop | PID 3474, UID 20020058; 8 native + 24 real scenes | PID 7007; 32 native + 3 guarded UI checks | PID 7073; 420 checks, SDK reparent success 0 |

Phone final archive:
`~/Library/Huawei/Logs/tooltip-validation-20261016/phone-1792130487896410000/`.
HAP SHA-256: `5a1266b95a00eb528fe1dce0d4fdf028ee481b0fb2056a4d85f63a292a037377`.
Computer final archive:
`~/Library/Huawei/Logs/tooltip-validation-20261016/2in1-1792130809464788000/`.
HAP SHA-256: `c26fdd58683ca12dae0860f29ebfc56651ff0cd83ed372b578efc498912242b0`.
The same computer HAP was then relaunched for a live demonstration. PID 7643
(UID 20020058) is left running with the real Unicode tooltip visible; its separate
`desktop-demo/` and `desktop-left-running.json` evidence is excluded from totals.
Preliminary partial/full runs are retained separately and excluded from final totals.

## Reproduction and evidence

From core/tests/ohos_smoke on the host, with one named guest running and unlocked:

```sh
NATIVEAPI_OHOS_DEVICEINFO_WRITER=0 NATIVEAPI_OHOS_KEYBOARD_ACL=0 \
NATIVEAPI_TEST_SUITE=tooltip \
NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  bash run.sh /Applications/DevEco-Studio.app/Contents
```

The same installed ordinary HAP then runs the existing views and parent suites.
Archives contain the actual HAP, normal-bundle metadata, native logs, exact native
UI trees, focus/ownership/pointer-state dumps, decoded screenshots, source snapshots
and SHA-256 hashes. The HAP's manifest/device lifecycle is checked before archive.
Local ignored copies are under `core/build-ohos/verification/tooltip-validation-20261016/`.

## Host checks and remaining scope

All 75 C ABI and 339 binding/reference outputs are current, with the same eight
baseline skips. Raw Rust/Dart regeneration has no diff. macOS host checks pass:
CTest 33/33, Rust workspace, Dart analysis, C# build with zero warnings/errors,
JS native/type checks and 4 pass/1 skip, Python native build/lint and 26 pass/1 skip,
and Go native build/tests. Native libraries compile for ARM64 and x86_64; runtime
acceptance here is ARM64 C++/C plus ArkTS, not every language binding on OHOS.
The Python runner passes Ruff/formatting and the shell runner parses.

Multiple windows/environments, arbitrarily borrowed nodes, SDK refusal/allocation
failure, old SDK images, nested conflicting tips, screen-edge placement, full
font/theme scaling and environment-teardown races remain open. Existing View
IME/edit/submit/read-only/focus/selection audits and the rest of the complete API
coverage ledger remain in scope. No completion claim or push is made here. Core checkpoint: `8f206d360` on main.
