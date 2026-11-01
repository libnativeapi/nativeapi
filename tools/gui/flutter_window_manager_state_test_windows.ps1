# No-input window_manager migration regression in the logged-on desktop.
# SSH runs in session 0, where the Windows shell's appbar service is absent.
param([switch]$Desktop)
$ErrorActionPreference = 'Stop'
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$scratch = Join-Path $workspace 'build\window_manager_migration'
New-Item -ItemType Directory -Force $scratch | Out-Null
$result = Join-Path $scratch 'result.txt'
$fixture = Join-Path $workspace 'bindings\dart\window_manager\example\build\windows\x64\runner\Debug\window_manager_example.exe'
$native = Join-Path $workspace 'core\build-wm\tests\Debug\window_migration_test.exe'

if ($Desktop) {
    $lines = [System.Collections.Generic.List[string]]::new()
    $exitCode = 0
    try {
        foreach ($executable in @($native, $fixture)) {
            if (!(Test-Path $executable)) { throw "Missing executable: $executable" }
            $tag = [IO.Path]::GetFileNameWithoutExtension($executable)
            $outLog = Join-Path $scratch "$tag.stdout.log"
            $errLog = Join-Path $scratch "$tag.stderr.log"
            $start = [Diagnostics.ProcessStartInfo]::new()
            $start.FileName = $executable
            $start.WorkingDirectory = Split-Path $executable
            $start.UseShellExecute = $false
            $start.RedirectStandardOutput = $true
            $start.RedirectStandardError = $true
            $process = [Diagnostics.Process]::new()
            $process.StartInfo = $start
            $process.Start() | Out-Null
            $stdout = $process.StandardOutput.ReadToEndAsync()
            $stderr = $process.StandardError.ReadToEndAsync()
            if (!$process.WaitForExit(90000)) {
                $process.Kill()
                throw "Timed out: $executable"
            }
            $output = $stdout.Result + $stderr.Result
            $stdout.Result | Set-Content -Encoding UTF8 $outLog
            $stderr.Result | Set-Content -Encoding UTF8 $errLog
            $lines.Add($output)
            if ($process.ExitCode -ne 0) { throw "Failed ($($process.ExitCode)): $executable" }
            if ($executable -eq $fixture -and $output -notmatch 'PASS window_manager native state regression') {
                throw 'Fixture did not report completion'
            }
        }
    } catch {
        $lines.Add("FAIL $_")
        $exitCode = 1
    }
    $lines.Add("EXIT $exitCode")
    $lines | Set-Content -Encoding UTF8 $result
    exit $exitCode
}

Remove-Item $result -ErrorAction SilentlyContinue
$taskName = 'NativeAPIWindowMigration-' + [Guid]::NewGuid().ToString('N')
try {
    $action = New-ScheduledTaskAction -Execute 'powershell.exe' `
        -Argument "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`" -Desktop"
    $principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive -RunLevel Limited
    Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal | Out-Null
    Start-ScheduledTask -TaskName $taskName
    $deadline = (Get-Date).AddSeconds(200)
    while (!(Test-Path $result) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 300 }
    if (!(Test-Path $result)) { throw 'Desktop regression did not finish; verify that the user is logged on' }
    Get-Content $result
    if ((Get-Content $result -Raw) -notmatch 'EXIT 0\s*$') { exit 1 }
} finally {
    Stop-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
}
