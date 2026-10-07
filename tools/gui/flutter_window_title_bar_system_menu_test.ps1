# End-to-end check of ShowSystemMenu (#66) from Flutter on Windows: a
# secondary click on flutter_window_title_bar_example's DragToMoveArea strip
# opens the native system menu at the pointer, with the item states of the
# window's current state, and a chosen item acts on the window. Owner-guarded
# clicks on the example's own window and menu only, no keyboard input.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class MenuWin {
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
}
"@

Start-Result "$RemoteScratch\flutter_window_title_bar_system_menu_test.result.txt"
$exe = $env:TITLE_BAR_EXAMPLE_EXE
if (-not $exe) {
  $exe = Get-FlutterExe "$RemoteWorkspace\examples\flutter_window_title_bar_example" 'window_title_bar_example'
}
Assert-Idle
$app = Start-GuiApp $exe
# The probe fails (a traceback on stderr) until the VM service is up, and
# PowerShell 5.1 turns native stderr into a terminating error under Stop.
function Get-ViewsQuietly {
  $saved = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try { @(Get-Views $app 2>$null) } catch { @() } finally { $ErrorActionPreference = $saved }
}
function Get-State {
  $script:win = Get-Win $app 'nativeapi*'
  if (-not $script:win) { throw 'The example window is missing' }
  $script:view = Get-ViewsQuietly | Select-Object -First 1
  if (-not $script:view) { throw 'The example has no Flutter view to probe' }
}
# The system menu's items in their fixed order, whatever the display
# language: Restore, Move, Size, Minimize, Maximize, Close.
function Open-SystemMenu([string]$When) {
  Get-State
  $point = ConvertTo-Screen $win (Get-TextCenter $view 'Drag this strip to move the window')
  Invoke-RightClick $app $point
  if (-not (Wait-Menu $app)) { throw "No system menu opened $When" }
  $menu = @(Get-OpenMenus $app)[0]
  Say "  menu $When at $($menu.Rect[0]),$($menu.Rect[1]) for a click at $($point[0]),$($point[1]): $(($menu.Items | % { "$($_.Title)=$(if ($_.Enabled) { 'on' } else { 'off' })" }) -join ', ')"
  Check "the system menu opens at the pointer $When" `
    ([Math]::Abs($menu.Rect[0] - $point[0]) -le 4 -and [Math]::Abs($menu.Rect[1] - $point[1]) -le 4) `
    "menu $($menu.Rect[0]),$($menu.Rect[1]) vs click $($point[0]),$($point[1])"
  if ($menu.Items.Count -ne 6) { throw "Expected the 6 system menu items, got $($menu.Items.Count)" }
  return $menu
}
function Select-SystemMenuItem($Menu, [int]$Index) {
  $pt = Get-RectCenter $Menu.Items[$Index].Rect
  Move-Cursor $pt 400
  Pause 0.2
  Assert-Owner $app.Proc.Id $pt[0] $pt[1]
  [WInput]::Click($pt[0], $pt[1])
  Pause 1.5
}

try {
  $deadline = (Get-Date).AddSeconds(40)
  while (@(Get-ViewsQuietly).Count -eq 0 -and (Get-Date) -lt $deadline) { Pause 0.5 }
  Get-State
  Invoke-Activate $app $win
  $width = $win.Right - $win.Left

  $menu = Open-SystemMenu 'on a restored window'
  $on = @($menu.Items | % { $_.Enabled })
  Check 'restored: Restore is off; Move, Size, Minimize, Maximize, Close are on' `
    (-not $on[0] -and $on[1] -and $on[2] -and $on[3] -and $on[4] -and $on[5])
  Select-SystemMenuItem $menu 4
  Get-State
  Check 'choosing Maximize maximizes the window' ([MenuWin]::IsZoomed([IntPtr]$win.Hwnd))

  $menu = Open-SystemMenu 'on a maximized window'
  $on = @($menu.Items | % { $_.Enabled })
  Check 'maximized: Restore, Minimize, Close are on; Move, Size, Maximize are off' `
    ($on[0] -and -not $on[1] -and -not $on[2] -and $on[3] -and -not $on[4] -and $on[5])
  Select-SystemMenuItem $menu 0
  Get-State
  Check 'choosing Restore restores the window' (-not [MenuWin]::IsZoomed([IntPtr]$win.Hwnd))
  Check 'at its earlier width' ([Math]::Abs(($win.Right - $win.Left) - $width) -le 2) "$($win.Right - $win.Left) vs $width"
} catch {
  Check 'the scenario ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
} finally {
  Stop-GuiApp $app | Out-Null
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
