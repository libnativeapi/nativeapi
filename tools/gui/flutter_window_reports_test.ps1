# Replays the window_manager reports carried over to nativeapi-core (#75-#78)
# in a real Flutter Windows app: tools/gui/window_reports_flutter.dart, built
# as a fixture against the workspace's Dart packages. Each scenario's steps
# are announced by the app and measured here: window geometry and styles,
# DWM caption buttons, and pixels of the app's solid colours (green
# background, amber panel, blue bottom bar). No input is sent.
#
#   $env:REPORTS_SCENARIOS = 'fullscreen'   # default: all
#   remote.sh win desktop tools/gui/flutter_window_reports_test.ps1 900
#
# $env:REPORTS_ARCHIVE: a .tar.gz with bindings/dart/{cnativeapi,nativeapi,
# nativeapi_flutter} and core/ at its root (tar keeps the packages' symlinks,
# which Copy-Item refuses); default $RemoteScratch\nativeapi-reports-src.tar.gz.
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
. "$PSScriptRoot\env.ps1"
. "$PSScriptRoot\winput.ps1"
. "$PSScriptRoot\guiapp.ps1"
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class ReportWin {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [StructLayout(LayoutKind.Sequential)] public struct MONITORINFO { public int cbSize; public RECT rcMonitor, rcWork; public uint dwFlags; }
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
  [DllImport("user32.dll", EntryPoint = "GetWindowLongPtrW")] static extern IntPtr GetLong(IntPtr h, int i);
  [DllImport("user32.dll")] static extern IntPtr GetWindow(IntPtr h, uint cmd);
  [DllImport("user32.dll")] static extern IntPtr MonitorFromWindow(IntPtr h, uint flags);
  [DllImport("user32.dll")] static extern bool GetMonitorInfo(IntPtr m, ref MONITORINFO i);
  [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr h, int a, out RECT r, int size);
  [DllImport("user32.dll")] static extern IntPtr GetDC(IntPtr h);
  [DllImport("user32.dll")] static extern int ReleaseDC(IntPtr h, IntPtr dc);
  [DllImport("gdi32.dll")] static extern uint GetPixel(IntPtr dc, int x, int y);
  public static int[] Rect(IntPtr h) { RECT r; GetWindowRect(h, out r); return new[] { r.L, r.T, r.R, r.B }; }
  // The rectangle the window paints, without invisible resize borders.
  public static int[] Frame(IntPtr h) {
    RECT r; if (DwmGetWindowAttribute(h, 9, out r, 16) != 0) GetWindowRect(h, out r);
    return new[] { r.L, r.T, r.R, r.B };
  }
  public static int[] Client(IntPtr h) {
    RECT r; GetClientRect(h, out r); POINT p = new POINT(); ClientToScreen(h, ref p);
    return new[] { p.X, p.Y, p.X + r.R, p.Y + r.B };
  }
  public static int[] Monitor(IntPtr h, bool work) {
    MONITORINFO i = new MONITORINFO(); i.cbSize = Marshal.SizeOf(typeof(MONITORINFO));
    GetMonitorInfo(MonitorFromWindow(h, 2), ref i);
    RECT r = work ? i.rcWork : i.rcMonitor; return new[] { r.L, r.T, r.R, r.B };
  }
  public static long Style(IntPtr h) { return GetLong(h, -16).ToInt64(); }
  public static long ExStyle(IntPtr h) { return GetLong(h, -20).ToInt64(); }
  public static bool HasOwner(IntPtr h) { return GetWindow(h, 4) != IntPtr.Zero; }
  // DWMWA_CAPTION_BUTTON_BOUNDS: empty without caption buttons.
  public static int CaptionButtonsWidth(IntPtr h) {
    RECT r; if (DwmGetWindowAttribute(h, 5, out r, 16) != 0) return -1; return r.R - r.L;
  }
  [DllImport("user32.dll")] static extern IntPtr SendMessageW(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll", EntryPoint = "GetClassLongPtrW")] static extern IntPtr GetClassLong(IntPtr h, int i);
  [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetWindowTextLength(IntPtr h);
  // WM_GETICON ICON_BIG / ICON_SMALL, then the class icon (GCLP_HICON).
  public static string Icons(IntPtr h) {
    return SendMessageW(h, 0x7F, (IntPtr)1, IntPtr.Zero).ToInt64() + "/" +
           SendMessageW(h, 0x7F, (IntPtr)0, IntPtr.Zero).ToInt64() + "/" + GetClassLong(h, -14).ToInt64();
  }
  public static int TitleLength(IntPtr h) { return GetWindowTextLength(h); }
  // 0xRRGGBB of a screen pixel.
  public static int Pixel(int x, int y) {
    IntPtr dc = GetDC(IntPtr.Zero); uint c = GetPixel(dc, x, y); ReleaseDC(IntPtr.Zero, dc);
    return (int)(((c & 0xFF) << 16) | (c & 0xFF00) | ((c >> 16) & 0xFF));
  }
}
"@

$green = 0x2E7D32; $amber = 0xFFC107; $blue = 0x1565C0
function Near([int]$c, [int]$want, [int]$tolerance = 24) {
  $d = 0
  foreach ($shift in 16, 8, 0) { $d = [Math]::Max($d, [Math]::Abs((($c -shr $shift) -band 255) - (($want -shr $shift) -band 255))) }
  $d -le $tolerance
}
function Hex([int]$c) { '#{0:X6}' -f $c }

# -- the fixture ----------------------------------------------------------------
$archive = if ($env:REPORTS_ARCHIVE) { $env:REPORTS_ARCHIVE } else { "$RemoteScratch\nativeapi-reports-src.tar.gz" }
$root = "$env:TEMP\nativeapi-window-reports"
$exe = "$root\app\build\windows\x64\runner\Debug\window_reports.exe"
$flutter = if ($env:FLUTTER) { $env:FLUTTER } else { "$env:USERPROFILE\fvm\versions\stable\bin\flutter.bat" }
function Build-Fixture {
  foreach ($d in "$root\bindings", "$root\core") { if (Test-Path $d) { Remove-Item -Recurse -Force $d } }
  New-Item -ItemType Directory -Force $root | Out-Null
  tar -xzf $archive -C $root bindings/dart/cnativeapi bindings/dart/nativeapi bindings/dart/nativeapi_flutter core 2>$null
  if (-not (Test-Path "$root\core\src")) { throw "Could not unpack $archive" }
  # A workspace member of the repository, not of this fixture.
  Remove-Item -Recurse -Force "$root\bindings\dart\nativeapi_flutter\example" -ErrorAction SilentlyContinue
  if (-not (Test-Path "$root\app\windows")) {
    cmd /c "`"$flutter`" create --no-pub --platforms=windows --empty --org=dev.nativeapi.test --project-name=window_reports `"$root\app`" > `"$root\create.log`" 2>&1"
  }
  Set-Content "$root\pubspec.yaml" -Encoding ascii -Value @"
name: nativeapi_window_reports_workspace
environment:
  sdk: ^3.13.0
workspace:
  - bindings/dart/cnativeapi
  - bindings/dart/nativeapi
  - bindings/dart/nativeapi_flutter
  - app
"@
  Set-Content "$root\app\pubspec.yaml" -Encoding ascii -Value @"
name: window_reports
publish_to: none
environment:
  sdk: ^3.13.0
resolution: workspace
dependencies:
  flutter:
    sdk: flutter
  nativeapi_flutter: any
flutter:
  assets:
    - assets/icon.ico
"@
  Copy-Item "$PSScriptRoot\window_reports_flutter.dart" "$root\app\lib\main.dart" -Force
  # window_manager#534 passed setIcon() a path inside flutter_assets.
  New-Item -ItemType Directory -Force "$root\app\assets" | Out-Null
  Copy-Item "$root\app\windows\runner\resources\app_icon.ico" "$root\app\assets\icon.ico" -Force
  Push-Location $root
  cmd /c "`"$flutter`" pub get > `"$root\build.log`" 2>&1"
  Pop-Location
  Push-Location "$root\app"
  cmd /c "`"$flutter`" build windows --debug >> `"$root\build.log`" 2>&1"
  $code = $LASTEXITCODE
  Pop-Location
  if ($code -ne 0 -or -not (Test-Path $exe)) {
    Get-Content "$root\build.log" | Select-String "error" | Select-Object -Last 15 | % { Say $_.Line }
    throw "The fixture did not build"
  }
}

# -- running a scenario ------------------------------------------------------------
function Start-Scenario([string]$Name) {
  $script:hand = "$root\handshake-$Name"
  if (Test-Path $hand) { Remove-Item -Recurse -Force $hand }
  New-Item -ItemType Directory -Force $hand | Out-Null
  $script:log = "$root\$Name.log"
  Remove-Item $log -ErrorAction SilentlyContinue
  $script:proc = Start-Process $exe -ArgumentList $Name, "`"$hand`"" -PassThru -RedirectStandardOutput $log
  $null = $proc.Handle
  $script:app = @{ Proc = $proc; Log = $log; Stdout = $null }
}
# Waits for `STEP <name>`, returns its state line; Go-Step lets the app go on.
function Wait-Step([string]$Name, [int]$TimeoutSeconds = 60) {
  $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    $line = Get-Content $log -ErrorAction SilentlyContinue | ? { $_ -like "STEP $Name *" -or $_ -like 'FAIL*' } | Select-Object -First 1
    if ($line -like 'FAIL*') { throw "The app failed: $line" }
    if ($line) { return $line }
    if ($proc.HasExited) { throw "The app exited before step $Name" }
    Start-Sleep -Milliseconds 50
  }
  throw "No step $Name"
}
function Go-Step([string]$Name) { New-Item -ItemType File -Force "$hand\go-$Name" | Out-Null }
# Raises and activates the app's window with a click on its own content, so
# no other window (nor the taskbar, over a full screen window) covers what the
# pixel checks read.
function Activate-App {
  $w = @(Get-AppWindows $proc.Id | ? { $_.ClientW -gt 100 })[0]
  Invoke-Activate $app $w 20
  Pause 0.5
}
function Get-AppWindow {
  $w = @(Get-AppWindows $proc.Id | ? { $_.ClientW -gt 100 }) | Select-Object -First 1
  if (-not $w) { throw 'The app window is missing' }
  $script:scale = $w.Scale
  [IntPtr]$w.Hwnd
}
# Screen points of the app's colour regions, from the window's client area.
function Get-Points([IntPtr]$H) {
  $c = [ReportWin]::Client($H)
  @{
    Panel = @([int](($c[0] + $c[2]) / 2), [int](($c[1] + $c[3] - 48 * $script:scale) / 2))
    Bar = @([int](($c[0] + $c[2]) / 2), ($c[3] - 6))
    Background = @(($c[0] + 12), ($c[1] + 12))
  }
}
function Stop-Scenario {
  $deadline = (Get-Date).AddSeconds(20)
  while (-not $proc.HasExited -and (Get-Date) -lt $deadline) {
    if (Get-Content $log -ErrorAction SilentlyContinue | ? { $_ -like 'STEP end *' }) { Go-Step 'end' }
    Start-Sleep -Milliseconds 100
  }
  if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force }
}

# Samples screen pixels for $Ms while the app performs step $Name: one
# array of colours per point, in time order.
function Watch-Step([string]$Name, $Points, [int]$Ms) {
  $samples = @($Points | % { ,(New-Object System.Collections.Generic.List[int]) })
  Go-Step $Name
  $end = (Get-Date).AddMilliseconds($Ms)
  while ((Get-Date) -lt $end) {
    for ($i = 0; $i -lt $Points.Count; $i++) { $samples[$i].Add([ReportWin]::Pixel($Points[$i][0], $Points[$i][1])) }
  }
  ,$samples
}
# EVENT lines the app printed between `STEP $From` and `STEP $To`.
function Get-Events([string]$From, [string]$To) {
  $lines = @(Get-Content $log)
  $a = [array]::FindIndex($lines, [Predicate[string]]{ param($l) $l -like "STEP $From *" })
  $b = [array]::FindIndex($lines, [Predicate[string]]{ param($l) $l -like "STEP $To *" })
  if ($a -lt 0 -or $b -lt 0) { return @() }
  @($lines[($a + 1)..($b - 1)] | ? { $_ -like 'EVENT *' } | % { $_.Substring(6) })
}
function Get-CaptionPoint([IntPtr]$H) {
  $f = [ReportWin]::Frame($H); $c = [ReportWin]::Client($H)
  @([int](($f[0] + $f[2]) / 2 - 150 * $script:scale), [int](($f[1] + $c[1]) / 2))
}

# -- #77 full screen ---------------------------------------------------------------
function Test-FullScreen {
  Say '== fullscreen (#77: window_manager#458, #579, #330)'
  Start-Scenario 'fullscreen'
  try {
    Wait-Step 'leave-fullscreen' | Out-Null
    $h = Get-AppWindow
    $mon = [ReportWin]::Monitor($h, $false)
    Check '#458 started in full screen: the window covers the monitor' `
      ((([ReportWin]::Frame($h)) -join ',') -eq ($mon -join ',')) "frame $(([ReportWin]::Frame($h)) -join ',') monitor $($mon -join ',')"
    Activate-App
    Go-Step 'leave-fullscreen'
    Wait-Step 'minimize' | Out-Null
    $frame = [ReportWin]::Frame($h)
    Check '#458 leaving full screen gives a window smaller than the monitor' `
      (($frame[2] - $frame[0]) -lt ($mon[2] - $mon[0]) -and ($frame[3] - $frame[1]) -lt ($mon[3] - $mon[1])) ($frame -join ',')
    $p = Get-Points $h
    $px = [ReportWin]::Pixel($p.Panel[0], $p.Panel[1])
    Check '#458 the content is painted, not only the background colour' (Near $px $amber) "panel pixel $(Hex $px)"
    $ex = [ReportWin]::ExStyle($h)
    Check '#458 it keeps a taskbar button (visible, unowned, not a tool window)' `
      ([ReportWin]::IsWindowVisible($h) -and -not [ReportWin]::HasOwner($h) -and -not ($ex -band 0x80)) ('exstyle 0x{0:X}' -f $ex)
    $style = [ReportWin]::Style($h)
    Check '#579 the title bar is back: WS_CAPTION' (($style -band 0x00C00000) -eq 0x00C00000) ('style 0x{0:X}' -f $style)
    Check '#579 the title bar is back: caption buttons' ([ReportWin]::CaptionButtonsWidth($h) -gt 0) "width $([ReportWin]::CaptionButtonsWidth($h))"
    Go-Step 'minimize'
    Wait-Step 'restore' | Out-Null
    Check '#458 minimizing works after leaving full screen' ([ReportWin]::IsIconic($h))
    Go-Step 'restore'
    Wait-Step 'enter-fullscreen' | Out-Null
    Check 'restored from minimized' (-not [ReportWin]::IsIconic($h))
    Go-Step 'enter-fullscreen'
    Wait-Step 'leave-fullscreen-again' | Out-Null
    $mon = [ReportWin]::Monitor($h, $false)
    $rows = @(2, 8, 20, 40) | % { [ReportWin]::Pixel([int](($mon[0] + $mon[2]) / 2), ($mon[3] - $_)) }
    Check '#330 in full screen the bottom shows the current bar, not old content' (@($rows | ? { -not (Near $_ $blue) }).Count -eq 0) (($rows | % { Hex $_ }) -join ' ')
    Go-Step 'leave-fullscreen-again'
    Wait-Step 'end' | Out-Null
    $p = Get-Points $h
    $c = [ReportWin]::Client($h)
    $rows = @(2, 8, 20, 40) | % { [ReportWin]::Pixel($p.Bar[0], ($c[3] - $_)) }
    Check '#330 after full screen the bottom shows the current bar' (@($rows | ? { -not (Near $_ $blue) }).Count -eq 0) (($rows | % { Hex $_ }) -join ' ')
    $style = [ReportWin]::Style($h)
    Check '#579 the title bar is back again' ((($style -band 0x00C00000) -eq 0x00C00000) -and [ReportWin]::CaptionButtonsWidth($h) -gt 0)
  } catch {
    Check 'fullscreen ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
  } finally { Stop-Scenario }
}

# -- #76 hidden title bar ----------------------------------------------------------
function Test-Hidden {
  Say '== hidden (#76: window_manager#554, #450, #378, #397, #547)'
  Start-Scenario 'hidden'
  try {
    Wait-Step 'maximize' | Out-Null
    $h = Get-AppWindow
    Activate-App
    $f = [ReportWin]::Frame($h)
    # Windows 11 draws a semi-transparent border: it darkens whatever is behind
    # the window, so compare geometry and change, not colours, between edges.
    $c = [ReportWin]::Client($h)
    $cx = [int](($f[0] + $f[2]) / 2); $cy = [int](($f[1] + $f[3]) / 2)
    $outsideTop = [ReportWin]::Pixel($cx, $f[1] - 1); $top = [ReportWin]::Pixel($cx, $f[1])
    $outsideLeft = [ReportWin]::Pixel($f[0] - 1, $cy); $left = [ReportWin]::Pixel($f[0], $cy)
    Say "  #547 top: outside $(Hex $outsideTop) edge $(Hex $top); left: outside $(Hex $outsideLeft) edge $(Hex $left); insets top $($c[1] - $f[1]) left $($c[0] - $f[0])"
    # The visible border: pixels from the edge inwards until the content.
    $topBorder = 0; while ($topBorder -lt 6 -and -not (Near ([ReportWin]::Pixel($cx, $f[1] + $topBorder)) $green)) { $topBorder++ }
    $leftBorder = 0; while ($leftBorder -lt 6 -and -not (Near ([ReportWin]::Pixel($f[0] + $leftBorder, $cy)) $green)) { $leftBorder++ }
    Check '#547 the top has a border as thick as the left one' `
      ($topBorder -gt 0 -and $topBorder -eq $leftBorder) "top $topBorder px, left $leftBorder px"
    $bmp = New-Object System.Drawing.Bitmap 160, 160
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen(($f[0] - 20), ($f[1] - 20), 0, 0, $bmp.Size)
    $bmp.Save("$RemoteScratch\flutter_window_reports_hidden_corner.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Go-Step 'maximize'
    Wait-Step 'restore' | Out-Null
    $work = [ReportWin]::Monitor($h, $true); $c = [ReportWin]::Client($h)
    Check '#554 maximized, it leaves the taskbar uncovered' ($c[3] -le $work[3]) "client $($c -join ',') work $($work -join ',')"
    Check '#450 maximized, its content fills the work area exactly' (($c -join ',') -eq ($work -join ',')) "client $($c -join ',') work $($work -join ',')"
    $px = [ReportWin]::Pixel([int](($work[0] + $work[2]) / 2), ($work[3] - 3))
    Check '#450 the bottom bar reaches the taskbar' (Near $px $blue) (Hex $px)
    Go-Step 'restore'
    Wait-Step 'enter-fullscreen' | Out-Null
    $restored = [ReportWin]::Client($h)
    Go-Step 'enter-fullscreen'
    Wait-Step 'leave-fullscreen' | Out-Null
    $mon = [ReportWin]::Monitor($h, $false); $c = [ReportWin]::Client($h)
    Check '#378 in full screen the content covers the monitor' (($c -join ',') -eq ($mon -join ',')) "client $($c -join ',') monitor $($mon -join ',')"
    $corner = [ReportWin]::Pixel(($mon[0] + 2), ($mon[1] + 2)); $bottom = [ReportWin]::Pixel([int](($mon[0] + $mon[2]) / 2), ($mon[3] - 3))
    Check '#378 no padding: the corner is content, the bottom is the bar' ((Near $corner $green) -and (Near $bottom $blue)) "corner $(Hex $corner) bottom $(Hex $bottom)"
    # A caption flashing on the way out would show above the restored content.
    $probe = ,@([int](($restored[0] + $restored[2]) / 2), ($restored[1] + 6))
    $samples = Watch-Step 'leave-fullscreen' $probe 1200
    $odd = @($samples[0] | ? { -not (Near $_ $green 40) } | Sort-Object -Unique)
    Check '#397 leaving full screen shows no title bar on the way' ($odd.Count -eq 0) "$($samples[0].Count) samples, odd: $(($odd | % { Hex $_ }) -join ' ')"
    Wait-Step 'end' | Out-Null
    $c = [ReportWin]::Client($h)
    Check 'and comes back to the restored size' (($c -join ',') -eq ($restored -join ',')) "client $($c -join ',') before $($restored -join ',')"
  } catch {
    Check 'hidden ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
  } finally { Stop-Scenario }
}

function Test-Transparent {
  Say '== transparent (#76: window_manager#576)'
  Start-Scenario 'transparent'
  try {
    Wait-Step 'look' | Out-Null
    $h = Get-AppWindow
    Activate-App
    Check '#576 with a transparent background the caption buttons are there' ([ReportWin]::CaptionButtonsWidth($h) -gt 0) "width $([ReportWin]::CaptionButtonsWidth($h))"
    Check '#576 and the title' ([ReportWin]::TitleLength($h) -gt 0)
    $f = [ReportWin]::Frame($h)
    $bmp = New-Object System.Drawing.Bitmap ($f[2] - $f[0]), 120
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($f[0], $f[1], 0, 0, $bmp.Size)
    $bmp.Save("$RemoteScratch\flutter_window_reports_transparent.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Go-Step 'look'
  } catch {
    Check 'transparent ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
  } finally { Stop-Scenario }
}

# -- #78 events, input and icon ----------------------------------------------------
function Test-Events {
  Say '== events (#78: window_manager#294, #207, #511, #534)'
  Start-Scenario 'events'
  try {
    Wait-Step 'minimize' | Out-Null
    $h = Get-AppWindow
    Activate-App
    Go-Step 'minimize'
    Wait-Step 'restore' | Out-Null
    Go-Step 'restore'
    Wait-Step 'double-click-caption' | Out-Null
    $after = Get-Events 'restore' 'double-click-caption'
    $focus = @($after | ? { $_ -eq 'Focused' -or $_ -eq 'Blurred' })
    Check '#294 restoring a minimized window reports focus, last' ($focus.Count -gt 0 -and $focus[-1] -eq 'Focused') ($after -join ' ')
    $caption = Get-CaptionPoint $h
    Invoke-DoubleClick $app $caption 400
    Pause 1.2
    Go-Step 'double-click-caption'
    Wait-Step 'double-click-caption-again' | Out-Null
    $after = Get-Events 'double-click-caption' 'double-click-caption-again'
    Check '#207 double-clicking the title bar maximizes' ([ReportWin]::IsZoomed($h))
    Check '#207 and reports Resized and Maximized' (($after -contains 'Resized') -and ($after -contains 'Maximized')) ($after -join ' ')
    $caption = Get-CaptionPoint $h
    Invoke-DoubleClick $app $caption 400
    Pause 1.2
    Go-Step 'double-click-caption-again'
    Wait-Step 'double-tap-panel' | Out-Null
    $after = Get-Events 'double-click-caption-again' 'double-tap-panel'
    Check '#207 double-clicking it again restores, with Resized and Restored' ((-not [ReportWin]::IsZoomed($h)) -and ($after -contains 'Resized') -and ($after -contains 'Restored')) ($after -join ' ')
    $p = Get-Points $h
    Invoke-DoubleClick $app $p.Panel 400
    Pause 1.2
    Check '#511 maximize() from onDoubleTapDown, with the pointer captured, maximizes' ([ReportWin]::IsZoomed($h)) "tap seen: $([bool](Get-Content $log | ? { $_ -eq 'TAP double-down' }))"
    Go-Step 'double-tap-panel'
    Wait-Step 'restore-after-tap' | Out-Null
    Go-Step 'restore-after-tap'
    Wait-Step 'set-icon' | Out-Null
    $before = [ReportWin]::Icons($h)
    Go-Step 'set-icon'
    Wait-Step 'end' | Out-Null
    $now = [ReportWin]::Icons($h)
    $line = Get-Content $log | ? { $_ -like 'ICON *' } | Select-Object -First 1
    Check '#534 setIcon() with a path inside flutter_assets succeeds' ($line -eq 'ICON setIcon=true') $line
    Check '#534 and changes the window icon' (($now -ne $before) -and -not ($now.Split('/')[0] -eq '0' -and $now.Split('/')[1] -eq '0')) "icons before $before after $now"
  } catch {
    Check 'events ran to the end' $false "$($_.Exception.Message) at line $($_.InvocationInfo.ScriptLineNumber)"
  } finally { Stop-Scenario }
}

Start-Result "$RemoteScratch\flutter_window_reports_test.result.txt"
Assert-Idle
if ($env:REPORTS_REBUILD -ne '0' -or -not (Test-Path $exe)) { Build-Fixture }
$wanted = if ($env:REPORTS_SCENARIOS) { $env:REPORTS_SCENARIOS.Split(',') } else { @('fullscreen', 'hidden', 'transparent', 'events') }
foreach ($s in $wanted) {
  switch ($s) {
    'fullscreen' { Test-FullScreen }
    'hidden' { Test-Hidden }
    'transparent' { Test-Transparent }
    'events' { Test-Events }
    default { Check "known scenario $s" $false }
  }
}
Say "$(if ($script:Failures) { 'FAILED' } else { 'OK' })"
