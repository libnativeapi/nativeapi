# Real-pointer check of Window::SetMaximizeButtonBounds (#70) on Windows 11:
# resting the cursor on an app-drawn maximize button opens the snap layouts,
# and the content window under it still receives the moves. Hover only - no
# clicks, no keyboard. Build core/tests/window_maximize_button_test first.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

$exe = $env:CORE_SNAP_LAYOUT_TEST_EXE
if (-not $exe) { $exe = "$RemoteScratch\core-build\tests\Debug\window_maximize_button_test.exe" }
if (-not (Test-Path $exe)) { throw "Missing executable: $exe" }
Start-Result "$RemoteScratch\core_window_snap_layout_test.result.txt"
$build = (Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion').CurrentBuild
$flyout = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' -ErrorAction SilentlyContinue).EnableSnapAssistFlyout
Say "Windows build $build, EnableSnapAssistFlyout=$(if ($null -eq $flyout) { 'default (on)' } else { $flyout })"
Assert-Idle

$info = New-Object System.Diagnostics.ProcessStartInfo $exe
$info.Arguments = '--serve'
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.RedirectStandardOutput = $true
$proc = [System.Diagnostics.Process]::Start($info)
$lines = New-Object System.Collections.Generic.List[string]
$script:read = $proc.StandardOutput.ReadLineAsync()
function Read-AppLines {
  while ($script:read.IsCompleted) {
    $line = $script:read.Result
    if ($null -eq $line) { break }
    $lines.Add($line)
    $script:read = $proc.StandardOutput.ReadLineAsync()
  }
}
# Waits while draining the app's output: a full pipe would block its message loop.
function Wait-Reading([double]$Seconds) {
  $end = (Get-Date).AddSeconds($Seconds)
  do { Read-AppLines; Start-Sleep -Milliseconds 20 } while ((Get-Date) -lt $end)
  Read-AppLines
}
$app = @{ Proc = $proc; Log = $null; Stdout = $null }
$rest = $null
try {
  $deadline = (Get-Date).AddSeconds(20)
  do { Pause 0.1; Read-AppLines } until (@($lines | ? { $_ -like 'READY *' -or $_ -like 'FAIL*' }).Count -gt 0 -or (Get-Date) -gt $deadline)
  $ready = $lines | ? { $_ -like 'READY *' } | Select-Object -First 1
  if (-not $ready) { throw "The app did not get ready: $($lines -join '; ')" }
  $b = $ready.Split(' ')[1..4] | % { [int]$_ }
  $center = @(($b[0] + [int]($b[2] / 2)), ($b[1] + [int]($b[3] / 2)))
  $win = Get-Win $app 'nativeapi snap layout'
  if (-not $win) { throw 'The window is missing' }
  Raise-AppWindows $proc.Id
  # Approach over the window's own content, below the title band.
  $approach = @(($win.Left + 120), ($win.Top + 200))
  # Leave sideways along the title band, away from the flyout below the button.
  $rest = $null
  Assert-Owner $proc.Id $approach[0] $approach[1]
  Move-Cursor $approach 400
  Wait-Reading 0.5
  # The flyout opens below the button, over this window. The window under
  # that point tells: ours before, the shell's while the flyout is up.
  $probe = @($center[0], ($center[1] + [int]($b[3] * 2)))
  $ownerBefore = [WInput]::Owner($probe[0], $probe[1])
  Assert-Owner $proc.Id $center[0] $center[1]
  $countBefore = $lines.Count
  Move-Cursor $center 500
  Wait-Reading 2.0  # The flyout opens after a short rest.
  $ownerDuring = [WInput]::Owner($probe[0], $probe[1])
  $process = (Get-Process -Id $ownerDuring -ErrorAction SilentlyContinue).ProcessName
  # Evidence: the screen around the button while the flyout is up.
  $bmp = New-Object System.Drawing.Bitmap 900, 500
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen([Math]::Max(0, $center[0] - 600), [Math]::Max(0, $center[1] - 60), 0, 0, $bmp.Size)
  $bmp.Save("$RemoteScratch\core_window_snap_layout_test.png", [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Check 'below the button is this window before the hover' ($ownerBefore -eq $proc.Id) "pid $ownerBefore"
  Check 'resting on the button opens a window of another process over it (the snap layouts)' `
    ($ownerDuring -ne $proc.Id -and $ownerDuring -ne 0) "pid $ownerDuring ($process)"
  # Natively the content gets no move over the button (it steps aside in the
  # hit test), so a move it logs inside the button was forwarded.
  $c = ($lines | ? { $_ -like 'BUTTON_CONTENT *' } | Select-Object -First 1).Split(' ')[1..4] | % { [int]$_ }
  $moves = @($lines | Select-Object -Skip $countBefore | ? { $_ -like 'CONTENT WM_MOUSEMOVE*' } | ? {
    $f = $_.Split(' '); $x = [int]$f[2]; $y = [int]$f[3]
    $x -ge $c[0] -and $x -lt $c[0] + $c[2] -and $y -ge $c[1] -and $y -lt $c[1] + $c[3]
  })
  Check 'the content window received the moves over the button' ($moves.Count -gt 0) "$($moves.Count) moves, last: $($moves | Select-Object -Last 1)"
  # Leave the button: the flyout closes and the content hears it.
  $rest = @(($center[0] - [int]($b[2] * 5)), $center[1])
  Assert-Owner $proc.Id $rest[0] $rest[1]
  Move-Cursor $rest 400
  Wait-Reading 1.5
  $ownerAfter = [WInput]::Owner($probe[0], $probe[1])
  Check 'the snap layouts closed after leaving' ($ownerAfter -eq $proc.Id) "pid $ownerAfter"
  Check 'the content window heard the pointer leave the button' (@($lines | ? { $_ -like 'CONTENT WM_MOUSELEAVE*' }).Count -gt 0)
  Check 'no click was sent' (@($lines | ? { $_ -like 'CONTENT WM_LBUTTON*' }).Count -eq 0)
} finally {
  if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue }
  $lines | Set-Content "$RemoteScratch\core_window_snap_layout_test.app.log" -Encoding utf8
}
