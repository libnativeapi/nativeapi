# No-input Windows 11 corner regression and a comparison image for inspection.
# Build core's window_corner_preference_windows_test first, then run through
# remote-hosts desktop. CORE_CORNER_TEST_EXE can override the executable path.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class CornerCaptureDpi {
  [DllImport("user32.dll")] public static extern IntPtr SetThreadDpiAwarenessContext(IntPtr context);
}
'@
[CornerCaptureDpi]::SetThreadDpiAwarenessContext([IntPtr](-4)) | Out-Null
$exe = $env:CORE_CORNER_TEST_EXE
if (-not $exe) { $exe = "$RemoteScratch\core-build\tests\Debug\window_corner_preference_windows_test.exe" }
if (-not (Test-Path $exe)) { throw "Missing executable: $exe" }
$log = "$RemoteScratch\core_window_corner_preference_test.app.log"
$picture = "$RemoteScratch\core_window_corner_preference_test.png"
Assert-Idle
$info = New-Object System.Diagnostics.ProcessStartInfo
$info.FileName = $exe
$info.Arguments = '--preview'
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $info
$writer = New-Object System.IO.StreamWriter($log, $false)
$writer.AutoFlush = $true
$lines = New-Object System.Collections.Generic.List[string]
$captured = $false
try {
  if (-not $proc.Start()) { throw 'Unable to start corner test' }
  $read = $proc.StandardOutput.ReadLineAsync()
  $deadline = (Get-Date).AddSeconds(45)
  while ((Get-Date) -lt $deadline) {
    if ($read.IsCompleted) {
      $line = $read.Result
      if ($null -eq $line) { break }
      $lines.Add($line)
      $writer.WriteLine($line)
      if ($line -match '^PREVIEW (-?\d+) (-?\d+) (\d+) (\d+)$') {
        $left = [int]$Matches[1]; $top = [int]$Matches[2]
        $width = [int]$Matches[3]; $height = [int]$Matches[4]
        # The image is for visual inspection, not an automated assertion.
        $bitmap = New-Object System.Drawing.Bitmap($width, $height)
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
          $graphics.CopyFromScreen($left, $top, 0, 0, $bitmap.Size)
          $bitmap.Save($picture, [System.Drawing.Imaging.ImageFormat]::Png)
          $captured = $true
        } finally { $graphics.Dispose(); $bitmap.Dispose() }
      }
      $read = $proc.StandardOutput.ReadLineAsync()
    } else { Start-Sleep -Milliseconds 30 }
  }
  if (-not $proc.WaitForExit(5000)) { throw 'Corner test timed out' }
  $errorText = $proc.StandardError.ReadToEnd()
  if ($errorText) { Write-Output $errorText }
  $lines | Write-Output
  if ($proc.ExitCode -ne 0 -or -not $lines.Contains('ALL PASS')) { throw 'Corner regression failed' }
  if (-not $captured) { throw 'Windows 11 comparison preview missing' }
  Write-Output "PASS comparison saved: $picture"
} finally {
  if ($proc.Id -and -not $proc.HasExited) { $proc.Kill(); $proc.WaitForExit() }
  $writer.Dispose()
  $proc.Dispose()
}
