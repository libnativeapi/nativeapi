# Logged-on Windows desktop regression for window_manager #593/#596.
# Uses app-owned HWND messages/geometry. No keys; guarded mouse only with -AllowPointer.
param([switch]$DesktopSession,[string]$Version='local',
      [ValidateSet('Close','Resize','Both')][string]$Case='Both',
      [ValidateSet('Separate','Merged')][string]$ThreadPolicy='Merged',
      [int]$VetoCount=0,
      [switch]$ExpectCrash,[switch]$AllowPointer,[string]$RunDirectory)
$ErrorActionPreference='Stop'
$workspace=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$bundle=Join-Path $workspace ('bindings\dart\window_manager\example\build\windows-reports\'+$Version)
$exe=Join-Path $bundle 'window_manager_example.exe'
if(-not(Test-Path $exe)){throw "Build the fixture first: $exe"}
if(-not $DesktopSession){
  if(-not(Get-Process explorer -ErrorAction SilentlyContinue | Where-Object {$_.SessionId -ne 0})){throw 'No logged-on desktop'}
  $RunDirectory=Join-Path $workspace ('core\build\window-manager-reports-'+[Guid]::NewGuid().ToString('N'))
  New-Item -ItemType Directory -Force $RunDirectory | Out-Null
  $taskName='NativeAPI-WindowReports-'+[Guid]::NewGuid().ToString('N')
  $arguments='-NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File "'+$PSCommandPath+'" -DesktopSession -Version "'+$Version+'" -Case '+$Case+' -ThreadPolicy '+$ThreadPolicy+' -RunDirectory "'+$RunDirectory+'"'
  if($ExpectCrash){$arguments+=' -ExpectCrash'}
  if($AllowPointer){$arguments+=' -AllowPointer'}
  $arguments+=' -VetoCount '+$VetoCount
  $action=New-ScheduledTaskAction -Execute powershell.exe -Argument $arguments
  $principal=New-ScheduledTaskPrincipal -UserId ([System.Security.Principal.WindowsIdentity]::GetCurrent().User.Value) -LogonType Interactive -RunLevel Limited
  try{
    Register-ScheduledTask -TaskName $taskName -Action $action -Principal $principal -Settings (New-ScheduledTaskSettingsSet -ExecutionTimeLimit ([TimeSpan]::FromMinutes(5))) | Out-Null
    Start-ScheduledTask -TaskName $taskName
    $deadline=(Get-Date).AddSeconds(280)
    while(-not(Test-Path "$RunDirectory\result.json") -and (Get-Date) -lt $deadline){Start-Sleep -Milliseconds 250}
    if(-not(Test-Path "$RunDirectory\result.json")){throw "No result: $RunDirectory"}
    Get-Content "$RunDirectory\result.log"
    $result=Get-Content "$RunDirectory\result.json" -Raw | ConvertFrom-Json
    $resultExitCode=[int]$result.exitCode
    Write-Output ('RESULT_FILE '+$RunDirectory+'\result.json')
    $result | Select-Object version,threadPolicy,case,directory,exitCode,passed,failed,os | ConvertTo-Json -Depth 8
    $result.measurements | Select-Object mode,meanMs,p95Ms,maxMs,frames | ConvertTo-Json -Depth 3
    exit $resultExitCode
  }finally{
    Stop-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false -ErrorAction SilentlyContinue
  }
}
$RemoteScratch=$RunDirectory
. (Join-Path $workspace '.agents\skills\gui-test\scripts\windows\winput.ps1')
. (Join-Path $workspace '.agents\skills\gui-test\scripts\windows\guiapp.ps1')
Start-Result "$RunDirectory\result.log"
$summary=@{exitCode=1;version=$Version;case=$Case;threadPolicy=$ThreadPolicy;directory=$RunDirectory;session=(Get-Process -Id $PID).SessionId}
$env:NATIVEAPI_REPORT_THREAD_POLICY=$ThreadPolicy
$app=$null
try{
  # Recover only orphaned processes from this exact fixture bundle, never an app
  # with the same executable name elsewhere.
  Get-Process window_manager_example -ErrorAction SilentlyContinue | Where-Object {$_.Path -eq $exe} | Stop-Process
  Add-Type @'
using System;using System.Runtime.InteropServices;
public static class ReportWin {
 [StructLayout(LayoutKind.Sequential)] struct POINT{public int x,y;}
 [StructLayout(LayoutKind.Sequential)] struct RECT{public int l,t,r,b;}
 [StructLayout(LayoutKind.Sequential)] struct LASTINPUT{public uint size,time;}
 [StructLayout(LayoutKind.Sequential)] struct MOUSE{public int x,y;public uint data,flags,time;public IntPtr extra;}
 [StructLayout(LayoutKind.Sequential)] struct INPUT{public uint type;public MOUSE mouse;}
 [DllImport("user32.dll")] static extern IntPtr WindowFromPoint(POINT p);
 [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h,out uint process);
 [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")] static extern bool GetCursorPos(out POINT p);
 [DllImport("user32.dll")] static extern bool SetCursorPos(int x,int y);
 [DllImport("user32.dll")] static extern bool GetLastInputInfo(ref LASTINPUT i);
 [DllImport("user32.dll")] static extern short GetAsyncKeyState(int key);
 [DllImport("user32.dll")] static extern uint SendInput(uint count,INPUT[] input,int size);
 [DllImport("user32.dll")] static extern IntPtr SendMessage(IntPtr hwnd,uint message,IntPtr wp,IntPtr lp);
 [DllImport("dwmapi.dll")] static extern int DwmGetWindowAttribute(IntPtr h,int attribute,out RECT r,int size);
 [DllImport("user32.dll")] static extern bool PostMessage(IntPtr hwnd,uint message,IntPtr wp,IntPtr lp);
 [DllImport("user32.dll")] static extern bool SetWindowPos(IntPtr hwnd,IntPtr after,int x,int y,int w,int h,uint flags);
 // Match the caption close button. WM_CLOSE is deliberately a required close
 // notification in nativeapi and cannot be used to test a cancellable attempt.
 public static void Close(long hwnd){if(!PostMessage(new IntPtr(hwnd),0x112,new IntPtr(0xf060),IntPtr.Zero))throw new Exception("SC_CLOSE failed");}
 public static double Resize(long hwnd,int w,int h){var clock=System.Diagnostics.Stopwatch.StartNew();if(!SetWindowPos(new IntPtr(hwnd),IntPtr.Zero,0,0,w,h,0x16))throw new Exception("Resize failed");return clock.Elapsed.TotalMilliseconds;}
 static uint Stamp(){var i=new LASTINPUT{size=8};GetLastInputInfo(ref i);return i.time;}
 static void Guard(int process){uint actual;GetWindowThreadProcessId(GetForegroundWindow(),out actual);if(actual!=process)throw new Exception("Fixture lost foreground; stopping input");}
 static void Mouse(uint flags){var input=new INPUT{mouse=new MOUSE{flags=flags}};if(SendInput(1,new[]{input},Marshal.SizeOf(typeof(INPUT)))!=1)throw new Exception("Mouse injection failed");}
 static void Glide(int process,int x,int y,int ms){
   POINT from;GetCursorPos(out from);int px=from.x,py=from.y;uint stamp=Stamp();
   for(int i=1;i<=ms/20;i++){
     System.Threading.Thread.Sleep(20);POINT actual;GetCursorPos(out actual);
     if(actual.x!=px||actual.y!=py||Stamp()!=stamp)throw new Exception("User activity detected; stopping gesture");
     Guard(process);double t=(double)i/(ms/20);t=t*t*(3-2*t);
     px=(int)Math.Round(from.x+(x-from.x)*t);py=(int)Math.Round(from.y+(y-from.y)*t);
     SetCursorPos(px,py);Mouse(1);stamp=Stamp();
   }
 }
 public static void ResizeDrag(long hwnd,int process){
   Guard(process);if(GetAsyncKeyState(1)<0)throw new Exception("Mouse already pressed");
   RECT rect;if(DwmGetWindowAttribute(new IntPtr(hwnd),9,out rect,16)!=0)throw new Exception("No DWM frame");
   int x=0,y=0;bool found=false;
   for(int dy=-16;dy<=16&&!found;dy+=2)for(int dx=-16;dx<=16;dx+=2){
     int px=rect.r+dx,py=rect.b+dy;uint candidate;
     GetWindowThreadProcessId(WindowFromPoint(new POINT{x=px,y=py}),out candidate);
     if(candidate==process&&SendMessage(new IntPtr(hwnd),0x84,IntPtr.Zero,new IntPtr((py<<16)|(px&65535))).ToInt64()==17){x=px;y=py;found=true;break;}
   }
   if(!found)throw new Exception("No owned bottom-right resize hit target");
   Glide(process,x,y,500);
   uint owner;GetWindowThreadProcessId(WindowFromPoint(new POINT{x=x,y=y}),out owner);
   if(owner!=process)throw new Exception("Resize press is not owned by the fixture");
   if(SendMessage(new IntPtr(hwnd),0x84,IntPtr.Zero,new IntPtr((y<<16)|(x&65535))).ToInt64()!=17)throw new Exception("Not the bottom-right resize border");
   try{Mouse(2);Glide(process,x-140,y-100,700);Glide(process,x+80,y+60,1000);}finally{Mouse(4);}
 }
}
'@
  function Launch([string]$Mode,[string]$Name){
    $log=Join-Path $RunDirectory ($Name+'.stdout.log');$err=Join-Path $RunDirectory ($Name+'.stderr.log')
    $proc=Start-Process $exe -ArgumentList $Mode -WorkingDirectory $bundle -PassThru -RedirectStandardOutput $log -RedirectStandardError $err
    try{
    $proc.Handle | Out-Null
    $deadline=(Get-Date).AddSeconds(40)
    do{
      Start-Sleep -Milliseconds 100
      $output=[string](Get-Content $log -Raw)
      $match=[regex]::Match($output,'http://127\.0\.0\.1:\d+/[^/\s]+=/')
      if($proc.HasExited){throw "Fixture exited during startup: $err"}
    }until(($match.Success -and $output.Contains('READY ')) -or (Get-Date) -gt $deadline)
    if(-not $match.Success){throw 'No debug VM service'}
    $vm=$match.Value
    $isolate=((Invoke-RestMethod ($vm+'getVM')).result.isolates | Where-Object {$_.name -eq 'main'} | Select-Object -First 1).id
    $win=@(Get-AppWindows $proc.Id | Where-Object {$_.ClientW -gt 100})[0]
    if(-not $win){throw 'Missing fixture window'}
    Say ('START '+$Mode+' nativeModules='+(@($proc.Modules | Where-Object {$_.ModuleName -like '*nativeapi*'}).Count))
    return @{Proc=$proc;Log=$log;Error=$err;Win=$win;Url=($vm+'ext.nativeapi.windowsReport?isolateId='+[uri]::EscapeDataString($isolate))}
    }catch{Stop-Process -Id $proc.Id -ErrorAction SilentlyContinue;throw}
  }
  function Report($App,[string]$Action='state'){
    $reply=Invoke-RestMethod ($App.Url+'&action='+$Action) -TimeoutSec 10
    if($reply.error){throw ($reply.error | ConvertTo-Json -Depth 5)}
    return $reply.result
  }
  function Stop($App){if($App -and -not $App.Proc.HasExited){Stop-Process -Id $App.Proc.Id -ErrorAction SilentlyContinue}}
  $summary.os=Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber
  if($Case -ne 'Resize'){
    $repeat=if($ExpectCrash){1}else{3}
    for($trial=0;$trial -lt $repeat;$trial++){
      $app=Launch 'close' ('close-'+$trial)
      $null=Report $app
      for($count=1;$count -le $VetoCount;$count++){
        [ReportWin]::Close($app.Win.Hwnd);Start-Sleep -Milliseconds 500
        $state=Report $app
        Check "#593 trial=$trial cancelled close=$count stays alive" (-not $app.Proc.HasExited -and $state.closes -eq $count)
      }
      $null=Report $app 'approve'
      [ReportWin]::Close($app.Win.Hwnd)
      $exited=$app.Proc.WaitForExit(15000)
      $errorText=Get-Content $app.Error -Raw
      if($ExpectCrash){
        Check '#593 published baseline reproduces Dart isolate assertion' ($exited -and $app.Proc.ExitCode -ne 0 -and $errorText -match 'Dart_EnterIsolate') "exit=$($app.Proc.ExitCode)"
      }else{
        Check "#593 trial=$trial approved close exits normally" ($exited -and $app.Proc.ExitCode -eq 0) "exit=$($app.Proc.ExitCode)"
        Check "#593 trial=$trial no VM isolate assertion" ($errorText -notmatch 'Dart_EnterIsolate|FATAL|error:')
        Check "#593 trial=$trial approved retry invokes listener once" (@(Get-Content $app.Log | Where-Object {$_ -like 'CLOSE_CALLBACK *'}).Count -eq ($VetoCount+2))
      }
      Stop $app;$app=$null
    }
  }
  if($Case -ne 'Close'){
    if($AllowPointer){Assert-Idle}
    $measurements=@()
    foreach($mode in @('resize-control','resize-native','resize-listener')){
      $app=Launch $mode $mode
      Start-Sleep -Milliseconds 1000
      $null=Report $app 'reset'
      $durations=@()
      for($index=0;$index -lt 60;$index++){
        $durations += [ReportWin]::Resize($app.Win.Hwnd,900+($index%20)*5,620+($index%20)*3)
        Start-Sleep -Milliseconds 16
      }
      Start-Sleep -Milliseconds 1000
      $state=Report $app
      $sorted=@($durations | Sort-Object)
      $measurement=@{mode=$mode;meanMs=($durations | Measure-Object -Average).Average;p95Ms=$sorted[[int]($sorted.Count*0.95)];maxMs=$sorted[-1];frames=$state.frames;buildUs=$state.buildUs;rasterUs=$state.rasterUs;resizeMs=$durations}
      $measurements+=$measurement
      Say ('MEASURE '+([pscustomobject]$measurement | Select-Object mode,meanMs,p95Ms,maxMs,frames | ConvertTo-Json -Compress))
      Check "$mode renders while resized" ($state.frames -gt 10) "frames=$($state.frames)"
      if($AllowPointer){
        $win=@(Get-AppWindows $app.Proc.Id | Where-Object {$_.ClientW -gt 100})[0]
        Invoke-Activate $app $win
        $null=Report $app 'reset'
        [ReportWin]::ResizeDrag($win.Hwnd,$app.Proc.Id)
        Start-Sleep -Milliseconds 800
        $drag=Report $app
        $raster=@($drag.rasterUs | Sort-Object)
        $gesture=@{mode=$mode;frames=$drag.frames;rasterP95Us=$raster[[int]($raster.Count*0.95)];rasterMaxUs=$raster[-1];buildUs=$drag.buildUs;rasterUs=$drag.rasterUs}
        $summary['drag-'+$mode]=$gesture
        Say ('DRAG '+([pscustomobject]$gesture | Select-Object mode,frames,rasterP95Us,rasterMaxUs | ConvertTo-Json -Compress))
        Check "$mode renders throughout a real resize drag" ($drag.frames -gt 10) "frames=$($drag.frames)"
      }
      Stop $app;$app=$null
    }
    $summary.measurements=$measurements
  }
  $summary.exitCode=[int]($script:Failures -gt 0)
}catch{Say ('FAIL '+($_ | Out-String));$summary.exitCode=1}
finally{
  Stop $app
  $summary.passed=@(Get-Content "$RunDirectory\result.log" | Where-Object {$_ -cmatch ' PASS '}).Count
  $summary.failed=@(Get-Content "$RunDirectory\result.log" | Where-Object {$_ -cmatch ' FAIL '}).Count
  $summary | ConvertTo-Json -Depth 8 | Out-File "$RunDirectory\result.json" -Encoding utf8
}
exit $summary.exitCode
