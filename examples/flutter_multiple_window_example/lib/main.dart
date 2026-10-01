// ignore_for_file: invalid_use_of_internal_member, implementation_imports

import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' hide WindowManager;
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart' hide Button;

void main() {
  // The stable channel does not offer `flutter config --enable-windowing`,
  // so turn the experimental windowing API on before the binding starts.
  isWindowingEnabled = true;
  na.Display? primaryDisplay = DisplayManager.instance.getPrimary();
  WindowManager.instance.setWillShowHook((windowId) {
    Window? window = WindowManager.instance.get(windowId);
    if (window != null && primaryDisplay != null) {
      switch (window.title) {
        case 'Primary Window':
          // Top row, centered, full width (60% of work area)
          _positionPrimaryWindow(window, primaryDisplay);
          break;
        case 'Secondary Window':
          // Bottom left (half of 60% of work area)
          _positionSecondaryWindow(window, primaryDisplay);
          break;
        case 'Tertiary Window':
          // Bottom right (half of 60% of work area)
          _positionTertiaryWindow(window, primaryDisplay);
          break;
      }
    }
    // A will-show hook replaces the show: nothing appears until it says so.
    WindowManager.instance.callOriginalShow(windowId);
  });
  WindowManager.instance.setWillHideHook((windowId) {
    // ignore: avoid_print
    print('[Dart] will hide hook $windowId');
  });
  runWidget(
    ViewCollection(
      views: [TertiaryWindow(), SecondaryWindow(), PrimaryWindow()],
    ),
  );
}

void _positionPrimaryWindow(Window window, na.Display display) {
  final workArea = display.workArea.toRect();

  // Calculate 60% of work area dimensions
  final totalWidth = workArea.width * 0.6;
  final totalHeight = workArea.height * 0.6;

  // Calculate starting position to center the layout
  final startX = workArea.left + (workArea.width - totalWidth) / 2;
  final startY = workArea.top + (workArea.height - totalHeight) / 2;

  // Top row height: 50% of total height
  final topRowHeight = totalHeight * 0.5;

  // Top row, centered, full width
  window.setSize(Size(totalWidth, topRowHeight).toNative(), false);
  window.position = Offset(startX, startY).toNative();
}

void _positionSecondaryWindow(Window window, na.Display display) {
  final workArea = display.workArea.toRect();

  // Calculate 60% of work area dimensions
  final totalWidth = workArea.width * 0.6;
  final totalHeight = workArea.height * 0.6;

  // Calculate starting position to center the layout
  final startX = workArea.left + (workArea.width - totalWidth) / 2;
  final startY = workArea.top + (workArea.height - totalHeight) / 2;

  // Top row height: 50% of total height
  final topRowHeight = totalHeight * 0.5;
  // Bottom row height: 50% of total height
  final bottomRowHeight = totalHeight * 0.5;

  // Bottom row: two windows side by side, each takes 50% width
  final bottomWindowWidth = totalWidth * 0.5;

  // Bottom left
  window.setSize(Size(bottomWindowWidth, bottomRowHeight).toNative(), false);
  window.position = Offset(startX, startY + topRowHeight).toNative();
}

void _positionTertiaryWindow(Window window, na.Display display) {
  final workArea = display.workArea.toRect();

  // Calculate 60% of work area dimensions
  final totalWidth = workArea.width * 0.6;
  final totalHeight = workArea.height * 0.6;

  // Calculate starting position to center the layout
  final startX = workArea.left + (workArea.width - totalWidth) / 2;
  final startY = workArea.top + (workArea.height - totalHeight) / 2;

  // Top row height: 50% of total height
  final topRowHeight = totalHeight * 0.5;
  // Bottom row height: 50% of total height
  final bottomRowHeight = totalHeight * 0.5;

  // Bottom row: two windows side by side, each takes 50% width
  final bottomWindowWidth = totalWidth * 0.5;

  // Bottom right
  window.setSize(Size(bottomWindowWidth, bottomRowHeight).toNative(), false);
  window.position = Offset(
    startX + bottomWindowWidth,
    startY + topRowHeight,
  ).toNative();
}

class PrimaryWindow extends StatefulWidget {
  const PrimaryWindow({super.key});

  @override
  State<PrimaryWindow> createState() => _PrimaryWindowState();
}

class _PrimaryWindowState extends State<PrimaryWindow> {
  final _windowController = RegularWindowController(
    size: const Size(800, 600),
    title: 'Primary Window',
  );

  @override
  Widget build(BuildContext context) {
    return RegularWindow(
      controller: _windowController,
      child: Host(
        title: 'Primary Window',
        home: WindowPanel(
          title: 'Primary Window',
          placement: 'Top row',
          description:
              'The will-show hook gave this window the top half of a block '
              '60% of the work area wide and tall, centred on the primary '
              'display.',
          actions: [
            Button(
              variant: ButtonVariant.filled,
              onPressed: () {
                Window? primaryWindow;
                final windows = WindowManager.instance.getAll();
                for (var window in windows) {
                  if (window.title == 'Primary Window') {
                    primaryWindow = window;
                    break;
                  }
                }
                if (primaryWindow != null) {
                  primaryWindow.setSize(
                    const Size(1000, 1000).toNative(),
                    false,
                  );
                  primaryWindow.show();
                }
              },
              child: const Text('Resize to 1000 × 1000'),
            ),
          ],
        ),
      ),
    );
  }
}

class SecondaryWindow extends StatefulWidget {
  const SecondaryWindow({super.key});

  @override
  State<SecondaryWindow> createState() => _SecondaryWindowState();
}

class _SecondaryWindowState extends State<SecondaryWindow> {
  final _windowController = RegularWindowController(
    size: const Size(800, 600),
    title: 'Secondary Window',
  );

  @override
  Widget build(BuildContext context) {
    return RegularWindow(
      controller: _windowController,
      child: const Host(
        title: 'Secondary Window',
        home: WindowPanel(
          title: 'Secondary Window',
          placement: 'Bottom left',
          description:
              'The left half of the bottom row, under the primary window.',
        ),
      ),
    );
  }
}

class TertiaryWindow extends StatefulWidget {
  const TertiaryWindow({super.key});

  @override
  State<TertiaryWindow> createState() => _TertiaryWindowState();
}

class _TertiaryWindowState extends State<TertiaryWindow> {
  final _windowController = RegularWindowController(
    size: const Size(800, 600),
    title: 'Tertiary Window',
  );

  @override
  Widget build(BuildContext context) {
    return RegularWindow(
      controller: _windowController,
      child: const Host(
        title: 'Tertiary Window',
        home: WindowPanel(
          title: 'Tertiary Window',
          placement: 'Bottom right',
          description:
              'The right half of the bottom row, under the primary window.',
        ),
      ),
    );
  }
}

/// What every window shows: its title and slot, where the hook put it, and
/// the frame nativeapi reads back for it.
class WindowPanel extends StatefulWidget {
  const WindowPanel({
    super.key,
    required this.title,
    required this.placement,
    required this.description,
    this.actions = const [],
  });

  /// The window's title, which is also how the hook and the frame read-out
  /// find its native window.
  final String title;

  /// The slot of the layout the hook put the window in.
  final String placement;

  final String description;
  final List<Widget> actions;

  @override
  State<WindowPanel> createState() => _WindowPanelState();
}

class _WindowPanelState extends State<WindowPanel> {
  Window? _nativeWindow() {
    for (final window in WindowManager.instance.getAll()) {
      if (window.title == widget.title) return window;
    }
    return null;
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    // Rebuilt whenever the view is resized, so the read-out follows the
    // window.
    MediaQuery.sizeOf(context);
    final window = _nativeWindow();
    final frame = window?.bounds.toRect();
    final value = vars.mono.copyWith(color: vars.colorContent);
    String rect(Rect r) =>
        '${r.left.round()}, ${r.top.round()}  '
        '${r.width.round()} × ${r.height.round()}';

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Padding(
          padding: EdgeInsets.symmetric(
            horizontal: vars.spacing4,
            vertical: vars.spacing25,
          ),
          child: Row(
            spacing: vars.spacing2,
            children: [
              Text(widget.title, style: vars.titleMedium),
              Badge(
                size: WidgetSize.small,
                variant: BadgeVariant.tinted,
                tint: BadgeTint.primary,
                child: Text(widget.placement),
              ),
              const Spacer(),
              ...widget.actions,
              Button(
                variant: ButtonVariant.normal,
                tint: ButtonTint.neutral,
                onPressed: () => setState(() {}),
                child: const Text('Read frame'),
              ),
            ],
          ),
        ),
        const Divider(),
        Expanded(
          child: SingleChildScrollView(
            padding: EdgeInsets.all(vars.spacing4),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              spacing: vars.spacing3,
              children: [
                Text(
                  widget.description,
                  style: vars.bodyMedium.copyWith(
                    color: vars.colorContentMuted,
                  ),
                ),
                Card(
                  variant: CardVariant.sunken,
                  size: WidgetSize.small,
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    spacing: vars.spacing1,
                    children: [
                      if (window == null || frame == null)
                        Text('No native window yet', style: vars.muted)
                      else ...[
                        Row(
                          children: [
                            SizedBox(
                              width: 72,
                              child: Text('Frame', style: vars.muted),
                            ),
                            Text(rect(frame), style: value),
                          ],
                        ),
                        Row(
                          children: [
                            SizedBox(
                              width: 72,
                              child: Text('Window id', style: vars.muted),
                            ),
                            Text('${window.id}', style: value),
                          ],
                        ),
                      ],
                    ],
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }
}
