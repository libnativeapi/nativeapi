// Read only WindowServer metadata for the requested demo process; no input.
import Foundation
import CoreGraphics

guard CommandLine.arguments.count == 2,
      let demoPid = Int(CommandLine.arguments[1]) else {
    fputs("Usage: swift check_tray_color_macos.swift <pid>\n", stderr)
    exit(2)
}
let windows = CGWindowListCopyWindowInfo(.optionAll, kCGNullWindowID) as? [[String: Any]] ?? []
let own = windows.filter { ($0[kCGWindowOwnerPID as String] as? Int) == demoPid }
guard !own.isEmpty else {
    fputs("No WindowServer windows for demo PID \(demoPid)\n", stderr)
    exit(1)
}
for window in own {
    let json = try JSONSerialization.data(withJSONObject: window, options: [.sortedKeys])
    print(String(data: json, encoding: .utf8)!)
}
