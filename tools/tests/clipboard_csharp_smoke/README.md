# Clipboard C# smoke test

On macOS, build the core test wrapper and a native shared library, then run from
this repository root. The wrapper snapshots and restores all native pasteboard data.

```sh
dotnet build tools/tests/clipboard_csharp_smoke/ClipboardSmoke.csproj
core/build/tests/clipboard_macos_test env \
  NATIVEAPI_LIBRARY_PATH="$PWD/bindings/python/build/libnativeapi.dylib" \
  dotnet tools/tests/clipboard_csharp_smoke/bin/Debug/net8.0/ClipboardSmoke.dll
```

Use `DOTNET_ROLL_FORWARD=Major` if only a newer .NET runtime is installed.
The Dart counterpart is `bindings/dart/nativeapi/tool/clipboard_smoke.dart`; run it
from that package directory through the same wrapper using `dart run`.
