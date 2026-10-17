# HarmonyOS API implementation and verification coverage

Goal: implement every existing nativeapi API that a normal HarmonyOS application
can implement, and verify its behavior in both a phone and computer emulator.
This is an open work ledger, not a completion claim. The broader project is paused;
the separately requested clipboard fixes and two-guest acceptance are recorded below.
An unsupported classification
requires a public SDK/permission/semantic reason; existing stubs alone are not evidence.

Scope baseline: the 36 public headers in the current codegen IR. The IR is an inventory baseline, not a scope boundary: C++-only free APIs and
public headers outside `API_HEADERS` also remain in scope. All methods and
constructors remain in scope, including parameters, errors, events, lifecycle and
binding integration. Shared value APIs also need coverage. No API is considered
implemented merely because a setter accepts input or a capability returns true.

| Public header | Methods/constructors in IR | Status / next evidence |
| --- | ---: | --- |
| `clipboard.h` | 14 | Public NDK multi-format backend and independent SDK-app interoperability accepted on both guests. Follow-up fixes submission-time revision checks, subscription epoch isolation and actual environment cleanup. Each guest passes 474 boundary assertions / 58 commands, plus the original 34-assertion / 17-command cross-app regression. Exact text/HTML/RGBA/file bytes, nested directory transfer, actual permission refusal/restoration, removed source files, C++/C/worker reads and three real environment lifetimes are covered. [Boundary evidence](ohos_clipboard_boundaries_validation.md). Interactive system File Manager UI, privileged URI-grant revocation, peer uninstall and broader codec/service fault injection are not claimed by this acceptance. |
| `geometry.h` | 2 | Shared value/ABI checkpoint: fractional signed Point/Size/Rectangle conversions and All/Symmetric edge order verified on UI/workers in both guests; no native geometry operation implied. [Evidence](ohos_foundation_values_validation.md). |
| `color.h` | 4 | All four methods and nine constants verified through C++/C on both guests. Malformed/null hex now returns transparent without throwing; all four hex formats, case, alpha and RGBA/ARGB packing covered. [Evidence](ohos_foundation_values_validation.md). |
| `keyboard.h` | 2 | Value/ABI checkpoint: constructors, empty/string/equality semantics and C++ modifier operators tested in both guests; fixed C conversion losing combined flags and verified all 256 masks. Actual input/event lifecycle remains in the keyboard/shortcut checkpoints; extra modifier display-name policy remains open. [Evidence](ohos_foundation_values_validation.md). |
| `event_request.h` | 8 | All eight consumer methods covered through C++/C on UI/workers in both guests, with required/cancelled/deferred/dropped/invalidated/late/concurrent/reentrant producer scenarios. These are shared decision-state tests; real system close is covered separately and other event producers remain open. [Evidence](ohos_foundation_values_validation.md). |
| `placement.h` | 0 | All 12 enum conversions verified in both guests. Actual menu/popup placement is a separate runtime audit. [Evidence](ohos_foundation_values_validation.md). |
| `dialog.h` | 0 | Audit open; retain full public surface. |
| `accessibility_manager.h` | 2 | Live public SDK state query implemented; UI/worker C++ and C ABI pass 12 checks twice on each guest, with independent SDK/list and system-service evidence in the disabled state. Enable is a no-op: activation requires a system API and WRITE_ACCESSIBILITY_CONFIG. Enabled-state/transitions, SDK failure and teardown remain open; see accessibility validation. |
| `display.h` | 11 | Held primary C++/C handles and live getters match independent SDK snapshots over eight phone rotations and eight computer available-area transitions; see display validation. Secondary displays, native wrappers, reconnect/ID reuse, error/teardown and remaining overloads stay open. |
| `display_manager.h` | 3 | Live public Input Kit cursor query replaces the origin stub. API 26 phone/computer each pass 38 explicit no-pointer/focus-transition C++/C/worker checks. Public NDK display observers now deliver live configuration/work-area events: phone 437 and computer 444 native checks, with partial/final/worker unsubscription and real queued-notice cancellation. Enumeration errors preserve cached identities. Positive pointer coordinates, authorization refusal with a pointer, multi-display/hotplug/ID reuse and error/teardown remain open; see cursor and display validation. |
| `url_opener.h` | 3 | Public SDK implementation; C++/C ABI HTTP/HTTPS dispatch verified on phone and computer (44 native + 12 UI checks per guest). Worker/UI-thread and validation paths verified. SDK denial/background, undeclared-querySchemes and pending-launch teardown audits remain open; see URL validation. |
| `app_info.h` | 4 | Implemented getters; phone/computer, worker and C ABI verified. Context teardown reviewed, not a separate assertion. |
| `device_info.h` | 7 | All getters verified through ordinary C++/C UI/workers on phone/computer; live Settings names, fallback, process ABI, restarts and restored changes. Actual service failure/teardown/other ABI audits remain open. [Evidence](ohos_device_info_validation.md). |
| `preferences.h` | 11 | Same-scope shared SDK lifetime and missing-key Remove semantics fixed. Phone/computer each pass 215 C++/C UI/worker assertions over eight rounds with independent SDK bidirectional data, concurrency, 1024-byte/16 MiB limits and two real process restarts. Storage errors, external handles/processes, XML-only/older images and remaining audits stay open. [Evidence](ohos_preferences_validation.md). |
| `secure_storage.h` | 12 | ASSET storage implemented; all storage operations, limits, isolation, enumeration and process persistence verified on both guests. Locked-state denial not exercised. |
| `launch_at_login.h` | 13 | Public SDK self-status queries replace the fixed false: computer 175 assertions over seven rounds/four actual Settings transitions, phone 75 assertions over three rounds with independent SDK capability false. C++/C, UI completed snapshots, worker fresh queries, constructors/metadata/foreign IDs, local display name and honest mutator refusal are covered; see startup validation. Programmatic controls have no normal self-service SDK; UI reads refresh asynchronously. Actual login/boot, older images, error/teardown and binding runtime audits remain open. |
| `message_dialog.h` | 21 | Native ArkUI implementation; phone/computer buttons, controls, live updates, worker/window/application modality, overlapping leases, parent/destruction cleanup and C ABI verified. IME/theme/long-content/error/teardown audit remains open. |
| `file_dialog.h` | 12 | Native SDK implementation in core `e07fe6c`. Computer: 52 native + 8 guarded UI checks passed. Phone save/single-open/URI bytes verified; separate native/C ABI cancellation scenario passed 10 native + 3 UI checks. Multiple selection and folder acceptance remain open. Direct ArkTS SDK comparison reproduces one-file return from the tested API 26 phone Browse path. Release API 24 comparison also returns one file (33 checks passed, 1 failed); save/single-open/real bytes pass on both versions. Folder acceptance remains unverified on both. See picker validation; no unsupported classification inferred. |
| `notification_manager.h` | 6 | Public SDK authorization/publish/cancel and WantAgent activation implemented. Phone/computer: 21 normal + 25 cold-launch/refusal checks each. Real body/button clicks launch fresh processes and queued onCreate events reach C++/C ABI; Shutdown retention is checked in system records. Teardown/concurrency and boundary/rate-limit audit remains open; see notification validation. |
| `image.h` | 6 | Native codecs and copy/move ownership implemented; PNG/JPEG/WebP, pixels/files/C ABI verified on both guests. Codec edge-case audit remains open. |
| `window_shape.h` | 5 | Builder and polygon-mask checkpoint below; wider binding/teardown audits remain open. Shared value/ABI constructors, copies, bounds/invalid values and stale/cross-type handles now covered in both guests; this adds no native shadow-rendering claim. [Evidence](ohos_foundation_values_validation.md). |
| `window_shadow.h` | 7 | Shared value/ABI constructors, copies, bounds/invalid values and stale/cross-type handles now covered in both guests; this adds no native shadow-rendering claim. [Evidence](ohos_foundation_values_validation.md).  Native custom rendering is now implemented through an optional Stage companion, including clipped shapes, copied configuration, live geometry/visibility and click-through helpers; [native shadow checkpoint](ohos_custom_shadow_validation.md). Computer primary shadows and explicit helper reconstruction now have [primary/recovery evidence](ohos_primary_shadow_validation.md). Primary rounded contours, boundaries, errors and teardown remain open. |
| `view.h` | 76 | Native ArkUI containers/Label/Button/TextField/ImageView implemented via host ContentSlot. Phone/computer native tree, geometry, modes, ownership and real C++/C ABI clicks verified; see view validation. Real hover tooltips now pass 8 native checks and 24 actual scenes on each guest through the optional companion host and public SDK popup, with exact text, focus/click preservation and cancellation checks; [tooltip evidence](ohos_tooltip_validation.md). User-edit/submit/IME, theme, secondary windows and teardown audit remain open. |
| `window.h` | 112 | Native SDK occlusion queries/events implemented: actual no/partial/full/transparent coverage, hide/restore, UI/worker C++/C ABI, object/manager/C event agreement and listener removal/re-add covered in the occlusion checkpoint. Actual SDK system title-button closes now share cancellable/deferred C++/C votes: computer 18 system clicks and 190 pre-termination assertions, including original SDK main-window termination; phone 175 assertions over eight programmatic child scenes with no native caption buttons exposed. See native-close validation. Public SDK reparenting and live parent getters now pass 420 C++/C checks on each guest: computer performs actual changes and old/active parent destruction, while the current phone SDK refuses mutations with 801 and preserves real relationships. [Parent evidence](ohos_window_parent_validation.md). Full remaining property/action/type/error/lifecycle audit stays open; retain the entire public surface.  Custom shadows now use core-generated RGBA and private SDK helpers; [native shadow checkpoint](ohos_custom_shadow_validation.md) covers C++/C, UI/workers, actual pixels/input, mask clipping, resize and native closure on both guests. Wider audits stay open. |
| `window_manager.h` | 12 | Actual SDK per-display stack queries replace focus-only hit order: worker refresh and nonblocking completed UI snapshots. Phone/computer each pass 1,188 assertions in nine SDK-compared scenes, with z-levels/raise, exclusions, identity, enumeration, ownership and destroyed-ID lookup. Foreign coverage, multi-display coordinates, errors/teardown, legacy swizzle methods and remaining audits stay open. [Evidence](ohos_window_manager_validation.md). |
| `window_drag_session.h` | 6 | Audit open; retain full public surface. |
| `drag_source.h` | 14 | Public native ArkUI programmatic sources implemented for ArkTS hosts without a mounted native content root. C++/C text/mixed/file-only bytes, preview pixels/snapshots, rejection/cancel/conflicts and main/worker destruction are covered by source validation. IsPositionSupported and IsOperationSelectionSupported expose absent SDK coordinates/source-offer masks; actual Copy/Move/None feedback is preserved. Explicit Move/Link preferences, cross-app grants, folders, pointer layouts and full environment/worker/lifecycle audits remain open. |
| `drop_target.h` | 6 | Public native ArkUI Copy destinations implemented on mounted NativeAPI content roots. API 26 phone/computer each pass 13 real SDK-source touch scenes covering C++/C text, mixed and file-only payloads, actual bytes, logical coordinates, exit, refusal, removal and callback destruction; see drop validation. Cross-app grants, folders, arbitrary ArkTS/Flutter content, child precedence and error/worker/teardown audits remain open. |
| `positioning_strategy.h` | 9 | Shared-window ownership repaired; new owning getter generated for all six bindings. Both guests verify UI/worker value factories and live SDK child move/resize, content coordinates, explicit closure and C-handle retention/release. CursorPosition is a value test only; actual positioning/rendering remains in consumer audits. [Evidence](ohos_foundation_values_validation.md). |
| `menu.h` | 40 | Native ArkUI menu/item collections, properties, nested presentation, lifecycle, checked/radio states and C ABI implemented through canonical host adapter. Phone/computer: 40 native + 4 guarded UI checks each; see menu validation. Cursor, actual keyboard/tooltip, full placement/type/error/teardown audits remain open. |
| `tray_icon.h` | 28 | Public HMS status-bar backend: desktop 40 native assertions plus real icon pixels/clicks/menus, replacement/template/tooltip/coalescing, C ABI and worker calls, hide/show and pending-update destruction verified. Phone 19 assertions and independent SDK capability false. SDK geometry/right/double-click/programmatic-menu limits documented; full boundary/menu/background/teardown audits remain open. See tray validation. |
| `tray_manager.h` | 3 | Capability and shared identity registry/Get/GetAll implemented and verified with C++/C ABI and workers on both guests. Hidden identities included; one visible SDK item per app. Full lifecycle/ownership/race audit remains open. See tray validation. |
| `shortcut.h` | 11 | Public NDK global registration checkpoint verifies metadata, stable C++/C aliases, enabled state, scope filtering and retained callback invalidation after unregister on both guests. Real input now verifies foreground/background activation, all four modifier-side pairings, disabled manager/shortcut, self-removal and no delivery after unregister: each guest has nine real chords, six business/public activations and nine direct SDK callbacks. Application-local C-registered actual callback and user-data release are verified in the local shortcut checkpoint; the global C-registered actual callback and remaining callback/lifecycle audits stay open. |
| `shortcut_manager.h` | 15 | Public API 14+ NDK global hotkeys replace the stub. Phone/computer each pass 67 registration assertions, including independent SDK-app occupancy, all left/right Ctrl/Alt variants, alias/order rejection, C++/C/worker lifecycle and release/reuse. Shared removal refusal preserves retryable identities and pre-dispatcher failure callbacks can re-enter safely. Actual global input now passes sixty native assertions on each guest, including SDK-positive negative business cases; see shortcut input validation. Application-local scope now uses the canonical component host without global reservation: both guests pass sixteen actual chords and 102 assertions, including C callback/user-data lifetime, worker registration, peer background input and an independently occupied global chord. Additional/native-only hosts, layouts/races/teardown remain open; see local shortcut validation. |
| `keyboard_monitor.h` | 4 | Public NDK press/release backend implemented. Phone/computer system-app debug fixtures each pass 356 native assertions over 14 real chords, including C++/C, background focus, multiple monitors, worker lifecycle, queued-epoch invalidation and callback destruction. Ordinary apps are denied; normal ACL apps have self-permission but SDK monitor registration still returns 201 on both guests. See keyboard monitor validation. Modifier-state events, other keys/cancellation/repeat, live permissions/service failures and environment/binding runtime audits remain open. |
| `application.h` | 17 | Owning-Ability lifecycle/state, shared cancellable/deferred C++/C ABI quit and real SDK termination, live primary-window queries, actual application multi-instance metadata and Light/Dark/System resources implemented. Phone: 49 + 25 native assertions, 7 guarded actions and actual background/foreground; computer: 45 + 25, 7 actions. Later integration checkpoint: desktop 62 native assertions plus five real main/child hide/restore cycles, actual mission-icon pixels and numeric Dock badges; phone 23 native assertions, actual launcher badges and direct SDK unsupported-operation comparison. Application/Window visibility reconciles SDK hiding and external restore. Multiple Abilities, failure/teardown, metadata/theme/ownership variants, inactive restoration and remaining dock/progress/menu-bar integrations remain open; see application and integration validation. |

## Runtime evidence already available

- Device metadata, display queries, preferences, text/HTML clipboard and window
  integration: prior phone run 96 checks, prior desktop run 108 checks plus three
  guest interaction checks. See the linked validation reports. These checks do not
  prove every method in those modules is finished.
- First implementation checkpoint: AppInfo, Image, ASSET SecureStorage and all four
  local clipboard formats passed on macbook-pro ARM64 phone and computer guests.
  Phone: 143 checks twice, zero failures. Computer: 156 checks then 155 checks, zero
  failures (clipboard callbacks can coalesce); three guest interaction checks passed.
  See [service checkpoint](ohos_services_validation.md). Cross-app clipboard file
  reception, unsupported/permission boundaries and remaining modules stay open.

- MessageDialog checkpoint: native ArkUI controls and real guest input now exercise
  the existing API on phone and computer. Modal calls require a worker; application
  modality covers attached nativeapi windows. See [dialog validation](ohos_dialog_validation.md)
  for exact results and the unresolved theme/IME/content/teardown cases.

- FileDialog investigation: [picker validation](ohos_picker_validation.md) records
  the complete computer run, positive phone save/single-open checks and failing
  multi-file interaction. Independent direct SDK reference paths preserve the
  remaining phone questions instead of weakening the two-original-file assertion.

- URL dispatch checkpoint: [URL validation](ohos_url_validation.md) records real
  browser address/ownership checks for four fresh C++/C ABI launches on each guest.
  Capability queries require the host's querySchemes declaration; synchronous Open
  requires a worker. SDK denial/background and teardown scenarios remain open.

- Notification checkpoint: [notification validation](ohos_notification_validation.md)
  records system authorization, real active-record replacement/removal and actual
  notification body/button events on both guests. Two cold launches per guest,
  Shutdown retention and actual authorization refusal now pass separate assertions.
  Teardown/concurrency and broader boundary cases remain open.

Native view checkpoint: computer 32 native + 3 guarded UI checks; phone 30 + 3.
Both use actual ArkUI trees, native geometry and C++/C ABI clicks. The computer
also checks root/child relayout after native window resize and restoration.
See [view validation](ohos_view_validation.md) for exact process IDs, evidence and
remaining cases. The final computer regression still passes all 269 checks.

Native menu checkpoint: each guest passes 40 native + 4 guarded UI checks,
including actual native checked/disabled states, absolute/relative geometry,
dynamic updates, submenu lifecycle/closure, pending cancellation and C ABI.
See [menu validation](ohos_menu_validation.md) for final process IDs and open cases.
The final computer regression again passes all 269 checks.

Accessibility state checkpoint: [validation](ohos_accessibility_validation.md) records
12 native checks repeated twice on each guest and the actual system service state.
The old local success flag is removed; positive state transitions remain open.

Application checkpoint: [validation](ohos_application_validation.md) records actual
Ability events/termination, C++/C ABI votes, rendered theme pixels and final guest PIDs.
The current computer regression again passes all 269 checks.

## Remaining audit and implementation groups

- Application multi-Ability/environment policy, SDK termination failure/reentry/teardown,
  appearance/metadata/window-ownership variants and remaining application integrations; URL dispatch
  SDK denial/background, undeclared-querySchemes and teardown audit.
- Display hotplug/multi-display/ID reuse, SDK fault injection, callback/teardown races and positive pointer cursor queries.
- Clipboard implementation defects and directory/permission/concurrent-read/environment-cleanup acceptance are closed by the boundary checkpoint. Further coverage remains for interactive system File Manager UI, privileged URI-grant revocation, peer uninstall and codec/service fault injection; these are not passing claims.
- Native encrypted storage locked-state denial, fault and teardown boundaries; enumeration, value/alias limits, isolation and process persistence already have guest evidence.
- Menu cursor/keyboard/tooltip, complete placement/property/type/error/teardown matrix; notification teardown/concurrency,
  boundary and rate-limit verification.
- Message-dialog IME/theme/long-content/error/teardown audit; native file dialogs,
  selection/cancel/close and their modal semantics.
- Native view tooltip placement/theme/SDK failure and multi-window audit, user-edit/submit/IME and read-only denial, native focus/selection transitions, secondary windows, theme/font scaling, borrowed-node interop and environment teardown.
- Every remaining window property/action, native dragging/resizing, shape/shadow,
  including main-window custom shadows, screen boundaries, failure rollback and
  broader shadow/default policy. Child custom rendering now has the checkpoint
  below, and the default child shadow toggle uses public setWindowShadowRadius.
  Input policies, desktop capability differences and main-window lifecycle also
  remain under audit.
- Drag source/drop target/session, global keyboard monitoring/shortcuts, accessibility,
  launch-at-login and tray: check current public SDK and normal-app permissions before
  classifying any operation as unsupported. Accessibility activation has a documented
  system-API restriction; enabled-state, failure and teardown evidence remain open.
- Pure value/geometry/color/keyboard/placement/positioning APIs and generated binding
  paths, including explicit negative paths for unsupported platform operations.

For every group, add concrete source references, commands, observed runtime results,
and unresolved cases here before closing the goal. The current goal remains active.

Related evidence: [native menus](ohos_menu_validation.md), [phone/window integration](ohos_window_validation.md),
[computer window acceptance](ohos_desktop_window_validation.md).

Tray checkpoint: [validation](ohos_tray_validation.md) records desktop PID 8024
(40 native assertions, six actual icon captures, native menus and system removal)
and phone PID 3700 (19 assertions), plus independent direct SDK comparison.
The final computer regression passes 268 checks (one clipboard callback coalesced
relative to the previous 269-check run; all unique labels remain). The full API
goal and all listed open audits remain active.

Application integration checkpoint: [validation](ohos_app_integration_validation.md)
records computer PID 21197 (62 native assertions, two rendered mission icons,
9/99+/clear system badges and five actual main/child hide/restore cycles) and
phone PID 3638 (23 assertions, actual launcher badges and direct SDK error 801
for desktop-only operations). Application regression passes computer 44 + 24
and phone 49 + 25 assertions, including real termination and appearance.
No full-module or full-goal completion is inferred from this checkpoint.
Computer general regression after the integration checkpoint passes 269 checks
(PID 6066), including actual drag/maximize/restore; tray regression passes 40
computer and 19 phone assertions. Other listed audits remain open.

Window occlusion checkpoint: [validation](ohos_occlusion_validation.md) records phone PID 3657 and computer PID 3398, 104 native assertions
each, twelve rendered scenes each and actual SDK/compositor pixel/window evidence. Other Window APIs and all
parameter, lifecycle and binding audits remain in scope.
This checkpoint also passes computer application integration (62 assertions/five
real group cycles), phone application lifecycle/theme/termination (49 + 25),
phone launcher badge regression from a Settings back stack (25), and computer
269-check general/window regression. The full API goal remains active.

Window style checkpoint: [validation](ohos_window_style_validation.md) records
child corner preferences and real native shadows, SDK/reference pixels, normal
manifest permission and unsupported/main/invalid/dead-window paths. Full window
shape, input, custom-shadow and other audits remain open.
Computer style passes 60 assertions and nineteen native scenes (including normal-
permission main shadows); phone passes 50 assertions and fourteen child scenes.
Direct SDK main shadow results distinguish missing desktop permission (201), normal
manifest permission success (0) and phone capability refusal (801). Shared-request
regression also passes computer integration 62, phone integration 23, phone occlusion
104 and computer general/window 269. The full goal remains active.

Window polygon-mask checkpoint: [validation](ohos_window_shape_validation.md) records
real compositor masks, copied even-odd polygons, rectangle restoration, shared C++/C
state and guarded touch pass-through. Computer PID 32207 passes 56 native assertions,
13 scenes, 702 sampled pixels and 26 touches; phone PID 10887 passes the same
56 assertions/13 scenes/702 pixels/26 touches. SDK resize/property lag is explicitly
tested as refusal followed by a successful retry after the new rectangle is visible.
Independent input polygons, legacy runtime and other listed audits remain open.
Existing corner/shadow regressions pass phone 50 and computer 60 assertions;
computer general/window regression PID 6560 passes 269 checks, including actual
drag/maximize/restore. Local CTest passes 31/31 and all six binding checks pass.
The full API goal remains active.

Native dragging checkpoint: [validation](ohos_window_drag_validation.md) records
real C++/C startMoving integration, native move events and direct SDK comparison.
Computer touch PID 17370 and phone PID 18132 each pass 41 native assertions and
eight real gesture/control scenes. The phone full-screen main window is rejected
with direct SDK 1300004; its native child moves. Independent input-area requests
return 201 in main/child windows on both ordinary-app guests; official restricted
permission and ten-rectangle limits are recorded. Restricted-ACL scenarios and
custom-title maximize policy remain open. The full API goal remains active.
Final computer PID 20756 additionally passes 45 native assertions with eight touch
drags, eight native mouse drags and eight click-only checks; positive scenes require
real move events and preserve window dimensions.
Computer general/window regression PID 23904 passes 269 checks; local CTest is
31/31 and six local binding checks pass. Full API and restricted-ACL audits remain open.

Window control-button checkpoint: [validation](ohos_window_controls_validation.md)
records computer PID 28140 (61 native assertions) and phone PID 3797 (59), with
eighteen real UI scenes each. Computer native button nodes, live C++/C getters,
exact object/manager/C event counts, SDK changes, hidden-title overrides and
programmatic maximize/recovery with hidden buttons are verified. Phone SDK
returns 801 for main controls and 1300004 for ordinary children, with no false
visibility or property-change reports. The independent fifteen-scene SDK probe
does not establish SetMaximizable policy support; capability policies, older SDK
event behavior, system hover transitions and other listed audits remain open.
The full API goal remains active.
Computer general/window regression PID 6351 passes 268 checks, including actual
drag/maximize/restore. Its unique check labels match the preceding run; only one
clipboard callback was coalesced. Local CTest passes 31/31 and all six local
binding checks pass. Reboot-related picker filename collisions were reproduced;
the subsequent aspect checkpoint fixes PID-only naming. The fresh-process
general run passed without manual interaction.

Content aspect checkpoint: [validation](ohos_window_aspect_validation.md)
records computer PID 23526 (52 native assertions, sixteen scenes and fifteen
real mouse edge resizes) and phone PID 3783 (51 assertions/sixteen scenes).
The backend uses nonpersistent API 21+ content constraints without immediate
resizing, serializes SDK promises, preserves successful values on rejection,
and releases queued requests on destruction. C++/borrowed/C aliases, worker/UI
calls, direct SDK changes, hidden-title content ratios, clearing and SDK size
limits are verified. Phone accepts configuration while retaining full-screen
geometry; independent recover/floating-mode requests return 801 (PID 3909).
Older SDKs, actual phone floating forms, persistence and remaining race/teardown
audits stay open. Computer controls regression passes 61 assertions/eighteen
scenes, and final general/window regression PID 4109 passes 269 checks. Local
CTest passes 31/31 and all six binding checks pass. Picker names are now unique
per run, with exact-name parsing and full computer picker acceptance; phone
selection with the longer names remains a follow-up. The full goal remains active.

Picker comparison checkpoint (2026-10-16): official release API 24 phone instance
installed alongside API 26, with real native library loading and positive save/URI/byte
checks. Multi-file and folder acceptance remain open on both phone images. Final
normal API 26 computer process 3144 again passes all 269 integration/picker/dialog/
window UI checks. See [picker validation](ohos_picker_validation.md).

Cursor checkpoint: [validation](ohos_cursor_validation.md) records public Input Kit
query integration and 38 explicit no-pointer checks on each API 26 guest. These
results do not prove positive cursor coordinates or the authorization-denial
branch with actual pointer hardware. Display hotplug/configuration events and
the complete Display/DisplayManager audit stay in scope.
Final computer regression for this cursor checkpoint: process 3085 passed all
269 integration/picker/dialog/window UI checks, zero failures.

Display observer checkpoint: [validation](ohos_display_validation.md) records phone
PID 3476 (437 native checks, eight real rotations) and computer PID 16505
(444 checks, eight real available-area transitions). Both require independent SDK
callback deltas, partial/all/worker unsubscription and positive pending counts
before cancelling real queued notifications. No actual hotplug/multi-display or
full-module completion is inferred. Local shared enumeration regression passes
and final computer regression passes all 269 checks (PID 3196), including actual
window dragging/maximize/restore. The whole HarmonyOS API goal remains active.

Clipboard interoperability checkpoint: [validation](ohos_clipboard_interop_validation.md)
records computer PID 19276 / peer 19488 and phone PID 3936 / peer 4064. Each
guest passes 34 native assertions plus 17 validated cross-app commands, with
distinct UIDs, exact text/HTML/RGBA and two real producer-file byte streams in
both directions. Same logical sandbox paths contain different contents; actual
SDK-authorized paths and inode separation prevent consumer-file confusion.
Deleted/read-denied files are rejected. Directory/grant/revocation/concurrency/
teardown cases remain open; the full HarmonyOS API goal remains active.
Final computer regression after the clipboard change passes 269 checks
(PID 3446), including actual dragging/maximize/restore.

Global shortcut registration checkpoint: [validation](ohos_shortcut_validation.md)
records desktop PID 4575 / peer 4797 and phone PID 15057 / peer 15207. Each
guest passes 67 native assertions and thirteen commands, including independent
SDK-app occupancy errors, native availability probes, four modifier-side variants,
C++/C/worker registration and released-resource reuse. This is registration
evidence; actual key-trigger delivery subsequently passed under the explicit
keyboard-input exception granted in this thread, as recorded in the input report.
Application-local scope and full keyboard monitoring stay in the full goal.
Final computer regression after this checkpoint passes all 269 checks (PID 9231).

System title-button close checkpoint: [native close validation](ohos_native_close_validation.md) records shared native/API requests, C++/C worker votes, last-observer removal, external destruction, stale decisions and SDK-owned main-window termination. The phone profile has no system caption controls, so native button interception is not claimed there.

Real global shortcut input checkpoint: [actual input validation](ohos_shortcut_input_validation.md) records the explicitly approved nine Ctrl+Alt+F9 chords on each named guest, with actual background focus, disabled-state SDK reference delivery and callback self-unregistration. Application-local host-based input subsequently passes the local shortcut checkpoint below. Full keyboard monitoring and remaining shortcut audits stay open.

Application-local shortcut checkpoint: [local validation](ohos_local_shortcut_validation.md)
records computer PID 7204 / peer 7773 and phone PID 3731 / peer 4204. Each passes
sixteen real Ctrl+Alt+F9 chords and 102 native assertions, including normal peer
background input, independently occupied global chords, disabled-state positive
component delivery, host detach/remount, self-removal, C callback/user-data release,
worker registration and event-listener removal before business delivery. The
canonical component host is required. Existing global registration passes 67
assertions on each guest; real global input regression passes sixty assertions
and nine chords each (computer PID 7859, phone PID 4287). Local CTest is 33/33,
six host-language binding checks and codegen check pass. Additional/native-only
hosts, repeat/focus timing, environment/concurrency audits, global C input and
full KeyboardMonitor remain open. The full HarmonyOS objective stays active.

The final computer regression for the local shortcut checkpoint passes 269 checks
(PID 3436), including real window dragging/maximize/restore. The computer guest
remains running. Full API and all remaining audits remain active.

Startup status checkpoint: [startup validation](ohos_startup_validation.md) records
computer PID 20109 (175 assertions/seven rounds/four real system Settings changes)
and phone PID 3582 (75 assertions/three rounds, public SDK device capability false).
The task's original disabled setting is restored on the computer. Programmatic
self-registration remains unavailable through the normal public SDK; fresh status
queries require a worker, with nonblocking last-completed UI snapshots. Actual
boot/login, SDK errors/teardown and remaining audits stay open. The global input
monitor permission refusal is observed directly; its subsequent native backend
and three-profile verification are recorded in the keyboard checkpoint below. The full HarmonyOS API objective remains active.

Final startup-checkpoint computer regression PID 16664 passes 268 checks. Unique
native assertions match the preceding 201-check baseline; only one duplicate
clipboard notification coalesces. The picker verifier now reads its own live UI
root when Settings retains focus, and window gestures activate the same test
process first. A source-export count fix is verified by 27 real computer drags
and 211 native assertions (PID 20025). All remaining audits and the full goal
stay active.

Keyboard monitoring checkpoint: [native keyboard monitor validation](ohos_keyboard_monitor_validation.md)
records 356 native assertions and 14 actual authorized chords on each system-app
debug guest (computer PID 21206, phone PID 8987). Ordinary and normal-ACL profiles
each pass four real-input refusal checks on both guests. A normal ACL grants the
permission but does not make the monitor service accept the application on these
images; only the tested system-app/debug configuration delivers global events.
The backend preserves native key-down/up codes, safely shares the one process
slot, and invalidates stopped/destroyed/old-epoch delivery. Modifier-state events
and remaining audits stay open. Computer regression PID 12396 passes 268 checks,
including actual window drag/maximize/restore. The full API goal remains active.

Device information checkpoint: [validation](ohos_device_info_validation.md) records
computer readers 14914/15081 and phone readers 3482/3889. Each guest passes
1,062 native assertions, 78 isolated actual-backend fault checks and 120 worker
samples across six live phases. Normal readers have no MANAGE_SETTINGS; separate
SDK writer UIDs change two Unicode names and restore the persisted original after
process restart. Public SDK snapshots, uname release and actual loaded ELF64
machine 183 agree with all seven getters. Controlled SDK exceptions/module/type
failures clear without residual JS exceptions. CTest is 33/33 and all six host
binding checks plus codegen check pass. Actual service failures, pending-worker
teardown, other ABIs/older images and remaining module audits stay open. This
checkpoint sends no keyboard input and makes no new window/picker regression
claim. The full HarmonyOS API goal remains active.

Preferences checkpoint: [validation](ohos_preferences_validation.md) records desktop
PIDs 16173/16919/17201 and phone PIDs 3394/3486/3766. Each normal-app guest passes
215 native assertions over eight rounds, with exact SDK map equality, SDK-to-native
publication, eight-thread concurrent writes/removals, unanchored final-close/reopen,
positive 1024-byte/16 MiB bounds and persistent data/clear across two actual
process restarts. Missing deletion now returns false and same-scope instances
share one synchronized SDK handle until final close. File naming/data locations
and SDK engine selection are preserved. All six host binding checks, CTest 33/33
and codegen check pass. Storage failures, external SDK/core instances, cross-process
and cross-app cases, XML-only/older images and remaining audits stay open. The
full HarmonyOS API objective remains active.

Window manager query checkpoint: [validation](ohos_window_manager_validation.md)
records desktop PID 15551 and phone PID 3649, each passing 1,188 native assertions
over nine actual SDK-compared scenes and 72 worker samples. Child layers and
SDK raise operations are independent of focus; initial UI reads refresh after
real SDK completion. C++/C lookup/enumeration/current/point/exclusions and retained
closed IDs pass. Per-display cache implementation is compiled on both guests;
actual multi-display/foreign/mask/error/teardown audits and legacy hook design
remain open. A test-export count regression from the preceding Preferences
checkpoint is repaired: both final HAPs again pass complete DeviceInfo
(1,062 native plus 78 controlled faults) and Preferences (215) suites. CTest
33/33, all six host binding checks and codegen check pass. The full HarmonyOS
API objective remains active.

- Foundation checkpoint (2026-10-16): no-throw colors, complete modifier bitmasks and safe shared-window positioning; UI/worker value contracts and actual SDK child geometry/closure verified on both normal-app guests. See [foundation validation](ohos_foundation_values_validation.md). Native rendering/input and remaining module audits stay open.

- Custom shadow checkpoint (2026-10-16): the previous false/null backend is replaced by copied configurations, core-rendered clipped contours and optional noninteractive Stage helpers. Phone caption-query refusal and retained old drawable sizes are handled with verified mode records and SDK geometry generations. See [custom shadow validation](ohos_custom_shadow_validation.md); this does not close the remaining window/main/embedding/error audits.

- Primary-shadow/recovery checkpoint (2026-10-16): actual computer main-window colors, alpha, decorations and defaults are verified; the Stage companion now recreates helpers after real SDK destruction. The full child suite covers the same recovery on phone and computer. See [primary-shadow validation](ohos_primary_shadow_validation.md); remaining modules and lifecycle audits stay active.

- Independent caption-policy checkpoint (2026-10-16): SetMinimizable,
  SetMaximizable and SetClosable now control their SDK caption buttons independently,
  retain choices across collective visibility/title styles and publish matching
  object/manager/C events. [Caption-policy validation](ohos_window_policy_validation.md)
  records computer 131 native checks/32 scenes and phone 124 checks/32 refusal
  scenes; collective regressions add computer 61 and phone 59 checks over eighteen
  scenes each. Real computer title double-click still maximizes with its button
  hidden: these are explicitly caption-only policies. Full user-action denial,
  full-screenability and the remaining window/API audits stay open.

- Custom title-action checkpoint (2026-10-16): PerformTitleBarDoubleClick now
  uses public SDK maximize/recover, honors applied policies and invalid modes,
  shares a pending guard across aliases and invalidates requests on native cleanup.
  Live free-window capability prevents phone SDK no-op success from being
  reported as an accepted action. Eligible child actions are independent of
  unavailable caption-button controls.
  [Title-action validation](ohos_title_action_validation.md) records real component
  gestures (20 per guest), 318 checks per guest plus computer 131/phone 124
  caption regressions, C++/C UI/workers, SDK/WMS geometry, resize events, ordinary/closed/
  invalid wrappers and native-menu split recovery. The native system title gesture
  remains unchanged; remaining Window and full API audits stay open.

- Freeform child fullscreen checkpoint (2026-10-16): eligible children now use
  actual immersive SDK maximize/recover and native-mode queries, including
  restoration to their previously maximized or original floating state.
  [Fullscreen validation](ohos_child_fullscreen_validation.md) records 517 native
  checks and 31 scenes per guest, plus 318 title-action regression checks and
  20 real custom double-clicks per guest. It covers C++/C
  UI/workers, repeated requests, real SDK mutations/restrictions, actual WMS
  geometry, confirmed object/manager/C events and destroyed/invalid wrappers.
  Independent phone SDK evidence distinguishes content-layout fullscreen flags
  from an enlarged native frame. The public global mouse monitor rejects the
  ordinary fixture with 201; this denial does not establish recording/privileged
  session behavior. SetFullScreenable, conflicting request serialization and the
  remaining API audits stay open.

- Fullscreen-mode policy checkpoint (2026-10-16): SetFullScreenable now applies
  real SDK supported modes, preserving known floating/split bits; primary
  WindowStage fallback and explicit host override publication cover the
  SDK's lack of a live mode-mask getter. Requests serialize across aliases,
  workers await their own result, and actual destruction cancels pending work.
  [Mode-policy validation](ohos_fullscreen_policy_validation.md) records 716
  checks/43 scenes per guest, 517 fullscreen and 318 title regression checks
  per guest, plus 28 independent SDK reference scenes per guest. A real held
  SDK outcome and four pending requests prove cancellation and late-result
  isolation on both profiles. Phone mode refusal remains 801; unknown child
  policy returns false. The SDK couples fullscreen and maximize restrictions.
  In-flight environment teardown, unreported external overrides, older images
  and remaining window/API audits stay open; the full objective remains active.

- Immersive content-layout checkpoint (2026-10-16): SetContentUnderTitleBar now
  waits for real SDK completion and live flag agreement on workers, refuses
  overlapping C++/C aliases, and cancels pending requests on native destruction.
  [Layout validation](ohos_window_layout_validation.md) records 947 checks/46
  scenes on each normal-app guest, independent SDK reference calls, actual WMS
  and native content geometry, phone ArkUI safe-area expansion/restoration,
  controlled boundary faults and late real SDK outcome isolation. Computer
  floating-main unchanged flags and decorated-child insets remain explicit;
  this is not a transparent native-caption overlay. Both guests rerun 517-check
  fullscreen and 716-check mode-policy regressions. Other modes, older images,
  real environment teardown and the wider API audit stay open.
