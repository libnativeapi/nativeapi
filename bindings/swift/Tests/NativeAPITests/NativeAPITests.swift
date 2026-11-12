import XCTest

@testable import NativeAPI

final class NativeAPITests: XCTestCase {
  func testPreferencesRoundTrip() throws {
    let preferences = try XCTUnwrap(Preferences.createWithScope("nativeapi-swift-tests"))
    defer { preferences.dispose() }

    let key = "nativeapi.swift.test.\(UUID().uuidString)"
    defer { _ = preferences.remove(key) }

    XCTAssertEqual(preferences.get(key, defaultValue: "absent"), "absent")
    XCTAssertTrue(preferences.set(key, value: "dark"))
    XCTAssertTrue(preferences.contains(key))
    XCTAssertEqual(preferences.get(key, defaultValue: "absent"), "dark")
    XCTAssertEqual(preferences.all[key], "dark")
    XCTAssertTrue(preferences.remove(key))
    XCTAssertEqual(preferences.get(key, defaultValue: "absent"), "absent")
  }

  func testDisplaysHaveGeometry() throws {
    let displays = DisplayManager.shared.getAll()
    XCTAssertFalse(displays.isEmpty, "a desktop session has at least one display")
    defer { displays.forEach { $0.dispose() } }

    let primary = try XCTUnwrap(DisplayManager.shared.getPrimary())
    defer { primary.dispose() }

    XCTAssertGreaterThan(primary.size.width, 0)
    XCTAssertGreaterThan(primary.size.height, 0)
    XCTAssertGreaterThan(primary.scaleFactor, 0)
    XCTAssertTrue(displays.contains { $0.id == primary.id })
  }

  func testColorHelpers() {
    XCTAssertEqual(Color.black.r, 0)
    XCTAssertEqual(Color.white.a, 255)
    XCTAssertEqual(Color.fromRgba(255, green: 0, blue: 0, alpha: 255).toRgba(), 0xFF00_00FF)
    XCTAssertEqual(Color.fromHex("#0000FF").toRgba(), 0x0000_FFFF)
  }

  func testKeyboardAcceleratorRoundTrip() {
    let accelerator = KeyboardAccelerator(modifiers: .meta, key: "k")
    XCTAssertFalse(accelerator.isEmpty)
    XCTAssertEqual(accelerator.toString(), "Cmd+k")

    let combined = KeyboardAccelerator(modifiers: [.meta, .shift], key: "k")
    XCTAssertEqual(combined.modifiers.rawValue, 9)
    XCTAssertTrue(combined.toString()?.contains("Shift") == true)
  }
}
