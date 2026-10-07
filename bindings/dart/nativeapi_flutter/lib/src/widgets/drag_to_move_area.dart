import 'package:flutter/foundation.dart';
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

/// A custom title-bar region that drags the window and performs the native
/// title-bar double-click action, including the macOS user preference. A
/// secondary click opens the native system window menu where supported
/// (Windows, or a supporting Linux window manager): on release on Windows, as
/// a native title bar does (the menu's modal loop would otherwise swallow the
/// release, and Flutter would ignore the next secondary click), and on press
/// elsewhere (a Wayland compositor accepts the request only with a press).
class DragToMoveArea extends StatelessWidget {
  const DragToMoveArea({super.key, required this.child, this.window});

  final Widget child;

  /// The target window. When omitted, resolves the current window on interaction.
  final Window? window;

  @override
  Widget build(BuildContext context) {
    void showSystemMenu(Offset position) {
      (window ?? WindowManager.instance.getCurrent())?.showSystemMenu(
        Point(x: position.dx, y: position.dy),
      );
    }

    final onRelease = defaultTargetPlatform == TargetPlatform.windows;
    return GestureDetector(
      behavior: HitTestBehavior.translucent,
      onSecondaryTapDown: onRelease
          ? null
          : (details) => showSystemMenu(details.globalPosition),
      onSecondaryTapUp: onRelease
          ? (details) => showSystemMenu(details.globalPosition)
          : null,
      onPanStart: (_) {
        (window ?? WindowManager.instance.getCurrent())?.startDragging();
      },
      onDoubleTap: () {
        (window ?? WindowManager.instance.getCurrent())
            ?.performTitleBarDoubleClick();
      },
      child: child,
    );
  }
}
