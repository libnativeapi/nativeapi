# Issue #23: hold the maximize button, cross several InkWells, then release.
# Requires an interactive desktop and permission to use the mouse (~55 seconds).
# Build the title-bar example with -t tools/gui/fixtures/maximize_button_drag_flutter.dart
# (an absolute target path works from the example directory), then run this script.
param([string]$Exe = '')
$ErrorActionPreference = 'Stop'
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$RemoteScratch = Join-Path $PSScriptRoot 'output\maximize-button-drag'
$Python = (Get-Command python).Source
New-Item -ItemType Directory -Force $RemoteScratch | Out-Null
$harness = Join-Path $workspace '.agents\skills\gui-test\scripts\windows'
. (Join-Path $harness 'winput.ps1')
. (Join-Path $harness 'guiapp.ps1')
Copy-Item -LiteralPath (Join-Path $workspace '.agents\skills\flutter-ui-probe\scripts\uiprobe.py') `
  -Destination (Join-Path $RemoteScratch 'uiprobe.py') -Force
if (-not $Exe) {
  $Exe = Get-FlutterExe (Join-Path $workspace 'examples\flutter_window_title_bar_example') 'window_title_bar_example'
}
if (-not (Test-Path -LiteralPath $Exe)) { throw "Missing fixture: $Exe" }
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class MaximizeDragWin {
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr hwnd);
}
'@

function Read-State {
  $script:win = Get-Win $app 'Maximize drag regression'
  $script:view = Get-Views $app | Where-Object { Test-ViewText $_ 'Drag fixture' } | Select-Object -First 1
  if (-not $script:win -or -not $script:view) { throw 'Fixture window/view is missing' }
}
function Point-For([string]$Text) {
  ConvertTo-Screen $script:win (Get-TextCenter $script:view $Text)
}
function Get-Counts {
  $line = @($script:view.texts | Where-Object { $_[0] -match '^Downs: ' })[0][0]
  if ($line -notmatch '^Downs: (\d+)  Ups: (\d+)  Cancels: (\d+)$') {
    throw "Missing pointer counts: $line"
  }
  @([int]$Matches[1], [int]$Matches[2], [int]$Matches[3])
}
function Check-Settled([string]$Label) {
  Check "$Label releases every pointer" (Test-ViewText $view 'Pointers: 0')
  Check "$Label clears every pressed state" (Test-ViewText $view 'Pressed: 0')
  Check "$Label disposes every ink splash" (Test-ViewText $view 'Splashes: 0')
  $counts = Get-Counts
  Check "$Label balances down/up/cancel" ($counts[0] -eq $counts[1] + $counts[2]) ($counts -join ', ')
}

Start-Result (Join-Path $RemoteScratch 'result.txt')
Assert-Idle
$log = Join-Path $RemoteScratch 'fixture.log'
$errorLog = Join-Path $RemoteScratch 'fixture.stderr.log'
# This is the visible interactive app under test, not a background helper.
$process = Start-Process -FilePath $Exe -PassThru `
  -RedirectStandardOutput $log -RedirectStandardError $errorLog
$app = @{ Proc = $process; Log = $log; Stdout = $null }
try {
  $deadline = (Get-Date).AddSeconds(40)
  do {
    Pause 0.5
    $saved = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { $views = @(Get-Views $app 2>$null) } catch { $views = @() }
    finally { $ErrorActionPreference = $saved }
    $readyView = $views | Where-Object { Test-ViewText $_ 'Drag fixture' } | Select-Object -First 1
    $readyWindow = Get-Win $app 'Maximize drag regression'
  } until (($readyView -and $readyWindow) -or $process.HasExited -or (Get-Date) -gt $deadline)
  Read-State
  [WInput]::Raise($win.Hwnd)
  Invoke-Click $app (Point-For 'Drag fixture')
  Pause 1.5
  Read-State
  $restoredWidth = $win.Right - $win.Left
  Check-Settled 'Initial state'

  for ($attempt = 1; $attempt -le 2; $attempt++) {
    Read-State
    $before = Get-Counts
    $start = Point-For 'Maximize'
    $legs = @()
    # The first leg crosses the native/non-client boundary. All later legs
    # cross distinct InkWells; repeated fake downs used to strand their ink.
    foreach ($card in @(3, 2, 1, 4, 5, 6)) {
      $point = Point-For "Card ${card}: 0"
      $legs += ,@($point[0], $point[1], 420)
    }
    Invoke-Drag $app $start $legs
    Pause 1.5
    Read-State
    $after = Get-Counts
    Check "Drag $attempt creates exactly one pointer down" ($after[0] - $before[0] -eq 1)
    Check "Drag $attempt does not maximize" (Test-ViewText $view 'Toggles: 0')
    Check-Settled "Drag $attempt"
  }

  # Every crossed card must still respond to an ordinary click.
  foreach ($card in 1..6) {
    Read-State
    Invoke-Click $app (Point-For "Card ${card}: 0")
    Pause 1.5
    Read-State
    Check "Card $card remains clickable" (Test-ViewText $view "Card ${card}: 1")
    Check-Settled "Card $card click"
  }

  Read-State
  Invoke-Click $app (Point-For 'Maximize')
  Pause 1.5
  Read-State
  Check 'Click still maximizes' ([MaximizeDragWin]::IsZoomed([IntPtr]$win.Hwnd))
  Check 'Maximized button reads Restore' (Test-ViewText $view 'Restore')
  Invoke-Click $app (Point-For 'Restore')
  Pause 1.5
  Read-State
  Check 'Click still restores' (-not [MaximizeDragWin]::IsZoomed([IntPtr]$win.Hwnd))
  Check 'Restore returns to the earlier width' ([Math]::Abs(($win.Right - $win.Left) - $restoredWidth) -le 2)
  Check 'Both maximize clicks reached Flutter' (Test-ViewText $view 'Toggles: 2')
  Check-Settled 'Maximize/restore'
} finally {
  Stop-GuiApp $app | Out-Null
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
exit $script:Failures
