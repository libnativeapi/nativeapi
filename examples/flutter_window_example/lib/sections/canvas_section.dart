import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    hide Button, Menu, MenuItem, Preferences;

import '../event_log.dart';
import '../widgets/state_badges.dart';
import '../widgets/window_canvas.dart';
import 'pane.dart';

/// The map of displays and windows, and under it what the selected window
/// reads back, with the four most used actions.
class CanvasSection extends StatelessWidget {
  const CanvasSection({
    super.key,
    required this.windows,
    required this.displays,
    required this.pane,
    required this.onSelect,
    required this.onMoreActions,
  });

  final List<Window> windows;
  final List<na.Display> displays;

  /// Null when no window is selected.
  final Pane? pane;
  final ValueChanged<Window> onSelect;
  final VoidCallback onMoreActions;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final pane = this.pane;
    return Padding(
      padding: EdgeInsets.all(vars.spacing3),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        spacing: vars.spacing3,
        children: [
          Expanded(
            child: WindowCanvas(
              windows: windows,
              displays: displays,
              selectedWindow: pane?.window,
              onWindowTap: onSelect,
            ),
          ),
          if (pane == null)
            Card(
              variant: CardVariant.sunken,
              size: WidgetSize.small,
              child: Text(
                'Tap a window on the canvas to select it.',
                style: vars.muted,
              ),
            )
          else
            IntrinsicHeight(
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                spacing: vars.spacing3,
                children: [
                  Expanded(
                    flex: 3,
                    child: _Identity(pane: pane, onMoreActions: onMoreActions),
                  ),
                  Expanded(flex: 2, child: _Geometry(window: pane.window)),
                ],
              ),
            ),
        ],
      ),
    );
  }
}

class _Identity extends StatelessWidget {
  const _Identity({required this.pane, required this.onMoreActions});

  final Pane pane;
  final VoidCallback onMoreActions;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final window = pane.window;
    final title = window.title ?? '';

    return Card(
      variant: CardVariant.sunken,
      size: WidgetSize.small,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        spacing: vars.spacing2,
        children: [
          Row(
            spacing: vars.spacing15,
            children: [
              Icon(
                FluentIcons.window_20_regular,
                size: vars.iconMedium,
                color: vars.colorPrimary[600],
              ),
              Expanded(
                child: Text(
                  title.isEmpty ? 'Untitled' : title,
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: vars.titleSmall.copyWith(color: vars.colorContent),
                ),
              ),
              Badge(
                size: WidgetSize.small,
                tint: BadgeTint.primary,
                child: Text('ID: ${window.id}'),
              ),
            ],
          ),
          StateBadges(window: window),
          Wrap(
            spacing: vars.spacing1,
            runSpacing: vars.spacing1,
            children: [
              OptionChip(
                label: 'Maximize',
                selected: window.isMaximized,
                onTap: () {
                  window.maximize();
                  pane.log(
                    'Action: maximize #${window.id}',
                    tone: LogTone.primary,
                  );
                },
              ),
              OptionChip(
                label: 'Minimize',
                selected: window.isMinimized,
                onTap: () {
                  window.minimize();
                  pane.log(
                    'Action: minimize #${window.id}',
                    tone: LogTone.warning,
                  );
                },
              ),
              OptionChip(
                label: 'Restore',
                onTap: () {
                  window.restore();
                  pane.log('Action: restore #${window.id}', tone: LogTone.info);
                },
              ),
              OptionChip(
                label: 'Hide',
                selected: !window.isVisible,
                onTap: () {
                  window.hide();
                  pane.log('Action: hide #${window.id}', tone: LogTone.warning);
                },
              ),
              ActionChip(label: 'More Actions', onTap: onMoreActions),
            ],
          ),
        ],
      ),
    );
  }
}

class _Geometry extends StatelessWidget {
  const _Geometry({required this.window});

  final Window window;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final bounds = window.bounds.toRect();
    final content = window.contentBounds.toRect();
    final value = vars.mono.copyWith(color: vars.colorContent);

    Widget line(String label, String text) => Row(
      children: [
        SizedBox(width: 64, child: Text(label, style: vars.muted)),
        Expanded(
          child: Text(
            text,
            maxLines: 1,
            overflow: TextOverflow.ellipsis,
            style: value,
          ),
        ),
      ],
    );

    return Card(
      variant: CardVariant.sunken,
      size: WidgetSize.small,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        spacing: vars.spacing15,
        children: [
          const SectionLabel('Geometry'),
          line('Size', sizeText(bounds.width, bounds.height)),
          line('Position', '(${bounds.left.toInt()}, ${bounds.top.toInt()})'),
          line('Content', sizeText(content.width, content.height)),
        ],
      ),
    );
  }
}
