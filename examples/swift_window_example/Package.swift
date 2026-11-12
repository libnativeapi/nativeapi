// swift-tools-version:5.9
import PackageDescription

let package = Package(
  name: "SwiftWindowExample",
  platforms: [.macOS(.v11)],
  dependencies: [
    .package(name: "NativeAPI", path: "../../bindings/swift")
  ],
  targets: [
    .executableTarget(
      name: "SwiftWindowExample",
      dependencies: [.product(name: "NativeAPI", package: "NativeAPI")],
      path: "Sources/SwiftWindowExample"
    )
  ]
)
