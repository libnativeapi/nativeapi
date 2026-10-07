# End-to-end check of MaximizeButtonArea (#70) in a real Flutter app on
# Windows 11: flutter_window_title_bar_example with its title bar hidden draws
# its own Maximize / Restore chip, wrapped in MaximizeButtonArea. Resting the
# cursor on it opens the snap layouts; clicking it maximizes and restores.
# Owner-guarded clicks on the example's own window only, no keyboard input.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class SnapWin {
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
}
"@
Add-Type -AssemblyName System.Drawing

Start-Result "$RemoteScratch\flutter_window_title_bar_snap_test.result.txt"
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
try {
  $deadline = (Get-Date).AddSeconds(40)
  while (@(Get-ViewsQuietly).Count -eq 0 -and (Get-Date) -lt $deadline) { Pause 0.5 }
  Get-State
  Invoke-Activate $app $win
  Invoke-Click $app (ConvertTo-Screen $win (Get-TextCenter $view 'Hidden'))
  Pause 1.5
  Get-State
  Check 'Hidden puts a Maximize chip into the strip' (Test-ViewText $view 'Maximize')
  $restoredWidth = $win.Right - $win.Left

  # Rest on the chip: the snap layouts open below it, over this window.
  $button = ConvertTo-Screen $win (Get-TextCenter $view 'Maximize')
  $below = @($button[0], ($button[1] + [int](80 * $win.Scale)))
  $ownerBefore = [WInput]::Owner($below[0], $below[1])
  Assert-Owner $app.Proc.Id $button[0] $button[1]
  Move-Cursor $button 500
  Pause 2.0
  $ownerDuring = [WInput]::Owner($below[0], $below[1])
  $bmp = New-Object System.Drawing.Bitmap 900, 520
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen([Math]::Max(0, $button[0] - 650), [Math]::Max(0, $button[1] - 60), 0, 0, $bmp.Size)
  $bmp.Save("$RemoteScratch\flutter_window_title_bar_snap_test.png", [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Check 'below the chip is the example before the hover' ($ownerBefore -eq $app.Proc.Id) "pid $ownerBefore"
  $process = (Get-Process -Id $ownerDuring -ErrorAction SilentlyContinue).ProcessName
  Check 'resting on the chip opens the snap layouts over it' `
    ($ownerDuring -ne $app.Proc.Id -and $ownerDuring -ne 0) "pid $ownerDuring ($process)"

  # Leave sideways along the strip, away from the flyout.
  $side = @(($button[0] - [int](220 * $win.Scale)), $button[1])
  Assert-Owner $app.Proc.Id $side[0] $side[1]
  Move-Cursor $side 400
  Pause 1.5
  Check 'leaving the chip closes the snap layouts' ([WInput]::Owner($below[0], $below[1]) -eq $app.Proc.Id)

  # A click on the chip reaches Flutter, which maximizes the window.
  Invoke-Click $app $button 400
  Pause 1.5
  Get-State
  Check 'clicking Maximize maximizes the window' ([SnapWin]::IsZoomed([IntPtr]$win.Hwnd)) "width $($win.Right - $win.Left)"
  Check 'the chip now reads Restore' (Test-ViewText $view 'Restore')

  $button = ConvertTo-Screen $win (Get-TextCenter $view 'Restore')
  Invoke-Click $app $button 400
  Pause 1.5
  Get-State
  Check 'clicking Restore restores the window' (-not [SnapWin]::IsZoomed([IntPtr]$win.Hwnd))
  Check 'at its earlier width' ([Math]::Abs(($win.Right - $win.Left) - $restoredWidth) -le 2) "$($win.Right - $win.Left) vs $restoredWidth"
  Check 'the chip reads Maximize again' (Test-ViewText $view 'Maximize')
} finally {
  Stop-GuiApp $app | Out-Null
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
