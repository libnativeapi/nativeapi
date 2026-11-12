// swift-tools-version:5.9
import Foundation
import PackageDescription

// This source-checkout package compiles the workspace's `core` submodule
// directly (Sources/CNativeAPI/core is a symlink), so it needs no CMake step
// and no separate native library at runtime. It builds on macOS, Linux and
// Windows; each host compiles only its own platform layer.
//
// The SwiftPM target rules below keep the platform split honest:
//   * macOS links the Apple frameworks and compiles platform/macos.
//   * Linux compiles platform/linux and gets its include/link flags from the
//     gtk+-3.0, x11 and xi pkg-config modules through systemLibrary targets.
//     pkg-config is used instead of unsafeFlags so the package stays usable as
//     a SwiftPM dependency.
//   * Windows compiles platform/windows (without the optional WinUI 3 layer)
//     and links the Win32 libraries the core imports. WindowsManifest.c makes
//     the linker embed the Common Controls v6 dependency the core needs.
//
// `core/build` only exists after someone configures the core with CMake; the
// package never needs it, but exclude it when present so SwiftPM does not
// discover CMake's own sources.
var excludes = [
  // Not part of the library.
  "core/tests",
  "core/examples",
  "core/docs",
  "core/cmake",
]

let platformExcludes: [String]
let nativeDependencies: [Target.Dependency]
let nativeLinkerSettings: [LinkerSetting]
let systemTargets: [Target]

#if os(macOS)
platformExcludes = ["android", "ios", "linux", "ohos", "wasm", "windows"]
nativeDependencies = []
nativeLinkerSettings = [
  .linkedFramework("Cocoa"),
  .linkedFramework("CoreServices"),
  .linkedFramework("QuartzCore"),
  .linkedFramework("Carbon"),
  .linkedFramework("IOKit"),
  .linkedFramework("ServiceManagement"),
]
systemTargets = []
#elseif os(Linux)
platformExcludes = ["android", "ios", "macos", "ohos", "wasm", "windows"]
nativeDependencies = ["CGTK3", "CX11", "CXI"]
nativeLinkerSettings = [
  .linkedLibrary("pthread"),
  .linkedLibrary("dl"),
  .linkedLibrary("m"),
]
systemTargets = [
  .systemLibrary(
    name: "CGTK3",
    path: "Sources/CGTK3",
    pkgConfig: "gtk+-3.0",
    providers: [.apt(["libgtk-3-dev"]), .yum(["gtk3-devel"])]
  ),
  .systemLibrary(
    name: "CX11",
    path: "Sources/CX11",
    pkgConfig: "x11",
    providers: [.apt(["libx11-dev"]), .yum(["libX11-devel"])]
  ),
  .systemLibrary(
    name: "CXI",
    path: "Sources/CXI",
    pkgConfig: "xi",
    providers: [.apt(["libxi-dev"]), .yum(["libXi-devel"])]
  ),
]
#elseif os(Windows)
platformExcludes = ["android", "ios", "linux", "macos", "ohos", "wasm"]
nativeDependencies = []
nativeLinkerSettings = [
  .linkedLibrary("user32"),
  .linkedLibrary("shell32"),
  .linkedLibrary("ole32"),
  .linkedLibrary("oleaut32"),
  .linkedLibrary("uuid"),
  .linkedLibrary("comctl32"),
  .linkedLibrary("dwmapi"),
  .linkedLibrary("gdi32"),
  .linkedLibrary("gdiplus"),
  .linkedLibrary("crypt32"),
  .linkedLibrary("advapi32"),
  .linkedLibrary("version"),
  .linkedLibrary("psapi"),
  .linkedLibrary("shlwapi"),
]
systemTargets = []
// WinUI 3 is opt-in in the core's CMake build; the Swift package ships the
// plain Win32 backend, so leave the XAML layer out.
excludes += [
  "core/src/platform/windows/menu_winui3_windows.cpp",
  "core/src/platform/windows/message_dialog_winui3_windows.cpp",
  "core/src/platform/windows/view_winui3_windows.cpp",
  "core/src/platform/windows/window_winui3_windows.cpp",
  "core/src/platform/windows/winui3_runtime_windows.cpp",
]
#else
platformExcludes = ["android", "ios", "linux", "macos", "ohos", "wasm", "windows"]
nativeDependencies = []
nativeLinkerSettings = []
systemTargets = []
#endif

excludes += platformExcludes.map { "core/src/platform/\($0)" }

let packageDirectory = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
if FileManager.default.fileExists(
  atPath: packageDirectory.appendingPathComponent("Sources/CNativeAPI/core/build").path
) {
  excludes.append("core/build")
}

let package = Package(
  name: "NativeAPI",
  platforms: [.macOS(.v11)],
  products: [
    .library(name: "NativeAPI", targets: ["NativeAPI"])
  ],
  targets: systemTargets + [
    .target(
      name: "CNativeAPI",
      dependencies: nativeDependencies,
      path: "Sources/CNativeAPI",
      exclude: excludes,
      publicHeadersPath: "include",
      linkerSettings: nativeLinkerSettings
    ),
    .target(name: "NativeAPI", dependencies: ["CNativeAPI"], path: "Sources/NativeAPI"),
    .testTarget(
      name: "NativeAPITests",
      dependencies: ["NativeAPI"],
      path: "Tests/NativeAPITests"
    ),
  ],
  cxxLanguageStandard: .cxx17
)
