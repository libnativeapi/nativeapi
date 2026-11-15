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
libraries="$resources/linux-static"
if [[ ! -d "$libraries" ]]; then
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
  ar=("$SWIFT_TOOLCHAIN_BIN/llvm-ar")
  toolchain=()
else
  swiftc=(swiftly run swiftc)
  ar=(swiftly run llvm-ar)
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

# Stage the Swift/Foundation static archives and strip the local-exec TLS
# exception object so the resulting archives link cleanly into an OHOS .so.
for name in swiftCore swift_Concurrency swift_StringProcessing swift_RegexParser \
  swiftMusl swiftSynchronization swiftDispatch dispatch BlocksRuntime \
  Foundation FoundationEssentials FoundationInternationalization CoreFoundation \
  _FoundationCShims _FoundationCollections _FoundationICU; do
  cp "$libraries/lib$name.a" "$output/"
done
cp "$libraries/aarch64/swiftrt.o" "$sdk/usr/lib/libc++.a" "$sdk/usr/lib/libc++abi.a" \
  "$sdk/usr/lib/libfts.a" "$output/"
"${ar[@]}" d "$output/libc++abi.a" cxa_exception_storage.cpp.o ${toolchain[@]+"${toolchain[@]}"}
