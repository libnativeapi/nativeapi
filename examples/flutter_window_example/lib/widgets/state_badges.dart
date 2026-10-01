import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart' show Window;

/// The window's boolean state, one badge each: tinted when on, outlined when
/// off.
class StateBadges extends StatelessWidget {
  const StateBadges({super.key, required this.window});

  final Window window;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    Widget badge(String label, bool on, BadgeTint tint) => Badge(
      size: WidgetSize.small,
      variant: on ? BadgeVariant.tinted : BadgeVariant.outlined,
      tint: on ? tint : BadgeTint.neutral,
      child: Text(label),
    );

    return Wrap(
      spacing: vars.spacing1,
      runSpacing: vars.spacing1,
      children: [
        badge('Visible', window.isVisible, BadgeTint.success),
        badge('Focused', window.isFocused, BadgeTint.primary),
        badge('Maximized', window.isMaximized, BadgeTint.info),
        badge('Minimized', window.isMinimized, BadgeTint.warning),
        badge('Fullscreen', window.isFullScreen, BadgeTint.danger),
        badge('Always on Top', window.isAlwaysOnTop, BadgeTint.info),
      ],
    );
  }
}
