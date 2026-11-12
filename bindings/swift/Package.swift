// swift-tools-version:5.9
import Foundation
import PackageDescription

// This source-checkout package compiles the workspace's `core` submodule
// directly (Sources/CNativeAPI/core is a symlink), so it needs no CMake step
// and no separate native library at runtime. The package targets macOS, the
// platform this binding is built and tested on; the core also supports Windows
// and Linux, where SwiftPM would need each platform's own setup.
// `core/build` only exists after someone configures the core with CMake; the
// package never needs it, but exclude it when present so SwiftPM does not
// discover CMake's own sources.
var excludes = [
  // Other platforms' implementations; the host's are compiled below.
  "core/src/platform/android",
  "core/src/platform/ios",
  "core/src/platform/linux",
  "core/src/platform/ohos",
  "core/src/platform/wasm",
  "core/src/platform/windows",
  // Not part of the library.
  "core/tests",
  "core/examples",
  "core/docs",
  "core/cmake",
]
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
  targets: [
    .target(
      name: "CNativeAPI",
      path: "Sources/CNativeAPI",
      exclude: excludes,
      publicHeadersPath: "include",
      linkerSettings: [
        .linkedFramework("Cocoa"),
        .linkedFramework("CoreServices"),
        .linkedFramework("QuartzCore"),
        .linkedFramework("Carbon"),
        .linkedFramework("IOKit"),
        .linkedFramework("ServiceManagement"),
      ]
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
