import 'package:dazzui_host/dazzui_host.dart';

import '../event_log.dart';
import '../widgets/state_badges.dart';
import 'pane.dart';

/// Visibility, maximized / minimized / full screen, and focus.
class StateSection extends StatelessWidget {
  const StateSection({super.key, required this.pane});

  final Pane pane;

  @override
  Widget build(BuildContext context) {
    final window = pane.window;
    final feedback = pane.feedback;
    final log = pane.log;

    return PaneBody(
      children: [
        PreferenceSection(
          label: 'Now',
          children: [StateBadges(window: window)],
        ),
        PreferenceSection(
          label: 'Visibility',
          children: [
            PreferenceRow(
              title: 'Visible',
              subtitle: window.isVisible ? 'Shown' : 'Hidden',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Show',
                    onTap: () {
                      window.show();
                      feedback('Window shown');
                      log('Action: show #${window.id}', tone: LogTone.success);
                    },
                  ),
                  ActionChip(
                    label: 'Show Inactive',
                    onTap: () {
                      window.showInactive();
                      feedback('Window shown (inactive)');
                    },
                  ),
                  ActionChip(
                    label: 'Hide',
                    onTap: () {
                      window.hide();
                      feedback('Window hidden');
                      log('Action: hide #${window.id}', tone: LogTone.warning);
                    },
                  ),
                ],
              ),
            ),
          ],
        ),
        PreferenceSection(
          label: 'Window state',
          children: [
            PreferenceRow(
              title: 'Maximized',
              subtitle: window.isMaximized ? 'Yes' : 'No',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Maximize',
                    onTap: () {
                      window.maximize();
                      feedback('Window maximized');
                      log(
                        'Action: maximize #${window.id}',
                        tone: LogTone.primary,
                      );
                    },
                  ),
                  ActionChip(
                    label: 'Unmaximize',
                    onTap: () {
                      window.unmaximize();
                      feedback('Window unmaximized');
                    },
                  ),
                ],
              ),
            ),
            PreferenceRow(
              title: 'Minimized',
              subtitle: window.isMinimized ? 'Yes' : 'No',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Minimize',
                    onTap: () {
                      window.minimize();
                      feedback('Window minimized');
                      log(
                        'Action: minimize #${window.id}',
                        tone: LogTone.warning,
                      );
                    },
                  ),
                  ActionChip(
                    label: 'Restore',
                    onTap: () {
                      window.restore();
                      feedback('Window restored');
                      log('Action: restore #${window.id}', tone: LogTone.info);
                    },
                  ),
                ],
              ),
            ),
            SwitchRow(
              title: 'Full screen',
              value: window.isFullScreen,
              onChanged: (v) {
                window.isFullScreen = v;
                feedback(
                  window.isFullScreen ? 'Fullscreen on' : 'Fullscreen off',
                );
              },
            ),
          ],
        ),
        PreferenceSection(
          label: 'Focus',
          children: [
            PreferenceRow(
              title: 'Focused',
              subtitle: window.isFocused ? 'Yes' : 'No',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Focus',
                    onTap: () {
                      window.focus();
                      feedback('Window focused');
                    },
                  ),
                  ActionChip(
                    label: 'Blur',
                    onTap: () {
                      window.blur();
                      feedback('Window blurred');
                    },
                  ),
                ],
              ),
            ),
          ],
        ),
      ],
    );
  }
}
