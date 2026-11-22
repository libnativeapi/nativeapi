# Run #50 in the logged-on Windows 11 desktop, without synthetic input.
# Build flutter_window_example with -t ../../tools/gui/window_frameless_focus_flutter.dart first.
param([switch]$DesktopSession, [string]$RunDirectory)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $DesktopSession) {
  if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object {$_.SessionId -ne 0})) {
    throw 'No logged-on Windows desktop'
  }
  $RunDirectory = Join-Path $workspace ('core\build\frameless-focus-' + [Guid]::NewGuid().ToString('N'))
  New-Item -ItemType Directory -Force $RunDirectory | Out-Null
  $taskName = 'NativeAPI-FramelessFocus-' + [Guid]::NewGuid().ToString('N')
  $arguments = '-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -DesktopSession -RunDirectory "' + $RunDirectory + '"'
  $action = New-ScheduledTaskAction -Execute powershell.exe -Argument $arguments
  $principal = New-ScheduledTaskPrincipal -UserId ([System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value) -LogonType Interactive -RunLevel Limited
  try {
    Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal -Settings (New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::FromMinutes(2)) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries) | Out-Null
    Start-ScheduledTask -TaskName $taskName
    $deadline = (Get-Date).AddSeconds(110)
    while (-not (Test-Path "$RunDirectory\result.json") -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 250 }
    if (-not (Test-Path "$RunDirectory\result.json")) { throw "No result: $RunDirectory" }
    Get-Content "$RunDirectory\stdout.log"
    Get-Content "$RunDirectory\stderr.log"
    $result = Get-Content "$RunDirectory\result.json" -Raw | ConvertFrom-Json
    $result | ConvertTo-Json
    exit ([int]$result.exitCode)
  } finally {
    Stop-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
  }
}

$result = @{exitCode = 1; session = (Get-Process -Id $PID).SessionId; directory = $RunDirectory}
$app = $null
try {
  if ($result.session -eq 0) { throw 'This test requires an interactive desktop session' }
  $bundle = Join-Path $workspace 'examples\flutter_window_example\build\windows\x64\runner\Debug'
  $app = Start-Process (Join-Path $bundle 'window_example.exe') -WorkingDirectory $bundle -PassThru -RedirectStandardOutput "$RunDirectory\stdout.log" -RedirectStandardError "$RunDirectory\stderr.log"
  $app.Handle | Out-Null
  if (-not $app.WaitForExit(45000)) { throw 'Fixture timed out' }
  if ($app.ExitCode -ne 0) { throw "Fixture exited with $($app.ExitCode)" }
  $line = Get-Content "$RunDirectory\stdout.log" | Where-Object {$_ -like 'FRAMELESS_RESULT *'} | Select-Object -Last 1
  if (-not $line) { throw 'Missing FRAMELESS_RESULT' }
  $report = $line.Substring('FRAMELESS_RESULT '.Length) | ConvertFrom-Json
  if ($report.passed -ne 33 -or @($report.failures).Count -ne 0) { throw ($report | ConvertTo-Json) }
  $result.passed = $report.passed
  $result.exitCode = 0
} catch {
  $result.error = $_.Exception.Message
} finally {
  if ($app -and -not $app.HasExited) { Stop-Process -Id $app.Id }
  $result | ConvertTo-Json | Out-File "$RunDirectory\result.json" -Encoding utf8
}
