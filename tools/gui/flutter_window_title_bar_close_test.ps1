# End-to-end check of cancellable close requests (#65) from Flutter on
# Windows: flutter_window_title_bar_example's "Keep open" cancels every close
# request through WindowCloseRequestedEvent. The window's own close button and
# the system menu's Close are clicked for real: cancelled, the window stays;
# allowed, it closes and the app exits normally. Owner-guarded clicks on the
# example's own window and menu only, no keyboard input.
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class CloseWin {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr h, int a, out RECT r, int size);
  [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr h);
  // DWMWA_CAPTION_BUTTON_BOUNDS, relative to the window: minimize, maximize, close.
  public static int[] CaptionButtons(IntPtr h) {
    RECT r; if (DwmGetWindowAttribute(h, 5, out r, Marshal.SizeOf(typeof(RECT))) != 0) return null;
    return new[] { r.L, r.T, r.R, r.B };
  }
}
"@

Start-Result "$RemoteScratch\flutter_window_title_bar_close_test.result.txt"
$exe = $env:TITLE_BAR_EXAMPLE_EXE
if (-not $exe) {
  $exe = Get-FlutterExe "$RemoteWorkspace\examples\flutter_window_title_bar_example" 'window_title_bar_example'
}
Assert-Idle
$app = Start-GuiApp $exe
# Opened now, or ExitCode stays empty for a Start-Process -PassThru process.
$null = $app.Proc.Handle
# The probe fails (a traceback on stderr) until the VM service is up, and
# PowerShell 5.1 turns native stderr into a terminating error under Stop.
function Get-ViewsQuietly {
  $saved = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try { @(Get-Views $app 2>$null) } catch { @() } finally { $ErrorActionPreference = $saved }
}
function Get-State {
  $script:win = Get-Win $app 'nativeapi*'
  if (-not $script:win) { throw 'The example window is missing' }
  $script:view = Get-ViewsQuietly | Select-Object -First 1
  if (-not $script:view) { throw 'The example has no Flutter view to probe' }
}
function Get-Cancelled {
  Get-State
  $t = @($view.texts | % { $_[0] } | ? { $_ -like 'Cancelled closes: *' }) | Select-Object -First 1
  if (-not $t) { throw 'The cancelled-closes counter is missing' }
  [int]$t.Split(':')[1].Trim()
}
function Click-CloseButton {
  Get-State
  $b = [CloseWin]::CaptionButtons([IntPtr]$win.Hwnd)
  if (-not $b) { throw 'No caption button bounds' }
  $third = [int](($b[2] - $b[0]) / 3)
  $pt = @(($win.Left + $b[2] - [int]($third / 2)), ($win.Top + [int](($b[1] + $b[3]) / 2)))
  Invoke-Click $app $pt 400
  Pause 1.5
}

try {
  $deadline = (Get-Date).AddSeconds(40)
  while (@(Get-ViewsQuietly).Count -eq 0 -and (Get-Date) -lt $deadline) { Pause 0.5 }
  Get-State
  Invoke-Activate $app $win
  Invoke-Click $app (ConvertTo-Screen $win (Get-TextCenter $view 'Keep open')) 400
  Pause 0.8
  Check 'the counter starts at 0' ((Get-Cancelled) -eq 0)

  Click-CloseButton
  Check 'with Keep open, the close button leaves the window open' ([CloseWin]::IsWindow([IntPtr]$win.Hwnd) -and -not $app.Proc.HasExited)
  Check 'and the example cancelled one request' ((Get-Cancelled) -eq 1)

  # The system menu's Close (the last of its fixed items), from the strip.
  Invoke-RightClick $app (ConvertTo-Screen $win (Get-TextCenter $view 'Drag this strip to move the window'))
  if (-not (Wait-Menu $app)) { throw 'No system menu opened' }
  $menu = @(Get-OpenMenus $app)[0]
  if ($menu.Items.Count -ne 6) { throw "Expected the 6 system menu items, got $($menu.Items.Count)" }
  $pt = Get-RectCenter $menu.Items[5].Rect
  Move-Cursor $pt 400
  Pause 0.2
  Assert-Owner $app.Proc.Id $pt[0] $pt[1]
  [WInput]::Click($pt[0], $pt[1])
  Pause 1.5
  Check 'with Keep open, the system menu Close leaves the window open' ([CloseWin]::IsWindow([IntPtr]$win.Hwnd) -and -not $app.Proc.HasExited)
  Check 'and the example cancelled a second request' ((Get-Cancelled) -eq 2)

  Invoke-Click $app (ConvertTo-Screen $win (Get-TextCenter $view 'Allow')) 400
  Pause 0.8
  Click-CloseButton
  $exited = $app.Proc.WaitForExit(10000)
  Check 'with Allow, the close button closes the window and the app exits' $exited
  if ($exited) { Check 'with exit code 0' ($app.Proc.ExitCode -eq 0) "exit code $($app.Proc.ExitCode)" }
} catch {
  Check 'the scenario ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
} finally {
  if (-not $app.Proc.HasExited) { Stop-GuiApp $app | Out-Null }
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
