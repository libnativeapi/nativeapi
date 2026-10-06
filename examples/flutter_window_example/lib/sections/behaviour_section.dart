import 'package:dazzui_host/dazzui_host.dart';

import 'pane.dart';

/// Stacking, what the user may do to the window, and the platform-specific
/// switches.
class BehaviourSection extends StatelessWidget {
  const BehaviourSection({super.key, required this.pane});

  final Pane pane;

  @override
  Widget build(BuildContext context) {
    final w = pane.window;

    Widget toggle(String label, bool value, void Function(bool) set) =>
        SwitchRow(
          title: label,
          value: value,
          onChanged: (v) {
            set(v);
            pane.feedback('$label: ${v ? 'ON' : 'OFF'}');
          },
        );

    return PaneBody(
      children: [
        PreferenceSection(
          label: 'Stacking',
          children: [
            toggle(
              'Always on Top',
              w.isAlwaysOnTop,
              (v) => w.isAlwaysOnTop = v,
            ),
            toggle(
              'Always on Bottom',
              w.isAlwaysOnBottom,
              (v) => w.isAlwaysOnBottom = v,
            ),
          ],
        ),
        PreferenceSection(
          label: 'Capabilities',
          children: [
            toggle('Resizable', w.isResizable, (v) => w.isResizable = v),
            toggle('Movable', w.isMovable, (v) => w.isMovable = v),
            toggle('Minimizable', w.isMinimizable, (v) => w.isMinimizable = v),
            toggle('Maximizable', w.isMaximizable, (v) => w.isMaximizable = v),
            toggle(
              'Fullscreenable',
              w.isFullScreenable,
              (v) => w.isFullScreenable = v,
            ),
            toggle('Closable', w.isClosable, (v) => w.isClosable = v),
          ],
        ),
        PreferenceSection(
          label: 'Platform specific',
          children: [
            toggle(
              'Control Buttons Visible',
              w.isWindowControlButtonsVisible,
              (v) => w.isWindowControlButtonsVisible = v,
            ),
            toggle(
              'Visible on All Workspaces',
              w.isVisibleOnAllWorkspaces,
              (v) => w.isVisibleOnAllWorkspaces = v,
            ),
            toggle(
              'Visible in Taskbar',
              w.isVisibleInTaskbar,
              (v) => w.isVisibleInTaskbar = v,
            ),
            toggle(
              'Ignore Mouse Events',
              w.isIgnoreMouseEvents,
              (v) => w.setIgnoreMouseEvents(v, false),
            ),
            toggle('Focusable', w.isFocusable, (v) => w.isFocusable = v),
          ],
        ),
      ],
    );
  }
}
