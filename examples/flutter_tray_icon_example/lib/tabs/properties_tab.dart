import 'package:dazzui_host/dazzui_host.dart' hide ContextMenu, Menu, MenuItem;
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

import '../tray_controller.dart';

/// Asks the shell to show its text editor.
typedef EditText = void Function(
  String title,
  String initial,
  ValueChanged<String> onSubmit,
);

/// One row per TrayIcon API. The block on top never shows what was written —
/// it shows what the native getters return afterwards.
class PropertiesTab extends StatelessWidget {
  const PropertiesTab({
    super.key,
    required this.controller,
    required this.onEdit,
  });

  final TrayController controller;
  final EditText onEdit;

  static const _titles = <String, String?>{
    'No title': null,
    '42%': '42%',
    '00:12': '00:12',
    '你好': '你好',
  };

  static const _tooltips = <String, String?>{
    'No tooltip': null,
    'Short': kDefaultTooltip,
    'Long':
        'A long tooltip that says rather more than a tooltip usually should, '
        'to see where the platform cuts it off',
    '2 lines': 'Line one\nLine two',
  };

  static const _triggers = <String, ContextMenuTrigger>{
    'Manual': ContextMenuTrigger.none,
    'Left': ContextMenuTrigger.clicked,
    'Right': ContextMenuTrigger.rightClicked,
    'Double': ContextMenuTrigger.doubleClicked,
  };

  @override
  Widget build(BuildContext context) {
    final entry = controller.selected;
    if (entry == null) return const SizedBox.shrink();
    final icon = entry.trayIcon;
    final title = icon.getTitle();
    final tooltip = icon.getTooltip();
    final visible = icon.isVisible();
    final trigger = icon.getContextMenuTrigger();

    return ListView(
      children: [
        _StateBlock(controller: controller, entry: entry),
        OptionRow(
          label: 'Title',
          children: [
            for (final MapEntry(:key, :value) in _titles.entries)
              OptionChip(
                label: key,
                selected: (title ?? '') == (value ?? ''),
                onTap: () => controller.setTitle(value),
              ),
            if (TrayController.titleSupported)
              ActionChip(
                label: 'Title…',
                onTap: () => onEdit('Title', title ?? '', controller.setTitle),
              )
            else
              const Hint('no titles on Windows'),
          ],
        ),
        OptionRow(
          label: 'Tooltip',
          children: [
            for (final MapEntry(:key, :value) in _tooltips.entries)
              OptionChip(
                label: key,
                selected: (tooltip ?? '') == (value ?? ''),
                onTap: () => controller.setTooltip(value),
              ),
            ActionChip(
              label: 'Tooltip…',
              onTap: () =>
                  onEdit('Tooltip', tooltip ?? '', controller.setTooltip),
            ),
          ],
        ),
        OptionRow(
          label: 'Visible',
          children: [
            OptionChip(
              label: 'Shown',
              selected: visible,
              onTap: () => controller.setVisible(true),
            ),
            OptionChip(
              label: 'Hidden',
              selected: !visible,
              onTap: () => controller.setVisible(false),
            ),
          ],
        ),
        OptionRow(
          label: 'Trigger',
          children: [
            for (final MapEntry(:key, :value) in _triggers.entries)
              OptionChip(
                label: key,
                selected: trigger == value,
                onTap: () => controller.setTrigger(value),
              ),
          ],
        ),
        OptionRow(
          label: 'Menu',
          children: [
            if (TrayController.openMenuSupported) ...[
              ActionChip(label: 'Open menu', onTap: controller.openMenu),
              ActionChip(
                label: 'Open, close in 2 s',
                onTap: () =>
                    controller.openMenu(closeAfter: const Duration(seconds: 2)),
              ),
            ] else
              const Hint('only the shell opens it on Linux'),
          ],
        ),
        // Only Windows has a second menu backend to choose.
        if (Menu.isBackendSupported(MenuBackend.winUi3))
          OptionRow(
            label: 'Backend',
            children: [
              for (final backend in MenuBackend.values)
                OptionChip(
                  label: backend == MenuBackend.winUi3 ? 'WinUI 3' : 'Native',
                  selected: controller.menuBackend == backend,
                  onTap: Menu.isBackendSupported(backend)
                      ? () => controller.setMenuBackend(backend)
                      : null,
                ),
            ],
          ),
      ],
    );
  }
}

class _StateBlock extends StatelessWidget {
  const _StateBlock({required this.controller, required this.entry});

  final TrayController controller;
  final TrayEntry entry;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final icon = entry.trayIcon;
    final bounds = icon.getBounds().toRect();
    final value = vars.mono.copyWith(color: vars.colorContent);
    String quote(String? s) =>
        s == null ? 'null' : '"${s.replaceAll('\n', r'\n')}"';

    Widget line(String text) =>
        Text(text, maxLines: 1, overflow: TextOverflow.ellipsis, style: value);

    Widget counter(String label, int count) => Badge(
      size: WidgetSize.small,
      variant: count > 0 ? BadgeVariant.tinted : BadgeVariant.outlined,
      tint: count > 0 ? BadgeTint.primary : BadgeTint.neutral,
      child: Text('$label $count'),
    );

    return Column(
      children: [
        Padding(
          padding: EdgeInsets.fromLTRB(
            vars.spacing25,
            vars.spacing2,
            vars.spacing25,
            vars.spacing2,
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  const Expanded(child: SectionLabel('Read back from getters')),
                  ActionChip(
                    label: 'Window to icon',
                    onTap: TrayController.boundsSupported
                        ? controller.moveWindowToIcon
                        : null,
                  ),
                  SizedBox(width: vars.spacing1),
                  ActionChip(label: 'Refresh', onTap: controller.refresh),
                ],
              ),
              SizedBox(height: vars.spacing05),
              line(
                'id ${icon.getId()} · visible ${icon.isVisible()} · '
                'trigger ${icon.getContextMenuTrigger().name}',
              ),
              line('title ${quote(icon.getTitle())}'),
              line('tooltip ${quote(icon.getTooltip())}'),
              line(
                'bounds ${bounds.left.round()},${bounds.top.round()} '
                '${bounds.width.round()}×${bounds.height.round()} · '
                'supported ${controller.supported} · '
                'getAll ${controller.managerCount}',
              ),
              SizedBox(height: vars.spacing15),
              Wrap(
                spacing: vars.spacing1,
                crossAxisAlignment: WrapCrossAlignment.center,
                children: [
                  counter('Left', entry.clicks),
                  counter('Right', entry.rightClicks),
                  counter('Double', entry.doubleClicks),
                  ActionChip(
                    label: 'Reset counts',
                    onTap: controller.resetCounters,
                  ),
                ],
              ),
            ],
          ),
        ),
        const Divider(),
      ],
    );
  }
}
