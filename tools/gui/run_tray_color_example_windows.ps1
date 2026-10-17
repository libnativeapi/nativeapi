# Launch the native C++ demo in the logged-on desktop session. No input sent.
param([switch]$DesktopSession, [string]$BuildDirectory = "core\build-tray-theme")
$ErrorActionPreference = "Stop"
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$build = Join-Path $workspace $BuildDirectory
$exe = Join-Path $build "examples\tray_color_example\Debug\tray_color_example.exe"
$statusFile = Join-Path $build "tray-colours-status.json"
$stdout = Join-Path $build "tray-colours.log"
$stderr = Join-Path $build "tray-colours-errors.log"
if (-not (Test-Path $exe)) { throw "Build the demo first: $exe" }

if ($DesktopSession) {
  $process = Start-Process $exe -PassThru -WindowStyle Hidden -RedirectStandardOutput $stdout -RedirectStandardError $stderr
  $deadline = (Get-Date).AddSeconds(20)
  do {
    Start-Sleep -Milliseconds 250
    $process.Refresh()
  } until ($process.HasExited -or $process.MainWindowHandle -ne 0 -or (Get-Date) -gt $deadline)
  $result = @{
    pid = $process.Id
    session = $process.SessionId
    title = $process.MainWindowTitle
    window = $process.MainWindowHandle.ToInt64()
    exited = $process.HasExited
    executable = $exe
  }
  $result | ConvertTo-Json | Out-File "$statusFile.tmp" -Encoding utf8
  Move-Item "$statusFile.tmp" $statusFile -Force
  if ($process.HasExited -or $process.MainWindowHandle -eq 0) { throw "No demo window appeared" }
  $process.WaitForExit()
  exit $process.ExitCode
}

# Reuse only this exact executable, without disturbing another running example.
$existing = Get-Process tray_color_example -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $exe -and $_.SessionId -ne 0 }
if ($existing) {
  # Session 0 cannot query the interactive desktop's window title. The runner
  # recorded it from that desktop when the process started.
  if (Test-Path $statusFile) {
    Get-Content $statusFile -Raw
  } else {
    $existing | Select-Object Id,SessionId | ConvertTo-Json
  }
  exit 0
}
if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $_.SessionId -ne 0 })) {
  throw "The host has no logged-on desktop session"
}
Remove-Item $statusFile -ErrorAction SilentlyContinue
$taskName = "NativeAPI-Tray-Colours-Demo"
$arguments = '-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -DesktopSession -BuildDirectory "' + $BuildDirectory + '"'
$action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument $arguments
$userSid = [System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value
$principal = New-ScheduledTaskPrincipal -UserId $userSid -LogonType Interactive -RunLevel Limited
$settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::Zero) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal -Settings $settings -Force | Out-Null
Start-ScheduledTask -TaskName $taskName
$deadline = (Get-Date).AddSeconds(25)
while (-not (Test-Path $statusFile) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 250 }
if (-not (Test-Path $statusFile)) { throw "The desktop launch produced no status; inspect task $taskName" }
$result = Get-Content $statusFile -Raw | ConvertFrom-Json
if ($result.exited -or $result.window -eq 0 -or $result.title -ne "Tray Colours") { throw "The demo window failed: $(Get-Content $statusFile -Raw)" }
Get-Content $statusFile -Raw
