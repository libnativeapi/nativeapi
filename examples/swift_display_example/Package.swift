// swift-tools-version:5.9
import PackageDescription

let package = Package(
  name: "SwiftDisplayExample",
  platforms: [.macOS(.v11)],
  dependencies: [
    .package(name: "NativeAPI", path: "../../bindings/swift")
  ],
  targets: [
    .executableTarget(
      name: "SwiftDisplayExample",
      dependencies: [.product(name: "NativeAPI", package: "NativeAPI")],
      path: "Sources/SwiftDisplayExample"
    )
  ]
)
