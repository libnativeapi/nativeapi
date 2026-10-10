import 'package:dazzui_host/dazzui_host.dart';

import '../signs/sign_style.dart';
import '../tray_controller.dart';

class AddTrayMenu extends StatefulWidget {
  const AddTrayMenu({
    super.key,
    required this.controller,
    required this.onAdded,
  });
  final TrayController controller;
  final VoidCallback onAdded;
  @override
  State<AddTrayMenu> createState() => _AddTrayMenuState();
}

class _AddTrayMenuState extends State<AddTrayMenu> {
  VoidCallback? _close;
  void _add(VoidCallback action) {
    _close?.call();
    action();
    widget.onAdded();
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars, controller = widget.controller;
    Widget item(String label, VoidCallback? action) => Button(
      size: WidgetSize.small,
      variant: ButtonVariant.normal,
      onPressed: action == null ? null : () => _add(action),
      child: Text(label),
    );
    return Popover(
      title: const Text('Add tray item'),
      side: PopoverSide.top,
      align: PopoverAlign.start,
      width: 260,
      trigger: (context, state) {
        _close = state.open ? state.toggle : null;
        return SizedBox(
          width: double.infinity,
          child: Button(
            size: WidgetSize.small,
            variant: ButtonVariant.normal,
            onPressed: state.toggle,
            child: const Text('+ Add'),
          ),
        );
      },
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        mainAxisSize: MainAxisSize.min,
        spacing: vars.spacing1,
        children: [
          const SectionLabel('Icons'),
          item('Default icon', () {
            controller.addIcon();
          }),
          for (final scene in Scene.values)
            item(
              '${scene.name[0].toUpperCase()}${scene.name.substring(1)}',
              () => controller.addScene(scene),
            ),
          item('Three icons', controller.addThreeIcons),
          SizedBox(height: vars.spacing2),
          const SectionLabel('Signs'),
          for (final style in SignStyle.values)
            item(
              style.label,
              TrayController.signsSupported
                  ? () {
                      controller.addSign(style);
                    }
                  : null,
            ),
          if (!TrayController.signsSupported)
            Text(
              'Custom tray signs are available on macOS.',
              style: vars.muted,
            ),
        ],
      ),
    );
  }
}
