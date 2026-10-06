# Guarded real-click regression for core's Windows focus policies.
# Build core/tests/window_focus_policy_test first. No keyboard input.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
$foreign = $env:CORE_FOCUS_TEST_FOREIGN -eq '1'
$suffix = if ($foreign) { '_foreign' } else { '' }
Start-Result "$RemoteScratch\core_window_focus_policy_test$suffix.result.txt"
$exe = "$RemoteScratch\core-build\tests\Debug\window_focus_policy_test.exe"
if ($env:CORE_FOCUS_TEST_EXE) { $exe = $env:CORE_FOCUS_TEST_EXE }
if (-not (Test-Path $exe)) { throw "Missing $exe" }
$out = "$RemoteScratch\core_window_focus_policy_test$suffix.app.log"
$err = "$RemoteScratch\core_window_focus_policy_test$suffix.error.log"
Assert-Idle
$helper = $null
$anchorApp = $null
if ($foreign) {
  $helperInfo = New-Object System.Diagnostics.ProcessStartInfo $exe
  $helperInfo.Arguments = '--anchor'
  $helperInfo.UseShellExecute = $false
  $helperInfo.CreateNoWindow = $true
  $helper = [System.Diagnostics.Process]::Start($helperInfo)
  $anchorApp = @{ Proc = $helper; Log = $null; Stdout = $null }
  try {
    $deadline = (Get-Date).AddSeconds(15)
    do { Pause 0.1; $anchor = Get-Win $anchorApp 'nativeapi foreign focus anchor' } until ($anchor -or $helper.HasExited -or (Get-Date) -gt $deadline)
    if (-not $anchor) { throw 'Foreign anchor did not appear' }
    Invoke-Activate $anchorApp $anchor
  } catch {
    Stop-Process -Id $helper.Id -ErrorAction SilentlyContinue
    throw
  }
}
# CreateNoWindow suppresses only the console. WindowStyle Hidden would make
# Windows ignore the app's first Show() and hide the anchor itself.
$psi = New-Object System.Diagnostics.ProcessStartInfo $exe
$psi.Arguments = if ($foreign) { '--foreign-clicks' } else { '--clicks' }
$psi.UseShellExecute = $false
$psi.CreateNoWindow = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$proc = [System.Diagnostics.Process]::Start($psi)
$script:PendingLine = $proc.StandardOutput.ReadLineAsync()
$script:OutputEnded = $false
$errorTask = $proc.StandardError.ReadToEndAsync()
Set-Content -Path $out -Value '' -Encoding utf8
function Read-AppLog {
  while (-not $script:OutputEnded -and $script:PendingLine.IsCompleted) {
    $line = $script:PendingLine.Result
    if ($null -eq $line) { $script:OutputEnded = $true; break }
    Add-Content -Path $out -Value $line -Encoding utf8
    $script:PendingLine = $proc.StandardOutput.ReadLineAsync()
  }
}
$app = @{ Proc = $proc; Log = $null; Stdout = $null }
try {
  $deadline = (Get-Date).AddSeconds(15)
  do {
    Pause 0.1
    Read-AppLog
    $initial = @(Get-Content $out -ErrorAction SilentlyContinue)
  } until ($initial -contains 'WAIT_ACTIVATION' -or $initial -contains 'READY 0' -or $proc.HasExited -or (Get-Date) -gt $deadline)
  if ($initial -contains 'WAIT_ACTIVATION') {
    $activationApp = if ($foreign) { $anchorApp } else { $app }
    $anchorTitle = if ($foreign) { 'nativeapi foreign focus anchor' } else { 'nativeapi focus anchor' }
    $anchor = Get-Win $activationApp $anchorTitle
    if (-not $anchor) { throw 'Anchor missing during activation' }
    Invoke-Activate $activationApp $anchor
  }
  foreach ($mode in 0..3) {
    $deadline = (Get-Date).AddSeconds(20)
    do {
      Pause 0.1
      Read-AppLog
      $ready = @(Get-Content $out -ErrorAction SilentlyContinue | Where-Object { $_ -eq "READY $mode" })
    } until ($ready.Count -gt 0 -or $proc.HasExited -or (Get-Date) -gt $deadline)
    if ($ready.Count -eq 0) { throw "The app did not reach mode $mode" }
    $win = Get-Win $app 'nativeapi focus palette'
    if (-not $win) { throw 'Palette disappeared' }
    $root = [System.Windows.Automation.AutomationElement]::FromHandle([IntPtr]$win.Hwnd)
    $condition = New-Object System.Windows.Automation.PropertyCondition ([System.Windows.Automation.AutomationElement]::NameProperty), 'Click without activating'
    $button = $root.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $condition)
    if (-not $button) { throw 'Native button missing' }
    $rect = $button.Current.BoundingRectangle
    if ($rect.Width -le 0 -or $rect.Height -le 0) { throw 'Native button has no visible bounds' }
    Invoke-Click $app @([int]($rect.X + $rect.Width / 2), [int]($rect.Y + $rect.Height / 2))
  }
  if (-not $proc.WaitForExit(15000)) { throw 'The app did not exit after the last click' }
  $deadline = (Get-Date).AddSeconds(3)
  do { Read-AppLog; Pause 0.05 } until ($script:OutputEnded -or (Get-Date) -gt $deadline)
  $lines = @(Get-Content $out)
  Check 'the real-click regression finished' ($lines -contains 'ALL PASS') ($lines -join '; ')
  Check 'all four blocked modes received a click' (@($lines | Where-Object { $_ -eq 'PASS blocked palette receives real button click' }).Count -eq 4)
  Check 'all four clicks preserved keyboard focus' (@($lines | Where-Object { $_ -eq 'PASS real click preserves anchor focus' }).Count -eq 4)
} catch {
  Check 'ran to completion' $false "$_"
} finally {
  Stop-Process -Id $proc.Id -ErrorAction SilentlyContinue
  $proc.WaitForExit(3000) | Out-Null
  Read-AppLog
  if ($errorTask.Wait(3000)) { Set-Content -Path $err -Value $errorTask.Result -Encoding utf8 }
  if ($helper) { Stop-Process -Id $helper.Id -ErrorAction SilentlyContinue }
}
Get-Content $out -ErrorAction SilentlyContinue | ForEach-Object { Say "app: $_" }
Get-Content $err -ErrorAction SilentlyContinue | ForEach-Object { Say "stderr: $_" }
exit $script:Failures
