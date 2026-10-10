import Cocoa
import FlutterMacOS
import QuartzCore

private class SignCanvas: NSView {
  override var isFlipped: Bool { true }
}

private class SignPlatformHost: NSView {
  let signKey: Int64
  private let canvas = SignCanvas()
  private var sign: NSView?
  var naturalSize = NSSize(width: 1, height: 1)
  override var isFlipped: Bool { true }

  init(key: Int64, sign: NSView?) {
    signKey = key
    self.sign = sign
    super.init(frame: .zero)
    wantsLayer = true
    layer?.masksToBounds = true
    addSubview(canvas)
    if let sign = sign { canvas.addSubview(sign) }
  }

  required init?(coder: NSCoder) { fatalError("init(coder:) is not supported") }

  override func setFrameSize(_ newSize: NSSize) {
    super.setFrameSize(newSize)
    needsLayout = true
  }

  override func layout() {
    super.layout()
    guard let sign = sign, bounds.width > 0, bounds.height > 0 else { return }
    let scale = min(1, max(0, bounds.width - 24) / naturalSize.width,
      max(0, bounds.height - 24) / naturalSize.height)
    let size = NSSize(width: naturalSize.width * scale, height: naturalSize.height * scale)
    canvas.frame = NSRect(x: (bounds.width - size.width) / 2,
      y: (bounds.height - size.height) / 2, width: size.width, height: size.height)
    canvas.bounds = NSRect(origin: .zero, size: naturalSize)
    sign.frame = NSRect(origin: .zero, size: naturalSize)
  }

  var isAttached: Bool { sign?.superview === canvas && canvas.superview === self }

  func detach() {
    sign?.removeFromSuperview()
    sign = nil
  }
}

private class WeakSignHost {
  weak var value: SignPlatformHost?
  init(_ value: SignPlatformHost) { self.value = value }
}

private class SignPlatformViewFactory: NSObject, FlutterPlatformViewFactory {
  private var signs: [Int64: NSView] = [:]
  private var hosts: [Int64: WeakSignHost] = [:]

  func register(key: Int64, sign: NSView) { signs[key] = sign }

  func unregister(key: Int64) {
    for host in hosts.values where host.value?.signKey == key { host.value?.detach() }
    signs.removeValue(forKey: key)
  }

  func createArgsCodec() -> (FlutterMessageCodec & NSObjectProtocol)? {
    FlutterStandardMessageCodec.sharedInstance()
  }

  func create(withViewIdentifier viewId: Int64, arguments args: Any?) -> NSView {
    let key = ((args as? [String: Any])?["sign"] as? NSNumber)?.int64Value ?? 0
    let host = SignPlatformHost(key: key, sign: signs[key])
    hosts = hosts.filter { $0.value.value != nil }
    hosts[viewId] = WeakSignHost(host)
    return host
  }

  func resize(id: Int64, size: NSSize) -> Bool {
    guard let host = hosts[id]?.value, size.width > 0, size.height > 0 else { return false }
    host.naturalSize = size
    host.needsLayout = true
    host.layoutSubtreeIfNeeded()
    return host.isAttached
  }
}

class MainFlutterWindow: NSWindow {
  private var signTypography: FlutterMethodChannel?
  private let signPlatformViews = SignPlatformViewFactory()

  override func awakeFromNib() {
    let flutterViewController = FlutterViewController()
    let windowFrame = self.frame
    self.contentViewController = flutterViewController
    self.setFrame(windowFrame, display: true)
    self.setContentSize(NSSize(width: 800, height: 600))
    self.center()

    RegisterGeneratedPlugins(registry: flutterViewController)
    flutterViewController.registrar(forPlugin: "NativeSignPreview")
      .register(signPlatformViews, withId: "dev.nativeapi.tray_sign/preview")

    // AppKit supplies typography, native text measurements and layer finishes.
    // View ownership, text, layout and tray hosting remain in nativeapi.
    signTypography = FlutterMethodChannel(
      name: "dev.nativeapi.tray_sign/typography",
      binaryMessenger: flutterViewController.engine.binaryMessenger)
    signTypography?.setMethodCallHandler { call, result in
      if call.method == "configurePreviewWindow",
         let args = call.arguments as? [String: Any],
         let address = args["address"] as? NSNumber,
         let pointer = UnsafeRawPointer(bitPattern: address.uintValue) {
        let window = Unmanaged<NSWindow>.fromOpaque(pointer).takeUnretainedValue()
        // The nativeapi handle owns this window and releases it after close.
        // AppKit's default self-release would consume that same ownership.
        window.isReleasedWhenClosed = false
        result(nil)
        return
      }
      if call.method == "registerEmbeddedSign",
         let args = call.arguments as? [String: Any],
         let address = args["root"] as? NSNumber,
         let pointer = UnsafeRawPointer(bitPattern: address.uintValue) {
        let sign = Unmanaged<NSView>.fromOpaque(pointer).takeUnretainedValue()
        self.signPlatformViews.register(key: address.int64Value, sign: sign)
        result(nil)
        return
      }
      if call.method == "unregisterEmbeddedSign",
         let args = call.arguments as? [String: Any],
         let address = args["root"] as? NSNumber {
        self.signPlatformViews.unregister(key: address.int64Value)
        result(nil)
        return
      }
      if call.method == "layoutEmbeddedSign",
         let args = call.arguments as? [String: Any],
         let id = args["id"] as? NSNumber,
         let width = args["width"] as? NSNumber,
         let height = args["height"] as? NSNumber {
        result(self.signPlatformViews.resize(id: id.int64Value,
          size: NSSize(width: width.doubleValue, height: height.doubleValue)))
        return
      }
      guard call.method == "style",
            let args = call.arguments as? [String: Any],
            let labels = args["labels"] as? [[String: Any]],
            let rootAddress = args["root"] as? NSNumber,
            let rootPointer = UnsafeRawPointer(bitPattern: rootAddress.uintValue)
      else { result(FlutterMethodNotImplemented); return }
      // Dart retains these native handles until this main-thread call returns.
      let root = Unmanaged<NSView>.fromOpaque(rootPointer).takeUnretainedValue()
      root.wantsLayer = true
      root.layer?.cornerRadius = (args["radius"] as? NSNumber)?.doubleValue ?? 0
      root.layer?.masksToBounds = true
      for entry in labels {
        guard let address = entry["address"] as? NSNumber,
              let pointer = UnsafeRawPointer(bitPattern: address.uintValue),
              let size = entry["size"] as? NSNumber else { continue }
        let field = Unmanaged<NSTextField>.fromOpaque(pointer).takeUnretainedValue()
        let fontName = entry["font"] as? String ?? ((entry["chinese"] as? Bool == true)
          ? "PingFangSC-Semibold" : "Helvetica-Bold")
        field.font = NSFont(name: fontName, size: size.doubleValue)
          ?? NSFont.systemFont(ofSize: size.doubleValue, weight: .bold)
      }
      for entry in args["views"] as? [[String: Any]] ?? [] {
        guard let address = entry["address"] as? NSNumber,
              let pointer = UnsafeRawPointer(bitPattern: address.uintValue) else { continue }
        let view = Unmanaged<NSView>.fromOpaque(pointer).takeUnretainedValue()
        view.wantsLayer = true
        let layer = view.layer
        layer?.cornerRadius = (entry["radius"] as? NSNumber)?.doubleValue ?? 0
        layer?.borderWidth = (entry["borderWidth"] as? NSNumber)?.doubleValue ?? 0
        let rgb = (entry["borderColor"] as? NSNumber)?.uint32Value ?? 0
        layer?.borderColor = NSColor(srgbRed: CGFloat((rgb >> 16) & 255) / 255,
          green: CGFloat((rgb >> 8) & 255) / 255, blue: CGFloat(rgb & 255) / 255, alpha: 1).cgColor
        layer?.masksToBounds = entry["clip"] as? Bool ?? false

      }
      result(nil)
    }

    super.awakeFromNib()
  }
}
