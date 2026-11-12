/*
 * The core's Win32 view layer imports GetWindowSubclass(), which only
 * ComCtl32 v6 exports. Without a manifest that asks for v6, the loader binds
 * the executable to the v5 DLL and terminates with STATUS_ENTRYPOINT_NOT_FOUND
 * before main() runs. Embed the dependency from an object file so every
 * executable that links this package gets it, with no manifest flag in
 * Package.swift (which would have to be an unsafeFlag).
 */
#if defined(_WIN32)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/* Keep the translation unit non-empty on every compiler. */
int nativeapi_swift_windows_manifest_anchor(void) { return 0; }
