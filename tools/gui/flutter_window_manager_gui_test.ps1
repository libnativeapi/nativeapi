# GUI test (Windows) of window_manager's widgets and close handling, on the
# fixture window_manager_flutter.dart built into the window_manager example
# (see flutter_window_manager_gui_test.py for the macOS twin):
#
# - WindowCaption: its title strip drags the window, its maximize button
#   maximizes and restores, its minimize button minimizes (the fixture restores
#   after 2 s), and with setPreventClose(true) its close button reports
#   onWindowClose and keeps the window. VirtualWindowFrame's top edge resizes.
# - With the native title bar, the window's own close button (DWM's caption
#   button bounds) is prevented the same way.
# - popUpWindowMenu() opens the system menu at the cursor.
# - setIgnoreMouseEvents(true, forward: true): the window drops out of hit
#   testing (WindowFromPoint passes it) while hovering still reaches it. Nothing
#   is pressed while it ignores the mouse.
# - destroy() closes the window past setPreventClose and the app exits.
#
# Owner-guarded clicks on the app's own window and menu only, no keyboard.
# Set WINDOW_MANAGER_EXE to the built example (flutter build windows --debug
# -t lib/gui_test_main.dart, with the fixture copied there).
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class WmWin {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr h, int a, out RECT r, int size);
  [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
  // DWMWA_CAPTION_BUTTON_BOUNDS, relative to the window: minimize, maximize, close.
  public static int[] CaptionButtons(IntPtr h) {
    RECT r; if (DwmGetWindowAttribute(h, 5, out r, Marshal.SizeOf(typeof(RECT))) != 0) return null;
    return new[] { r.L, r.T, r.R, r.B };
  }
}
"@

Start-Result "$RemoteScratch\flutter_window_manager_gui_test.result.txt"
$exe = $env:WINDOW_MANAGER_EXE
if (-not $exe) { throw 'Set WINDOW_MANAGER_EXE to the built fixture' }
$caption = 'WM GUI caption'
$button = 46

Assert-Idle
$app = Start-GuiApp $exe
$null = $app.Proc.Handle
function Get-ViewsQuietly {
  $saved = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try { @(Get-Views $app 2>$null) } catch { @() } finally { $ErrorActionPreference = $saved }
}
function Get-State {
  $script:win = Get-Win $app 'wm gui test'
  if (-not $script:win) { throw 'The fixture window is missing' }
  $script:view = Find-View (Get-ViewsQuietly) 'TARGET'
  if (-not $script:view) { throw 'The fixture has no Flutter view to probe' }
}
function Get-Value([string]$Name) {
  Get-State
  $t = @($view.texts | % { $_[0] } | ? { $_ -like "$Name *" }) | Select-Object -First 1
  if ($null -eq $t) { throw "The $Name line is missing" }
  $t.Substring($Name.Length + 1)
}
function Press([string]$Label, [double]$Settle = 1.2) {
  Get-State
  Invoke-Click $app (ConvertTo-Screen $win (Get-TextCenter $view $Label)) 400
  Pause $Settle
}
# 0: close, 1: maximize / restore, 2: minimize.
function Get-CaptionButton([int]$Index) {
  Get-State
  $c = Get-TextCenter $view $caption
  ConvertTo-Screen $win @(($view.size[0] - $button / 2 - $Index * $button), $c[1])
}

try {
  $deadline = (Get-Date).AddSeconds(40)
  while (-not (Find-View (Get-ViewsQuietly) 'TARGET') -and (Get-Date) -lt $deadline) { Pause 0.5 }
  Get-State
  Invoke-Activate $app $win ([int](20 * $win.Scale))
  Check 'the fixture starts with WindowCaption' (Test-ViewText $view $caption)

  # DragToMoveArea inside WindowCaption.
  Get-State
  $before = $win
  $start = ConvertTo-Screen $win (Get-TextCenter $view $caption)
  Invoke-Drag $app $start @(@(($start[0] + 40), ($start[1] + 20), 400), @(($start[0] + 160), ($start[1] + 100), 800))
  Pause 1.5
  Get-State
  Check 'dragging the caption moved the window' ([math]::Abs($win.Left - $before.Left - 160) -le 8 -and [math]::Abs($win.Top - $before.Top - 100) -le 8) "from $($before.Left),$($before.Top) to $($win.Left),$($win.Top)"

  # VirtualWindowFrame's top edge (DragToResizeArea).
  $before = $win
  $edge = ConvertTo-Screen $win @(($view.size[0] / 2 - 120), 3)
  Invoke-Drag $app $edge @(@($edge[0], ($edge[1] - 20), 400), @($edge[0], ($edge[1] - 80), 800))
  Pause 1.5
  Get-State
  # The drag legs are physical pixels: 80 up.
  Check 'the top edge resized the window' ([math]::Abs(($win.ClientH - $before.ClientH) - 80) -le 6) "client height $($before.ClientH) -> $($win.ClientH)"

  # Maximize and restore with the caption's own button.
  $before = $win
  Invoke-Click $app (Get-CaptionButton 1) 400
  Pause 1.8
  Get-State
  Check 'the caption maximize button maximized the window' ([WmWin]::IsZoomed([IntPtr]$win.Hwnd))
  Check 'and isMaximized says so' ((Get-Value 'maximized') -eq 'true') (Get-Value 'maximized')
  Invoke-Click $app (Get-CaptionButton 1) 400
  Pause 1.8
  Get-State
  Check 'pressing it again restored the window' (-not [WmWin]::IsZoomed([IntPtr]$win.Hwnd) -and [math]::Abs($win.ClientW - $before.ClientW) -le 4) "client width $($before.ClientW) -> $($win.ClientW)"
  $ev = Get-Value 'events'
  Check 'and the legacy events saw maximize, unmaximize' ($ev -like '*maximize*' -and $ev -like '*unmaximize*') $ev

  # Minimize; the fixture restores after 2 s.
  Invoke-Click $app (Get-CaptionButton 2) 400
  Pause 0.8
  $iconic = [WmWin]::IsIconic([IntPtr]$win.Hwnd)
  Pause 3
  $ev = Get-Value 'events'
  Check 'the caption minimize button minimized the window' $iconic
  Check 'and it came back, with minimize and restore reported' (-not [WmWin]::IsIconic([IntPtr]$win.Hwnd) -and $ev -like '*minimize*' -and $ev -like '*restore*') $ev
  Get-State
  Invoke-Activate $app $win ([int](20 * $win.Scale))

  # Prevented close through the caption's close button.
  Press 'Prevent close on'
  Invoke-Click $app (Get-CaptionButton 0) 400
  Pause 1.5
  Check 'with preventClose, the caption close button reports onWindowClose' ((Get-Value 'closeReports') -eq '1') (Get-Value 'closeReports')
  Check 'and the window stays' ([WmWin]::IsWindow([IntPtr]$win.Hwnd) -and -not $app.Proc.HasExited)

  # Prevented close through the window's own close button.
  Press 'Native title bar' 1.5
  Get-State
  $b = [WmWin]::CaptionButtons([IntPtr]$win.Hwnd)
  if (-not $b) { throw 'No caption button bounds' }
  $third = [int](($b[2] - $b[0]) / 3)
  Invoke-Click $app @(($win.Left + $b[2] - [int]($third / 2)), ($win.Top + [int](($b[1] + $b[3]) / 2))) 400
  Pause 1.5
  Check 'with preventClose, the native close button reports onWindowClose' ((Get-Value 'closeReports') -eq '2') (Get-Value 'closeReports')
  Check 'and the window stays' ([WmWin]::IsWindow([IntPtr]$win.Hwnd) -and -not $app.Proc.HasExited)
  Press 'Hidden title bar' 1.5

  # popUpWindowMenu at the cursor, which rests on the chip.
  Press 'Menu in 1.5s' 0.2
  $opened = Wait-Menu $app
  Check 'popUpWindowMenu() opened the system menu' $opened
  if ($opened) {
    $menu = @(Get-OpenMenus $app)[0]
    Check 'with the system menu items' ($menu.Items.Count -ge 5) "$($menu.Items.Count) items"
    # Close it by clicking the app's own window clear of the menu.
    Get-State
    Invoke-Click $app (ConvertTo-Screen $win @(($view.size[0] - 40), ($view.size[1] - 30))) 400
    Check 'and it closes again' (Wait-Menu $app -Closed)
  }

  # Ignore the mouse, forwarding moves; no presses while it lasts.
  $hovers = [int](Get-Value 'hovers')
  Press 'Ignore mouse 8s' 0.8
  Check 'setIgnoreMouseEvents is on' ((Get-Value 'ignoring') -eq 'true')
  Get-State
  $c = Get-TextCenter $view 'TARGET'
  $pt = ConvertTo-Screen $win $c
  $passed = $true
  foreach ($d in @(@(-80, -30), @(60, 20), @(-20, 40), @(90, -10))) {
    $p = ConvertTo-Screen $win @(($c[0] + $d[0]), ($c[1] + $d[1]))
    Move-Cursor $p 300
    if ([WInput]::Owner($p[0], $p[1]) -eq $app.Proc.Id) { $passed = $false }
  }
  Pause 0.5
  Check 'hit testing passes through the window' $passed
  $now = [int](Get-Value 'hovers')
  Check 'with forward, hovering still reaches the window' ($now -gt $hovers) "$hovers -> $now"
  Pause 8
  Check 'the window takes the mouse back after 8 s' ((Get-Value 'ignoring') -eq 'false' -and [WInput]::Owner($pt[0], $pt[1]) -eq $app.Proc.Id)
  $clicks = [int](Get-Value 'clicks')
  Press 'TARGET'
  Check 'a click lands again' ([int](Get-Value 'clicks') -eq $clicks + 1) (Get-Value 'clicks')

  # destroy() past preventClose.
  Check 'preventClose is still on' ((Get-Value 'preventClose') -eq 'true')
  Press 'destroy()' 0.2
  $exited = $app.Proc.WaitForExit(10000)
  Check 'destroy() closed the window and the app exited' $exited
} catch {
  Check 'the scenario ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
} finally {
  if (-not $app.Proc.HasExited) { Stop-GuiApp $app | Out-Null }
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
