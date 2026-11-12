import Foundation
import NativeAPI

// A minimal native window driven from Swift. Run `swift run SwiftWindowExample`
// and close the window, or pass `--smoke` for a self-closing check.
let smoke = CommandLine.arguments.contains("--smoke")

guard let window = Window.create() else {
  FileHandle.standardError.write(Data("nativeapi: could not create a window\n".utf8))
  exit(1)
}

window.setTitle("NativeAPI Swift")
window.setSize(Size(width: 720, height: 480), animate: false)
window.center()

let listener = window.addListener { event in
  switch event {
  case .moved(_, let position):
    print("moved to \(Int(position.x)),\(Int(position.y))")
  case .resized(_, let size):
    print("resized to \(Int(size.width))x\(Int(size.height))")
  case .closeRequested:
    print("close requested")
  default:
    break
  }
}

window.show()

if smoke {
  DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
    _ = window.close()
    Application.shared.quit(0)
  }
}

let exitCode = Application.shared.runWithWindow(window)

_ = window.removeListener(listener)
window.dispose()
exit(exitCode)
