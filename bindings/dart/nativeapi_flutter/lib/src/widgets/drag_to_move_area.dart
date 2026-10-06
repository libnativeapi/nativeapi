import 'package:flutter/widgets.dart';
import 'package:nativeapi/nativeapi.dart'
    hide
        Brightness,
        Color,
        Display,
        EdgeInsets,
        Image,
        ModifierKey,
        ShortcutManager,
        Size,
        TextField,
        View;

/// A custom title-bar region that drags the window and toggles maximization
/// on double tap. A secondary-button press opens the native system window menu
/// where supported (Windows, or a supporting Linux window manager).
class DragToMoveArea extends StatelessWidget {
  const DragToMoveArea({super.key, required this.child, this.window});

  final Widget child;

  /// The target window. When omitted, resolves the current window on interaction.
  final Window? window;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      behavior: HitTestBehavior.translucent,
      onSecondaryTapDown: (details) {
        (window ?? WindowManager.instance.getCurrent())?.showSystemMenu(
          Point(x: details.globalPosition.dx, y: details.globalPosition.dy),
        );
      },
      onPanStart: (_) {
        (window ?? WindowManager.instance.getCurrent())?.startDragging();
      },
      onDoubleTap: () {
        final target = window ?? WindowManager.instance.getCurrent();
        if (target == null) return;
        if (target.isMaximized) {
          target.unmaximize();
        } else {
          target.maximize();
        }
      },
      child: child,
    );
  }
}
