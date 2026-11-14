import XCTest

// Only the dedicated simulator is driven. No macOS pointer/keyboard events.
final class PromptTests: XCTestCase {
  @MainActor
  func testConfirmProtocolExample() {
    let springboard = XCUIApplication(bundleIdentifier: "com.apple.springboard")
    let documents = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    let stop = documents.appendingPathComponent("protocol-stop")
    try? FileManager.default.removeItem(at: stop)
    try! "ready".write(to: documents.appendingPathComponent("protocol-ready"), atomically: true, encoding: .utf8)
    let deadline = Date().addingTimeInterval(120)
    var confirmations = 0
    while Date() < deadline {
      if FileManager.default.fileExists(atPath: stop.path) { break }
      for alert in springboard.alerts.allElementsBoundByIndex {
        guard alert.staticTexts.matching(
          NSPredicate(format: "label CONTAINS %@", "Protocol Handler Example")
        ).count > 0 else { continue }
        let open = alert.buttons.matching(
          NSPredicate(format: "label IN %@", ["Open", "打开"])
        ).firstMatch
        if open.exists && open.isHittable {
          open.tap()
          confirmations += 1
          print("Confirmed test-app URL prompt: \(confirmations)")
        }
      }
      Thread.sleep(forTimeInterval: 0.25)
    }
    print("Handled \(confirmations) test-app URL confirmations")
  }
}
