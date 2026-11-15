import CNativeAPI
import Foundation

/// A repeating timer driven directly by the platform's native UI message loop.
///
/// Unlike `DispatchQueue.main.asyncAfter`, `RunLoopTimer` continues firing
/// while a synchronous popup menu or modal tracking loop is active.
@MainActor
public final class RunLoopTimer {
  private final class Box {
    let action: @MainActor () -> Void
    init(_ action: @escaping @MainActor () -> Void) {
      self.action = action
    }
  }

  private var identifier: UInt = 0

  public init(milliseconds: UInt32, action: @escaping @MainActor () -> Void) {
    let context = Unmanaged.passRetained(Box(action)).toOpaque()
    identifier = native_swift_timer_start(
      milliseconds,
      { userData in
        guard let userData else { return }
        let box = Unmanaged<Box>.fromOpaque(userData).takeUnretainedValue()
        MainActor.assumeIsolated {
          box.action()
        }
      },
      context,
      nativeapiReleaseUserData
    )
  }

  public func invalidate() {
    guard identifier != 0 else { return }
    let current = identifier
    identifier = 0
    native_swift_timer_stop(current)
  }

  deinit {
    if identifier != 0 {
      native_swift_timer_stop(identifier)
    }
  }
}

extension Application {
  /// Connects Swift's `DispatchQueue.main` / `@MainActor` executor to the
  /// calling UI thread's native event loop. Called automatically by `run()`
  /// and `runWithWindow(_:)`; exposed for hosts that drive the loop externally.
  public func installMainQueue() {
    native_swift_install_main_queue()
  }
}
