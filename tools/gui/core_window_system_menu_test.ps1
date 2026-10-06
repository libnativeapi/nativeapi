# Real system-menu selection with owner-guarded clicks, no keyboard input.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
$exe = $env:CORE_SYSTEM_MENU_TEST_EXE
if (-not $exe) { $exe = "$RemoteScratch\core-build\tests\Debug\window_system_menu_windows_test.exe" }
if (-not (Test-Path $exe)) { throw "Missing executable: $exe" }
Assert-Idle
$info = New-Object System.Diagnostics.ProcessStartInfo
$info.FileName = $exe
$info.Arguments = '--clicks'
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $info
$writer = New-Object System.IO.StreamWriter("$RemoteScratch\core_window_system_menu_test.app.log", $false)
$writer.AutoFlush = $true
$lines = New-Object System.Collections.Generic.List[string]
$clicks = 0
try {
  if (-not $proc.Start()) { throw 'Unable to start system menu test' }
  $readError = $proc.StandardError.ReadToEndAsync()
  $read = $proc.StandardOutput.ReadLineAsync()
  $app = @{ Proc = $proc; Log = $null; Stdout = $null }
  $deadline = (Get-Date).AddSeconds(90)
  while ((Get-Date) -lt $deadline) {
    if ($read.IsCompleted) {
      $line = $read.Result
      if ($null -eq $line) { break }
      $writer.WriteLine($line)
      $lines.Add($line)
      if ($line -match '^CLICK (\d+) (-?\d+) (-?\d+)$') {
        # Invoke-Click moves smoothly and checks point ownership immediately
        # before pressing. Menu item coordinates come from GetMenuItemRect.
        Invoke-Click $app @([int]$Matches[2], [int]$Matches[3])
        $clicks++
      }
      $read = $proc.StandardOutput.ReadLineAsync()
    } else { Start-Sleep -Milliseconds 30 }
  }
  if (-not $proc.WaitForExit(5000)) { throw 'System menu test timed out' }
  $errorText = $readError.Result
  if ($errorText) { Write-Output $errorText }
  $lines | Write-Output
  if ($proc.ExitCode -ne 0 -or -not $lines.Contains('ALL PASS') -or $clicks -ne 6) { throw 'System menu regression failed' }
} finally {
  if ($proc.Id -and -not $proc.HasExited) { $proc.Kill(); $proc.WaitForExit() }
  $writer.Dispose()
  $proc.Dispose()
}
