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

  @MainActor
  func testRunLoopTimerFiresAndInvalidatesReentrantly() {
    let fired = expectation(description: "RunLoopTimer fires and stops cleanly")
    var ticks = 0
    var timer: RunLoopTimer?
    timer = RunLoopTimer(milliseconds: 10) {
      ticks += 1
      if ticks == 2 {
        timer?.invalidate()
        fired.fulfill()
      }
    }
    wait(for: [fired], timeout: 2.0)
    RunLoop.main.run(until: Date().addingTimeInterval(0.05))
    XCTAssertEqual(ticks, 2)
  }

  @MainActor
  func testViewTypographyBorderedAccessibilityAndResize() throws {
    let container = try XCTUnwrap(View.create())
    defer { container.dispose() }
    container.setFrame(Rectangle(x: 0, y: 0, width: 120, height: 80))

    var resizedSizes: [Size] = []
    let listenerId = container.addListener { event in
      if case let .resized(_, newSize) = event {
        resizedSizes.append(newSize)
      }
    }
    defer { _ = container.removeListener(listenerId) }

    container.setFrame(Rectangle(x: 0, y: 0, width: 240, height: 140))
    XCTAssertEqual(resizedSizes.last?.width, 240)
    XCTAssertEqual(resizedSizes.last?.height, 140)

    container.setAccessibilityLabel("Primary toolbar")
    XCTAssertEqual(container.accessibilityLabel, "Primary toolbar")
    container.setAccessibilityLabel(nil)
    XCTAssertNil(container.accessibilityLabel)

    let label = try XCTUnwrap(Label.create("Hello"))
    defer { label.dispose() }
    label.setFontFamily("Menlo")
    label.setFontWeight(700)
    XCTAssertEqual(label.fontFamily, "Menlo")
    XCTAssertEqual(label.fontWeight, 700)

    let button = try XCTUnwrap(Button.create("Save"))
    defer { button.dispose() }
    button.setFontSize(15)
    button.setFontFamily("Helvetica")
    button.setFontWeight(600)
    XCTAssertEqual(button.fontSize, 15, accuracy: 0.5)
    XCTAssertEqual(button.fontFamily, "Helvetica")
    XCTAssertEqual(button.fontWeight, 600)

    let field = try XCTUnwrap(TextField.create("Draft"))
    defer { field.dispose() }
    XCTAssertTrue(field.isBordered)
    field.setBordered(false)
    XCTAssertFalse(field.isBordered)
    field.setFontFamily("Menlo")
    field.setFontSize(13)
    field.setFontWeight(500)
    XCTAssertEqual(field.fontFamily, "Menlo")
    XCTAssertEqual(field.fontWeight, 500)
  }
}
