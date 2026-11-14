# Real WinEvent / owned COM dialog regression for #29. No mouse or keyboard input.
param([switch]$DesktopSession, [switch]$ConstructionOnly, [switch]$SkipBuild, [string]$RunDirectory)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$build = Join-Path $workspace 'core\build-issue29'
$targets = @('window_cross_thread_dialog_windows_test', 'window_property_changed_test', 'window_manager_ui_thread_windows_test', 'window_controls_test')

if ($DesktopSession) {
  $result = @{ session = (Get-Process -Id $PID).SessionId; exitCode = 0; tests = @() }
  try {
    $selected = if ($ConstructionOnly) { @($targets[0]) } else { $targets }
    foreach ($target in $selected) {
      $exe = Join-Path $build ('tests\Release\' + $target + '.exe')
      $options = @{ FilePath = $exe; WorkingDirectory = $workspace; PassThru = $true;
        RedirectStandardOutput = (Join-Path $RunDirectory ($target + '.stdout.log'));
        RedirectStandardError = (Join-Path $RunDirectory ($target + '.stderr.log')) }
      if ($ConstructionOnly) { $options.ArgumentList = '--construction-only' }
      $process = Start-Process @options
      $process.Handle | Out-Null
      if (-not $process.WaitForExit(120000)) {
        Stop-Process -Id $process.Id -Force
        throw "$target timed out"
      }
      $result.tests += @{ name = $target; exitCode = $process.ExitCode }
      if ($process.ExitCode -ne 0) { $result.exitCode = 1 }
    }
  } catch {
    $result.exitCode = 1
    $_ | Out-String | Out-File (Join-Path $RunDirectory 'launcher-error.log') -Encoding utf8
  } finally {
    $result | ConvertTo-Json -Depth 5 | Out-File (Join-Path $RunDirectory 'result.json') -Encoding utf8
  }
  exit $result.exitCode
}

if (-not $SkipBuild) {
  & cmake -S (Join-Path $workspace 'core') -B $build -DNATIVEAPI_BUILD_EXAMPLES=OFF -DBUILD_TESTING=ON
  if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }
  $log = Join-Path $build 'build.log'
  $targetArgs = $targets -join ' '
  cmd /c "cmake --build `"$build`" --config Release --target $targetArgs -- /nologo /v:minimal > `"$log`" 2>&1"
  if ($LASTEXITCODE -ne 0) { Get-Content $log; throw 'Build failed' }
}
if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $_.SessionId -ne 0 })) {
  throw 'The host has no logged-on desktop session'
}
$runId = [Guid]::NewGuid().ToString('N')
$RunDirectory = Join-Path $workspace ('tools\gui\output\owned-dialog-' + $runId)
New-Item -ItemType Directory -Path $RunDirectory -Force | Out-Null
$taskName = 'NativeAPI-OwnedDialog-' + $runId
$arguments = '-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -DesktopSession -RunDirectory "' + $RunDirectory + '"'
if ($ConstructionOnly) { $arguments += ' -ConstructionOnly' }
$action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument $arguments
$principal = New-ScheduledTaskPrincipal -UserId ([System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value) -LogonType Interactive -RunLevel Limited
$settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::FromMinutes(6)) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
try {
  Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal -Settings $settings | Out-Null
  Start-ScheduledTask -TaskName $taskName
  $deadline = (Get-Date).AddSeconds(340)
  $resultFile = Join-Path $RunDirectory 'result.json'
  while (-not (Test-Path $resultFile) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 250 }
  if (-not (Test-Path $resultFile)) { throw "Desktop check timed out: $RunDirectory" }
  Get-ChildItem $RunDirectory -Filter '*.log' | ForEach-Object { Get-Content $_.FullName }
  $result = Get-Content $resultFile -Raw | ConvertFrom-Json
  $result | ConvertTo-Json -Depth 5
  Write-Output "RESULT_DIRECTORY=$RunDirectory"
  if ($result.session -eq 0 -or $result.exitCode -ne 0) { throw "Desktop check failed: $RunDirectory" }
} finally {
  Stop-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
  Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
}
