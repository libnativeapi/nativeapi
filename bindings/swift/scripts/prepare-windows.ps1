param()
$ErrorActionPreference = 'Stop'

# Git can check a symlink out as a text file when core.symlinks is false.
# SwiftPM then silently discovers no core sources and fails at the umbrella
# header. Repair only this known link; never touch the core checkout itself.
$packageRoot = Split-Path -Parent $PSScriptRoot
$workspaceRoot = [IO.Path]::GetFullPath((Join-Path $packageRoot '../..'))
$coreRoot = Join-Path $workspaceRoot 'core'
$linkPath = Join-Path $packageRoot 'Sources/CNativeAPI/core'
if (!(Test-Path -LiteralPath (Join-Path $coreRoot 'include/nativeapi.h'))) {
  throw "Initialize core first: git -C `"$workspaceRoot`" submodule update --init core"
}
$item = Get-Item -LiteralPath $linkPath -Force -ErrorAction SilentlyContinue
if ($item -and $item.LinkType) {
  $targets = @($item.Target)
  if ($targets.Count -ne 1) { throw "Unexpected core link targets: $($item.Target)" }
  $target = [string]$targets[0]
  $resolved = if ([IO.Path]::IsPathRooted($target)) {
    [IO.Path]::GetFullPath($target)
  } else {
    [IO.Path]::GetFullPath((Join-Path (Split-Path $linkPath -Parent) $target))
  }
  if ($resolved -ne $coreRoot) { throw "Unexpected core link target: $resolved" }
  Write-Output 'NativeAPI Swift core link is ready.'
  exit 0
}
if ($item) {
  if ($item.PSIsContainer -or (Get-Content -LiteralPath $linkPath -Raw).Trim() -ne '../../../../core') {
    throw "Refusing to replace unexpected content at $linkPath"
  }
  Remove-Item -LiteralPath $linkPath
}
try {
  New-Item -ItemType SymbolicLink -Path $linkPath -Target '../../../../core' | Out-Null
} catch {
  # Restore Git's placeholder if the host does not allow symlink creation.
  [IO.File]::WriteAllText($linkPath, '../../../../core')
  throw 'Enable Windows Developer Mode (or symlink privileges), then run prepare-windows.ps1 again.'
}
Write-Output 'NativeAPI Swift core link is ready.'
