import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    hide Button, Menu, MenuItem, Preferences;

import 'pane.dart';

/// Where the window is and how big, the limits on its size, and the
/// OS-driven move and resize.
class GeometrySection extends StatelessWidget {
  const GeometrySection({super.key, required this.pane});

  final Pane pane;

  @override
  Widget build(BuildContext context) {
    final window = pane.window;
    final feedback = pane.feedback;
    final bounds = window.bounds.toRect();
    final content = window.contentBounds.toRect();
    final min = window.minimumSize.toSize();
    final max = window.maximumSize.toSize();
    // No limit reads back as zero (minimum) or as a huge value (maximum).
    String limit(Size s) =>
        (s.width <= 0 && s.height <= 0) || s.width >= 1e6 || s.height >= 1e6
        ? 'none'
        : sizeText(s.width, s.height);

    return PaneBody(
      children: [
        PreferenceSection(
          label: 'Position & size',
          children: [
            PreferenceRow(
              title: 'Size',
              subtitle: sizeText(bounds.width, bounds.height),
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: '800 × 600',
                    onTap: () {
                      window.setSize(const Size(800, 600).toNative(), false);
                      feedback('Size set to 800 × 600');
                    },
                  ),
                  ActionChip(
                    label: '1024 × 768',
                    onTap: () {
                      window.setSize(const Size(1024, 768).toNative(), false);
                      feedback('Size set to 1024 × 768');
                    },
                  ),
                ],
              ),
            ),
            PreferenceRow(
              title: 'Content size',
              subtitle: sizeText(content.width, content.height),
              trailing: ActionChip(
                label: 'Set Content 760×540',
                onTap: () {
                  window.contentSize = const Size(760, 540).toNative();
                  feedback('Content size set to 760 × 540');
                },
              ),
            ),
            PreferenceRow(
              title: 'Position',
              subtitle: '(${bounds.left.round()}, ${bounds.top.round()})',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Center',
                    onTap: () {
                      window.center();
                      feedback('Window centered');
                    },
                  ),
                  ActionChip(
                    label: '(100, 100)',
                    onTap: () {
                      window.position = const Offset(100, 100).toNative();
                      feedback('Position set to (100, 100)');
                    },
                  ),
                  ActionChip(
                    label: '(400, 300)',
                    onTap: () {
                      window.position = const Offset(400, 300).toNative();
                      feedback('Position set to (400, 300)');
                    },
                  ),
                ],
              ),
            ),
          ],
        ),
        PreferenceSection(
          label: 'Size constraints',
          children: [
            PreferenceRow(
              title: 'Minimum size',
              subtitle: limit(min),
              trailing: ActionChip(
                label: 'Set Min 400×300',
                onTap: () {
                  window.minimumSize = const Size(400, 300).toNative();
                  feedback('Minimum size set to 400 × 300');
                },
              ),
            ),
            PreferenceRow(
              title: 'Maximum size',
              subtitle: limit(max),
              trailing: ActionChip(
                label: 'Set Max 1200×900',
                onTap: () {
                  window.maximumSize = const Size(1200, 900).toNative();
                  feedback('Maximum size set to 1200 × 900');
                },
              ),
            ),
            PreferenceRow(
              title: 'Minimum and maximum',
              trailing: ActionChip(
                label: 'Reset Constraints',
                onTap: () {
                  window.minimumSize = const Size(0, 0).toNative();
                  window.maximumSize = const Size(0, 0).toNative();
                  feedback('Size constraints reset');
                },
              ),
            ),
          ],
        ),
        PreferenceSection(
          label: 'Interactions',
          footer:
              'Both hand the pointer to the window manager: move the mouse '
              'after pressing, release to stop.',
          children: [
            PreferenceRow(
              title: 'Move with the mouse',
              trailing: ActionChip(
                label: 'Start Dragging',
                onTap: () {
                  window.startDragging();
                  feedback('Drag started (move the mouse)');
                },
              ),
            ),
            PreferenceRow(
              title: 'Resize from bottom-right',
              trailing: ActionChip(
                label: 'Start Resizing',
                onTap: () {
                  window.startResizing(ResizeEdge.bottomRight);
                  feedback('Resize started from bottom-right (move the mouse)');
                },
              ),
            ),
          ],
        ),
      ],
    );
  }
}
