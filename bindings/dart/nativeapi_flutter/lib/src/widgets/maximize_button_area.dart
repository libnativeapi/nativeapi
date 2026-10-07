import 'package:flutter/rendering.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart' as na;

/// Marks [child], the app's own maximize button in a custom title bar, as the
/// window's maximize button for the system.
///
/// On Windows 11, resting the pointer on it then opens the snap layouts that
/// a hidden title bar takes away. The child keeps handling the pointer itself
/// (hover, press, tap), so it still maximizes or restores the window on a
/// click. The area follows the child as it moves or resizes and is removed
/// with it. Elsewhere this is a no-op wrapper (see
/// `Window.isMaximizeButtonBoundsSupported`).
///
/// Assumes the Flutter view fills the window's content area, as it does in a
/// regular Flutter window.
class MaximizeButtonArea extends SingleChildRenderObjectWidget {
  const MaximizeButtonArea({
    super.key,
    required Widget super.child,
    this.window,
  });

  /// The window the button belongs to. When omitted, the current window.
  final na.Window? window;

  @override
  RenderObject createRenderObject(BuildContext context) =>
      _RenderMaximizeButtonArea(window);

  @override
  void updateRenderObject(BuildContext context, RenderObject renderObject) {
    (renderObject as _RenderMaximizeButtonArea).window = window;
  }
}

class _RenderMaximizeButtonArea extends RenderProxyBox {
  _RenderMaximizeButtonArea(this._window);

  na.Window? _window;
  na.Window? _reported;
  Rect? _bounds;
  bool _scheduled = false;

  set window(na.Window? value) {
    if (identical(value, _window)) return;
    _clear();
    _window = value;
    markNeedsPaint();
  }

  na.Window? _resolve() => _window ?? na.WindowManager.instance.getCurrent();

  @override
  void paint(PaintingContext context, Offset offset) {
    super.paint(context, offset);
    // Positions are final once painted; report after the frame, not in it.
    if (_scheduled) return;
    _scheduled = true;
    SchedulerBinding.instance.addPostFrameCallback((_) {
      _scheduled = false;
      if (!attached || !hasSize) return;
      final bounds = localToGlobal(Offset.zero) & size;
      final window = _resolve();
      if (window == null) return;
      if (bounds == _bounds && identical(window, _reported)) return;
      if (!identical(window, _reported)) _clear();
      if (window.setMaximizeButtonBounds(
        na.Rectangle(
          x: bounds.left,
          y: bounds.top,
          width: bounds.width,
          height: bounds.height,
        ),
      )) {
        _reported = window;
        _bounds = bounds;
      }
    });
  }

  void _clear() {
    _reported?.setMaximizeButtonBounds(
      const na.Rectangle(x: 0, y: 0, width: 0, height: 0),
    );
    _reported = null;
    _bounds = null;
  }

  @override
  void detach() {
    _clear();
    super.detach();
  }
}
