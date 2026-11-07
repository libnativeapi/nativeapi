# Core #76/#78: run the fixture in the logged-on desktop. Keyboard input is
# opt-in, requires user approval, and is guarded by foreground/focus ownership.
param([switch]$DesktopSession, [switch]$AllowKeyboard, [switch]$AllowPointer, [string]$RunDirectory,
      [string]$ExecutablePath)
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$workspace = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $ExecutablePath) {
  $ExecutablePath = Join-Path $workspace 'examples\flutter_window_title_bar_example\build\windows\x64\runner\Debug\window_title_bar_example.exe'
}
if (-not (Test-Path $ExecutablePath)) { throw "Build the remaining-reports Flutter fixture first: $ExecutablePath" }

if (-not $DesktopSession) {
  if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $_.SessionId -ne 0 })) {
    throw 'No logged-on desktop session'
  }
  if (-not $RunDirectory) { $RunDirectory = Join-Path $workspace ('core\build\reports76-78-' + [Guid]::NewGuid().ToString('N')) }
  New-Item -ItemType Directory -Force $RunDirectory | Out-Null
  $taskName = 'NativeAPI-Reports76-78-' + [Guid]::NewGuid().ToString('N')
  $arguments = '-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -DesktopSession -RunDirectory "' + $RunDirectory + '" -ExecutablePath "' + $ExecutablePath + '"'
  if ($AllowKeyboard) { $arguments += ' -AllowKeyboard' }
  if ($AllowPointer) { $arguments += ' -AllowPointer' }
  $action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument $arguments
  $principal = New-ScheduledTaskPrincipal -UserId ([System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value) -LogonType Interactive -RunLevel Limited
  $settings = New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::FromMinutes(5)) -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries
  try {
    Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal -Settings $settings | Out-Null
    Start-ScheduledTask -TaskName $taskName
    $deadline = (Get-Date).AddSeconds(280)
    $resultFile = Join-Path $RunDirectory 'result.json'
    while (-not (Test-Path $resultFile) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 250 }
    if (-not (Test-Path $resultFile)) { throw "No result: $RunDirectory" }
    Get-Content (Join-Path $RunDirectory 'result.log')
    $result = Get-Content $resultFile -Raw | ConvertFrom-Json
    $result | ConvertTo-Json -Depth 8
    exit $result.exitCode
  } finally {
    Stop-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
  }
}

$RemoteScratch = $RunDirectory
. (Join-Path $workspace '.agents\skills\gui-test\scripts\windows\winput.ps1')
. (Join-Path $workspace '.agents\skills\gui-test\scripts\windows\guiapp.ps1')
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class RemainingWin {
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int l,t,r,b; }
 [StructLayout(LayoutKind.Sequential)] struct POINT {public int x,y;}
 [StructLayout(LayoutKind.Sequential)] struct GUI {public int size; public uint flags; public IntPtr active,focus,capture,menu,move,caret; public RECT rect;}
 [StructLayout(LayoutKind.Sequential)] struct LASTINPUT {public uint size,time;}
 [StructLayout(LayoutKind.Sequential)] struct KEYBOARD {public ushort vk,scan; public uint flags,time; public IntPtr extra;}
 [StructLayout(LayoutKind.Sequential)] struct MOUSE {public int x,y;public uint data,flags,time;public IntPtr extra;}
 [StructLayout(LayoutKind.Explicit,Size=32)] struct UNION {[FieldOffset(0)] public KEYBOARD key;[FieldOffset(0)]public MOUSE mouse;}
 [StructLayout(LayoutKind.Sequential)] struct INPUT {public uint type; public UNION data;}
 [StructLayout(LayoutKind.Sequential)] struct MONITOR {public int size;public RECT monitor,work;public uint flags;}
 [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll")] static extern bool GetGUIThreadInfo(uint thread,ref GUI info);
 [DllImport("user32.dll")] static extern bool IsChild(IntPtr parent,IntPtr child);
 [DllImport("user32.dll")] static extern uint SendInput(uint count,INPUT[] inputs,int size);
 [DllImport("user32.dll")] static extern uint MapVirtualKey(uint code,uint kind);
 [DllImport("user32.dll")] static extern short GetAsyncKeyState(int key);
 [DllImport("user32.dll")] static extern bool GetLastInputInfo(ref LASTINPUT value);
 [DllImport("user32.dll")] static extern bool GetCursorPos(out POINT point);
 [DllImport("user32.dll")] static extern bool SetCursorPos(int x,int y);
 [DllImport("user32.dll")] static extern IntPtr WindowFromPoint(POINT point);
 [DllImport("user32.dll")] static extern bool GetMonitorInfo(IntPtr monitor,ref MONITOR info);
 [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr window,out RECT rect);
 [DllImport("user32.dll")] static extern bool ClientToScreen(IntPtr window,ref POINT point);
 [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr window,int attr,out RECT rect,int size);
 [DllImport("user32.dll")] static extern IntPtr GetDC(IntPtr h);
 [DllImport("user32.dll")] static extern int ReleaseDC(IntPtr h,IntPtr dc);
 [DllImport("gdi32.dll")] static extern uint GetPixel(IntPtr dc,int x,int y);
 public static uint LastInput() {var l=new LASTINPUT{size=(uint)Marshal.SizeOf(typeof(LASTINPUT))};GetLastInputInfo(ref l);return l.time;}
 public static void Guard(long hwnd,int process) {
   var root=new IntPtr(hwnd);uint p;
   var active=GetForegroundWindow();var thread=GetWindowThreadProcessId(active,out p);
   var info=new GUI{size=Marshal.SizeOf(typeof(GUI))};
   if(active!=root||p!=process||!GetGUIThreadInfo(thread,ref info)||info.focus==IntPtr.Zero||
       (info.focus!=root&&!IsChild(root,info.focus))) throw new Exception("Foreground/focus is not the fixture; no key sent");
   GetWindowThreadProcessId(info.focus,out p);
   if(p!=process) throw new Exception("Keyboard focus belongs to another process");
 }
 public static void Key(long hwnd,int process,int vk) {
   if(vk!=13&&vk!=27) throw new Exception("Only Enter/Escape are allowed");
   Guard(hwnd,process);
   foreach(int modifier in new[]{16,17,18,91,92}) if(GetAsyncKeyState(modifier)<0) throw new Exception("A modifier is held; stopping");
   POINT before,after;GetCursorPos(out before);
   var input=new INPUT{type=1,data=new UNION{key=new KEYBOARD{vk=(ushort)vk,scan=(ushort)MapVirtualKey((uint)vk,0)}}};
   try {if(SendInput(1,new[]{input},Marshal.SizeOf(typeof(INPUT)))!=1) throw new Exception("Key down failed");}
   finally {input.data.key.flags=2;SendInput(1,new[]{input},Marshal.SizeOf(typeof(INPUT)));}
   GetCursorPos(out after);
   if(before.x!=after.x||before.y!=after.y) throw new Exception("Mouse activity detected; stopping");
   Guard(hwnd,process);
 }
 public static int[] Work(long handle) {var m=new MONITOR{size=Marshal.SizeOf(typeof(MONITOR))};if(!GetMonitorInfo(new IntPtr(handle),ref m))throw new Exception("Missing monitor");return new[]{m.work.l,m.work.t,m.work.r,m.work.b};}
 public static int[] Frame(long handle) {RECT r;if(DwmGetWindowAttribute(new IntPtr(handle),9,out r,16)!=0)throw new Exception("Missing DWM frame");return new[]{r.l,r.t,r.r,r.b};}
 public static int Pixel(int x,int y) {var dc=GetDC(IntPtr.Zero);var c=GetPixel(dc,x,y);ReleaseDC(IntPtr.Zero,dc);return (int)(((c&255)<<16)|(c&65280)|((c>>16)&255));}
 static void Mouse(uint flags) {
   var input=new INPUT{data=new UNION{mouse=new MOUSE{flags=flags}}};
   if(SendInput(1,new[]{input},Marshal.SizeOf(typeof(INPUT)))!=1)throw new Exception("Mouse input failed");
 }
 static void Wait(long hwnd,int process,int x,int y,uint stamp,int ms) {
   System.Threading.Thread.Sleep(ms);POINT p;GetCursorPos(out p);
   if(p.x!=x||p.y!=y||LastInput()!=stamp)throw new Exception("User activity during gesture; stopping");
   Guard(hwnd,process);
 }
 static void Glide(long hwnd,int process,int x,int y,int ms) {
   POINT p;GetCursorPos(out p);uint stamp=LastInput();int px=p.x,py=p.y;
   for(int i=1;i<=ms/20;i++) {
     Wait(hwnd,process,px,py,stamp,20);
     double t=(double)i/(ms/20);t=t*t*(3-2*t);
     px=(int)Math.Round(p.x+(x-p.x)*t);py=(int)Math.Round(p.y+(y-p.y)*t);
     SetCursorPos(px,py);Mouse(1);stamp=LastInput();
   }
 }
 public static void Drag(long hwnd,int process,int x,int y,int endX,int endY) {
   Guard(hwnd,process);
   if(GetAsyncKeyState(1)<0)throw new Exception("Mouse button already held");
   Glide(hwnd,process,x,y,500);
   uint owner;GetWindowThreadProcessId(WindowFromPoint(new POINT{x=x,y=y}),out owner);
   if(owner!=process)throw new Exception("Press point is not owned by fixture");
   Guard(hwnd,process);
   try {
     Mouse(2);Wait(hwnd,process,x,y,LastInput(),150);
     int dx=endX-x,dy=endY-y;double distance=Math.Sqrt((double)dx*dx+(double)dy*dy);
     double fraction=Math.Min(1,60/Math.Max(1,distance));
     Glide(hwnd,process,x+(int)(dx*fraction),y+(int)(dy*fraction),400);
     Glide(hwnd,process,endX,endY,1000);
   } finally {Mouse(4);}
 }
}
'@

Start-Result (Join-Path $RunDirectory 'result.log')
$app = $null
$summary = @{session=(Get-Process -Id $PID).SessionId;exitCode=1;keyboard=$AllowKeyboard.IsPresent;pointer=$AllowPointer.IsPresent;mixedDpi=$false;directory=$RunDirectory}
try {
  Assert-Idle
  $app = Start-GuiApp $ExecutablePath
  $app.Proc.Handle | Out-Null
  $deadline = (Get-Date).AddSeconds(40)
  do {
    $match = [regex]::Match((Get-Content $app.Log -Raw),'http://127\.0\.0\.1:\d+/[^/\s]+=/')
    if (-not $match.Success) { Start-Sleep -Milliseconds 200 }
  } until ($match.Success -or (Get-Date) -gt $deadline)
  if (-not $match.Success) { throw 'No debug VM service' }
  $vm = $match.Value
  $isolate = ((Invoke-RestMethod ($vm+'getVM')).result.isolates | Where-Object {$_.name -eq 'main'} | Select-Object -First 1).id
  function Report([string]$Action='state',[string]$Parameters='') {
    $url = $vm+'ext.nativeapi.windowReport?isolateId='+[uri]::EscapeDataString($isolate)+'&action='+$Action+$Parameters
    $reply=Invoke-RestMethod $url -TimeoutSec 15
    if($reply.error){throw ($reply.error | ConvertTo-Json -Depth 8)}
    return $reply.result
  }
  $s=Report
  $h=[long]$s.hwnd
  $w=@(Get-AppWindows $app.Proc.Id | Where-Object {$_.Hwnd -eq $h})[0]
  Invoke-Activate $app $w
  $script:inputStamp=[RemainingWin]::LastInput()
  function Guard-User {
    if([RemainingWin]::LastInput() -ne $script:inputStamp){throw 'User input detected; stopping before the next key or gesture'}
  }
  $summary.os=(Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber)
  $summary.displays=$s.displays
  Say ('Environment: '+($summary.os | ConvertTo-Json -Compress))
  Say ('Displays: '+($s.displays | ConvertTo-Json -Depth 5 -Compress))

  foreach($style in @('normal','hidden')) {
    $s=Report 'style' ('&style='+$style)
    if($style -eq 'hidden') {
      [RemainingWin]::Guard($h,$app.Proc.Id)
      $f=[RemainingWin]::Frame($h);$cx=[int](($f[0]+$f[2])/2);$cy=[int](($f[1]+$f[3])/2)
      function Is-Green([int]$Color) {return ([Math]::Abs((($Color -shr 16) -band 255)-46) -lt 25 -and [Math]::Abs((($Color -shr 8) -band 255)-125) -lt 25 -and [Math]::Abs(($Color -band 255)-50) -lt 25)}
      $top=0;while($top -lt 8 -and -not(Is-Green ([RemainingWin]::Pixel($cx,$f[1]+$top)))){$top++}
      $left=0;while($left -lt 8 -and -not(Is-Green ([RemainingWin]::Pixel($f[0]+$left,$cy)))){$left++}
      Check '#76 hidden title bar retains the top border' ($top -gt 0 -and $top -lt 8 -and $top -eq $left) "top=$top px left=$left px"
    }
    if($AllowKeyboard) {
      foreach($kind in @('hide','minimize')) {
        foreach($keyName in @('enter','escape')) {
          Guard-User
          $s=Report 'prepare'
          $s=Report 'cycle' ('&kind='+$kind)
          Check "$style/$kind actually hides or minimizes" $(if($kind -eq 'hide'){$s.wasHidden}else{$s.wasMinimized})
          Check "$style/$kind restores TextField focus" ($s.focused -and $s.textFocused)
          $vk=if($keyName -eq 'enter'){13}else{27}
          for($count=1;$count -le 2;$count++) {
            Guard-User
            [RemainingWin]::Key($h,$app.Proc.Id,$vk)
            $script:inputStamp=[RemainingWin]::LastInput()
            Start-Sleep -Milliseconds 100
            Guard-User
            $s=Report
            $down=@($s.keys | Where-Object {$_ -eq ($keyName+':down')}).Count
            $up=@($s.keys | Where-Object {$_ -eq ($keyName+':up')}).Count
            Check "$style/$kind $keyName press $count arrives without being swallowed" ($down -eq $count -and $up -eq $count) "down=$down up=$up"
            if($keyName -eq 'enter'){Check "$style/$kind TextField submission $count" ($s.submitted -eq $count) "submitted=$($s.submitted)"}
          }
        }
      }
    }
  }
  if(-not $AllowKeyboard){Say 'SKIP keyboard input: pass -AllowKeyboard only with user approval'}
  $s=Report 'style' '&style=normal'
  $s=Report 'size'
  $displays=@($s.displays)
  if($displays.Count -lt 2 -or @($displays.scale | Select-Object -Unique).Count -lt 2) {
    Say 'SKIP #78 mixed-DPI move: two monitors with different scale factors are required'
  }else{
    $summary.mixedDpi=$true
    foreach($style in @('normal','hidden')) {
      $s=Report 'style' ('&style='+$style)
      foreach($resizable in @('true','false')) {
        $s=Report 'size' ('&resizable='+$resizable)
        for($trip=0;$trip -lt 2;$trip++){
          for($index=0;$index -lt $displays.Count;$index++){
            Guard-User
            $s=Report 'move' ('&index='+$index)
            $target=$displays[$index]
            $label="#78 $style resizable=$resizable trip=$trip monitor=$index"
            Check "$label Flutter DPR follows the target" ([Math]::Abs($s.dpr-$target.scale) -lt 0.01) "dpr=$($s.dpr) target=$($target.scale)"
            Check "$label preserves logical client size" ([Math]::Abs($s.content[2]-320) -le 2 -and [Math]::Abs($s.content[3]-460) -le 2) ($s.content -join ',')
            Check "$label Flutter layout preserves logical size" ([Math]::Abs($s.view[0]-320) -le 2 -and [Math]::Abs($s.view[1]-460) -le 2) ($s.view -join ',')
          }
        }
      }
    }
    if($AllowPointer) {
      $Python='python'
      Copy-Item (Join-Path $workspace '.agents\skills\flutter-ui-probe\scripts\uiprobe.py') (Join-Path $RunDirectory 'uiprobe.py')
      $s=Report 'style' '&style=hidden'
      $s=Report 'size' '&resizable=false'
      $s=Report 'move' '&index=0'
      for($trip=0;$trip -lt 2;$trip++) {
        foreach($index in @(1,0)) {
          Guard-User
          $views=@(Get-Views $app)
          $view=Find-View $views 'Drag between displays'
          if(-not $view){throw 'Drag handle was not found in the Flutter render tree'}
          $w=@(Get-AppWindows $app.Proc.Id | Where-Object {$_.Hwnd -eq $h})[0]
          $start=ConvertTo-Screen $w (Get-TextCenter $view 'Drag between displays')
          $target=$displays[$index];$work=[RemainingWin]::Work([long]$target.handle)
          $endX=$work[0]+[int](180*$target.scale);$endY=$work[1]+[int](120*$target.scale)
          [RemainingWin]::Drag($h,$app.Proc.Id,$start[0],$start[1],$endX,$endY)
          $script:inputStamp=[RemainingWin]::LastInput()
          Start-Sleep -Milliseconds 1000
          Guard-User
          $s=Report
          $label="#78 real drag trip=$trip monitor=$index hidden/fixed"
          Check "$label reaches target DPR" ([Math]::Abs($s.dpr-$target.scale) -lt 0.01) "dpr=$($s.dpr) target=$($target.scale)"
          Check "$label preserves client and Flutter layout size" ([Math]::Abs($s.content[2]-320) -le 2 -and [Math]::Abs($s.content[3]-460) -le 2 -and [Math]::Abs($s.view[0]-320) -le 2 -and [Math]::Abs($s.view[1]-460) -le 2) "client=$($s.content -join ',') view=$($s.view -join ',')"
        }
      }
    }else{Say 'SKIP real cross-monitor dragging: pass -AllowPointer with user approval'}
  }
  $s=Report
  Check 'Flutter reports no layout or framework errors' (@($s.errors).Count -eq 0) ($s.errors -join '; ')
  $s | ConvertTo-Json -Depth 8 | Out-File (Join-Path $RunDirectory 'fixture-state.json') -Encoding utf8
  $summary.exitCode=[int]($script:Failures -gt 0)
} catch {
  Say ('FAIL '+($_ | Out-String))
  $summary.exitCode=1
} finally {
  if($app){Stop-GuiApp $app | Out-Null}
  $summary.passed=@(Get-Content (Join-Path $RunDirectory 'result.log') | Where-Object {$_ -cmatch ' PASS '}).Count
  $summary.failed=@(Get-Content (Join-Path $RunDirectory 'result.log') | Where-Object {$_ -cmatch ' FAIL '}).Count
  $summary | ConvertTo-Json -Depth 8 | Out-File (Join-Path $RunDirectory 'result.json') -Encoding utf8
}
exit $summary.exitCode
