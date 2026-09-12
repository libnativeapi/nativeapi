param([string]$Destination = "$PSScriptRoot/../build/winui3-packages")
$ErrorActionPreference = 'Stop'
$packages = @(
  @{ Id = 'microsoft.windowsappsdk'; Version = '1.6.250108002'; Directory = 'winappsdk'; Hash = 'C01C03EF5ADB65D9BE4769311E45A53F03749432FDDB235173824B729C304380' },
  @{ Id = 'microsoft.windows.cppwinrt'; Version = '2.0.240405.15'; Directory = 'cppwinrt'; Hash = 'E889007B5D9235931E7340DDF737D2C346EEBDD23C619F1F4F2426A2AAE47180' },
  @{ Id = 'microsoft.web.webview2'; Version = '1.0.2903.40'; Directory = 'webview2'; Hash = 'EF128016DD1E51C59178C827ED5B8AA3322C57AFA8675D930F8109505542AD74' }
)
New-Item -ItemType Directory -Path $Destination -Force | Out-Null
$Destination = (Resolve-Path -LiteralPath $Destination).Path
foreach ($package in $packages) {
  $archive = Join-Path $Destination "$($package.Directory).zip"
  if (!(Test-Path -LiteralPath $archive) -or
      (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $package.Hash) {
    $url = "https://api.nuget.org/v3-flatcontainer/$($package.Id)/$($package.Version)/$($package.Id).$($package.Version).nupkg"
    Invoke-WebRequest -Uri $url -OutFile $archive
  }
  if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $package.Hash) {
    throw "Package hash mismatch: $($package.Id)"
  }
  Expand-Archive -LiteralPath $archive -DestinationPath (Join-Path $Destination $package.Directory) -Force
}
Write-Output "WinUI3 build packages restored to $Destination"
Write-Output 'Install Windows App Runtime 1.6 separately on machines running the modern backend.'
