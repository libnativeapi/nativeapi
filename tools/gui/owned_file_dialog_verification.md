# Windows owned file dialog regression (#29)

Verified on 2026-11-14 on `windows-laptop`: Windows 11 x64 10.0.26200.9974,
Visual Studio 2022 Release build, logged-on desktop session 1.

## Cause and fix

The WinEvent observer wraps a dialog HWND on the owner's platform thread while
the dialog belongs to another STA. Previously, `TrackWindowLifetime` treated a
failed cross-thread subclass installation as successful first-time tracking.
The constructor then read a complete property snapshot: four getters each waited
up to five seconds for posted work on the dialog thread. That thread can itself
be waiting for the owner's focus initialization to finish.

Tracking and its initial snapshot now run asynchronously on the HWND's own
thread. The queued task checks the HWND, thread and WindowId before constructing
a temporary wrapper; it does not retain the original wrapper. Same-thread setup
remains immediate, and only a successful first subclass installation seeds the
snapshot. Existing synchronous public getter behavior is unchanged.

This follows the Win32 constraints that
[out-of-context WinEvent callbacks run on the installing thread](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwineventhook)
and [subclass helpers cannot subclass across threads](https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass).

## Results

The deterministic reproduction creates a window on a worker that processes sent
messages but withholds posted work. It wraps that HWND from the main thread,
then resumes the worker to verify initialization, property delivery and cleanup.

| Check | Before (`f77602a`) | Fixed |
| --- | --- | --- |
| Cross-thread HWND wrapping | 20,081.1 ms; fails the 1-second limit | 0.2275 ms |
| Native title change after worker resumes | Not delivered | Exactly once |
| Registry entry after native destruction | Retained | Removed |
| Initial snapshot | No change event | No change event |

The same executable also creates real COM dialogs on separate STA threads while
the owner thread pumps its WinEvent observer. Readiness is measured from `Show`
to a timer callback on the dialog STA finding a visible dialog. Cancellation
uses the dialog's `IDCANCEL` command, with no mouse or keyboard injection.

| Real dialog | Ready in | Result |
| --- | --- | --- |
| Owned `IFileSaveDialog`, first | 728.398 ms | Pass |
| Owned `IFileSaveDialog`, repeat | 516.319 ms | Pass |
| Unowned `IFileSaveDialog` | 417.689 ms | Pass |
| Owned `IFileOpenDialog` | 552.476 ms | Pass |

All four met the 3-second limit, preserved the requested owner, kept the owner
disabled only for the owned modal cases, returned `ERROR_CANCELLED` without
opening or saving a selected file, restored the owner's enabled state, and
produced a WindowCreated event.

- `window_cross_thread_dialog_windows_test`: all 31 checks passed.
- `window_property_changed_test`: passed.
- `window_manager_ui_thread_windows_test`: passed, including callbacks reaching
  the UI isolate thread when manager observation was first requested elsewhere.
- `window_controls_test`: passed, including cross-thread caption getters and
  setters, independent button policies, aliases and fullscreen transitions.

This is native integration coverage of the reported HWND / STA / WinEvent path.
It does not claim to run the reporter's Flutter AOT application or Windows 10.
No package release is part of this fix.

## Reproduce

Initialize `core` before syncing. The runner builds the four tests and schedules
them in the logged-on Windows desktop; running GUI tests directly in SSH session
0 is insufficient.

```bash
fastforge --no-version-check host exec windows-laptop -- powershell -NoProfile -ExecutionPolicy Bypass -File tools/gui/verify_owned_file_dialog_windows.ps1
```

On Windows itself:

```powershell
tools/gui/verify_owned_file_dialog_windows.ps1
# Deterministic stall reproduction only (also suitable for the old core):
tools/gui/verify_owned_file_dialog_windows.ps1 -ConstructionOnly
```

The runner uses `core/build-issue29`, prints each test's output and exit code,
and removes its scheduled task. `-SkipBuild` reruns existing binaries. Logs and
`result.json` remain under ignored `tools/gui/output/owned-dialog-<run-id>`.
The recorded fixed run is `8103125ecb354c0da3059c22eb92c70f`; the baseline run is
`d61371c551d844c2959a5f31318ef393`.
