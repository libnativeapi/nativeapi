#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 2 ]]; then
  echo "Usage: $0 <ARM64 Swift Static Linux SDK sysroot> <output directory>" >&2
  exit 2
fi

binding="$(cd "$(dirname "$0")/.." && pwd)"
sdk="$(cd "$1" && pwd)"
mkdir -p "$2"
output="$(cd "$2" && pwd)"
resources="$sdk/usr/lib/swift_static"
if [[ ! -d "$resources/linux-static" ]]; then
  echo "Expected the ARM64 sysroot from the Swift Static Linux SDK." >&2
  exit 1
fi
# Serialized SDK modules reference this empty Clang VFS overlay.
if [[ ! -e "$resources/<clang-system-vfs-overlay>" ]]; then
  printf '%s\n' '{"version":0,"roots":[]}' > "$resources/<clang-system-vfs-overlay>"
fi
if [[ ! -f "$binding/Sources/CNativeAPI/core/include/nativeapi.h" ]]; then
  echo "Initialize the NativeAPI core submodule before building the Swift binding." >&2
  exit 1
fi

if [[ -n "${SWIFT_TOOLCHAIN_BIN:-}" ]]; then
  swiftc=("$SWIFT_TOOLCHAIN_BIN/swiftc")
  toolchain=()
else
  swiftc=(swiftly run swiftc)
  toolchain=(+6.4.2)
fi

sources=()
while IFS= read -r -d '' source; do sources+=("$source"); done \
  < <(find "$binding/Sources/NativeAPI" -name '*.swift' -print0)

# Swift objects use the musl runtime ABI. The OHOS NDK compiles the C/C++
# core separately and links it with this object into the UIAbility library.
"${swiftc[@]}" -target aarch64-swift-linux-musl -sdk "$sdk" -resource-dir "$resources" \
  -static-stdlib -swift-version 5 -parse-as-library -whole-module-optimization -O \
  -I "$binding/Sources/CNativeAPI/include" -emit-object -emit-module -module-name NativeAPI \
  -emit-module-path "$output/NativeAPI.swiftmodule" -o "$output/NativeAPI.o" \
  "${sources[@]}" ${toolchain[@]+"${toolchain[@]}"}
