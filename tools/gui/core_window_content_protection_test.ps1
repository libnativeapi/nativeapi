# No-input regression: native display affinity and capture of our own pixels.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
$exe = $env:CORE_CONTENT_PROTECTION_TEST_EXE
if (-not $exe) { $exe = "$RemoteScratch\core-build\tests\Debug\window_content_protection_windows_test.exe" }
if (-not (Test-Path $exe)) { throw "Missing executable: $exe" }
Assert-Idle
$info = New-Object System.Diagnostics.ProcessStartInfo
$info.FileName = $exe
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $info
try {
  if (-not $proc.Start()) { throw 'Unable to start content protection test' }
  $readOutput = $proc.StandardOutput.ReadToEndAsync()
  $readError = $proc.StandardError.ReadToEndAsync()
  if (-not $proc.WaitForExit(30000)) { throw 'Content protection test timed out' }
  $outputText = $readOutput.Result
  $errorText = $readError.Result
  $outputText | Out-File "$RemoteScratch\core_window_content_protection_test.app.log" -Encoding utf8
  Write-Output $outputText
  if ($errorText) { Write-Output $errorText }
  if ($proc.ExitCode -ne 0 -or $outputText -notmatch '(?m)^ALL PASS\r?$') { throw 'Content protection regression failed' }
} finally {
  if ($proc.Id -and -not $proc.HasExited) { $proc.Kill(); $proc.WaitForExit() }
  $proc.Dispose()
}
