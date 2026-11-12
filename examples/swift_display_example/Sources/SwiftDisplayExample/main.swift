import Foundation
import NativeAPI

// Prints the displays attached to this desktop.
let displays = DisplayManager.shared.getAll()
guard !displays.isEmpty else {
  print("no displays")
  exit(0)
}

for display in displays {
  let size = display.size
  let workArea = display.workArea
  print(
    "\(display.name ?? "display") \(Int(size.width))x\(Int(size.height)) "
      + "at \(Int(display.position.x)),\(Int(display.position.y)) "
      + "work area \(Int(workArea.width))x\(Int(workArea.height)) "
      + "scale \(display.scaleFactor) primary \(display.isPrimary)"
  )
  display.dispose()
}
