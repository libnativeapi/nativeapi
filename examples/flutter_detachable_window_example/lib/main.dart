// ignore_for_file: invalid_use_of_internal_member, implementation_imports

import 'dart:ui' show AppExitType;

import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:nativeapi_flutter/nativeapi_flutter.dart' as na;

import 'src/demo/panels.dart';
import 'src/detachable/detachable.dart';

void main() {
  // The stable channel does not offer `flutter config --enable-windowing`,
  // so turn the experimental windowing API on before the binding starts.
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  runWidget(const DetachableWindowApp());
}

class _MainWindowDelegate with fw.RegularWindowControllerDelegate {
  _MainWindowDelegate(this.onCloseRequested);

  final void Function(fw.RegularWindowController controller) onCloseRequested;

  @override
  void onWindowCloseRequested(fw.RegularWindowController controller) {
    onCloseRequested(controller);
  }
}

const Size _mainWindowSize = Size(840, 600);

/// A newest-first list of what the controller did, mirrored to the console.
class ActivityLog extends ValueNotifier<List<String>> {
  ActivityLog() : super(const []);

  void add(String message) {
    final now = DateTime.now();
    final stamp =
        '${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}';
    debugPrint('[detachable] $message');
    value = ['$stamp  $message', ...value.take(49)];
  }
}

class DetachableWindowApp extends StatefulWidget {
  const DetachableWindowApp({super.key});

  @override
  State<DetachableWindowApp> createState() => _DetachableWindowAppState();
}

class _DetachableWindowAppState extends State<DetachableWindowApp> {
  final _log = ActivityLog();

  late final List<HostWindow> _mainWindows = [
    _createMainWindow('A'),
    _createMainWindow('B'),
  ];

  HostWindow _createMainWindow(String name) {
    final controller = fw.RegularWindowController(
      size: _mainWindowSize,
      constraints: const BoxConstraints(minWidth: 720, minHeight: 480),
      title: 'nativeapi · Window $name',
      delegate: _MainWindowDelegate(_closeMainWindow),
    );
    return HostWindow(
      controller: controller,
      // One DazzUI host per window: each view is its own app with the
      // shared theme, toasts and localizations.
      builder: (context) => Host(
        title: 'nativeapi · Window $name',
        home: MainPage(name: name, log: _log),
      ),
    );
  }

  @override
  void initState() {
    super.initState();
    _placeMainWindowsSideBySide();
  }

  void _placeMainWindowsSideBySide() {
    final area = na.DisplayManager.instance.getPrimary()?.workArea.toRect();
    if (area == null) return;
    const gap = 24.0;
    final count = _mainWindows.length;
    final frames = [
      for (final host in _mainWindows) nativeWindowOf(host.controller),
    ];
    final width = frames.first?.bounds.width ?? _mainWindowSize.width;
    // Overlap them when the screen is too narrow to fit them all.
    final step = ((area.width - width) / (count - 1)).clamp(0.0, width + gap);
    final left = area.left + (area.width - width - step * (count - 1)) / 2;
    for (var i = 0; i < count; i++) {
      frames[i]?.position = Offset(
        left + step * i,
        area.top + 60 + 40.0 * i,
      ).toNative();
    }
  }

  /// Closing a main window pops its panels out instead of losing them; the
  /// app ends with the last main window.
  void _closeMainWindow(fw.RegularWindowController controller) {
    _controller.floatItemsInView(controller.rootView.viewId);
    _controller.unregisterHostWindow(controller);
    setState(() {
      _mainWindows.removeWhere((host) => host.controller == controller);
    });
    // Destroy only once the frame that removes its view has been built.
    SchedulerBinding.instance.addPostFrameCallback((_) {
      controller.destroy();
      if (_mainWindows.isEmpty) {
        ServicesBinding.instance.exitApplication(AppExitType.required);
      }
    });
  }

  late final DetachController _controller = DetachController(
    items: [
      DetachableItem(
        id: 'inspector',
        title: 'Inspector',
        builder: (context) => const InspectorPanel(itemId: 'inspector'),
      ),
      DetachableItem(
        id: 'stopwatch',
        title: 'Stopwatch',
        builder: (context) => const StopwatchPanel(itemId: 'stopwatch'),
      ),
    ],
    initialSlots: const {'inspector': 'A-left', 'stopwatch': 'A-bottom'},
    logger: _log.add,
  );

  @override
  void dispose() {
    _controller.dispose();
    for (final host in _mainWindows) {
      host.controller.destroy();
    }
    _log.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return DetachableWindows(
      controller: _controller,
      hosts: List.of(_mainWindows),
      floatingWindowBuilder: (context, item, content) =>
          Host(title: item.title, home: content),
    );
  }
}

/// A main window: a panel slot on each side of a workspace.
class MainPage extends StatelessWidget {
  const MainPage({super.key, required this.name, required this.log});

  final String name;
  final ActivityLog log;

  @override
  Widget build(BuildContext context) {
    final workspace = _Workspace(name: name, log: log);

    // The two windows lay their slots out differently on purpose: a panel
    // takes the size and shape of whatever slot it is docked in, and keeps
    // that size when it is torn off.
    return switch (name) {
      'A' => Row(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          SizedBox(width: 240, child: _slot('A-left', 'Sidebar')),
          const VerticalDivider(),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Expanded(child: workspace),
                const Divider(),
                SizedBox(height: 210, child: _slot('A-bottom', 'Bottom panel')),
              ],
            ),
          ),
        ],
      ),
      _ => Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          SizedBox(height: 170, child: _slot('$name-top', 'Top strip')),
          const Divider(),
          Expanded(
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Expanded(child: workspace),
                const VerticalDivider(),
                SizedBox(
                  width: 330,
                  child: _slot('$name-right', 'Wide sidebar'),
                ),
              ],
            ),
          ),
        ],
      ),
    };
  }

  static Widget _slot(String id, String label) => DockSlot(
    slotId: id,
    emptyBuilder: (context, isDropTarget) =>
        _EmptySlot(label: label, isDropTarget: isDropTarget),
  );
}

/// What a free slot looks like: its name and size on a plain fill covering the
/// whole slot, highlighted while a dragged panel would dock into it.
///
/// The drop target is part of what the example demonstrates, so it is drawn
/// here rather than taken from the kit, in the theme's colours.
class _EmptySlot extends StatelessWidget {
  const _EmptySlot({required this.label, required this.isDropTarget});

  final String label;
  final bool isDropTarget;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final dragging = DetachScope.of(context).isMovingWindow;
    final accent = isDropTarget
        ? vars.colorPrimary[600]!
        : dragging
        ? vars.colorPrimary[500]!
        : vars.colorContentSubtle;
    return LayoutBuilder(
      builder: (context, slot) => AnimatedContainer(
        duration: vars.motionDuration,
        curve: vars.motionEasing,
        // Fill the slot edge to edge, with no margin or border: the
        // highlighted area is exactly what a docked panel (and its torn-off
        // window) occupies.
        color: isDropTarget
            ? vars.colorPrimary[500]!.withValues(alpha: vars.washEdge)
            : dragging
            ? vars.colorPrimary[500]!.withValues(alpha: vars.washSurface)
            : vars.colorSurfaceSunken,
        child: Center(
          child: FittedBox(
            fit: BoxFit.scaleDown,
            child: Padding(
              padding: EdgeInsets.all(vars.spacing2),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Icon(
                    isDropTarget
                        ? FluentIcons.arrow_download_24_regular
                        : FluentIcons.board_24_regular,
                    color: accent,
                    size: 32,
                  ),
                  SizedBox(height: vars.spacing15),
                  Text(label, style: vars.titleSmall.copyWith(color: accent)),
                  Text(
                    isDropTarget
                        ? 'Release to dock'
                        : '${slot.maxWidth.round()} × '
                              '${slot.maxHeight.round()}',
                    style: vars.mono.copyWith(color: accent),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class _Workspace extends StatelessWidget {
  const _Workspace({required this.name, required this.log});

  final String name;
  final ActivityLog log;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final controller = DetachScope.of(context);

    return Padding(
      padding: EdgeInsets.all(vars.spacing5),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('Window $name', style: vars.titleLarge),
          SizedBox(height: vars.spacing1),
          Text(
            'Drag a panel by its header to pop it out into a window of its own, '
            'and onto any empty slot, in this window or the other one, to dock '
            'it there. Panels keep their state throughout: text, counters, '
            'scroll position, and the running stopwatch. Closing a window pops '
            'its panels out.',
            style: vars.bodySmall.copyWith(color: vars.colorContentMuted),
          ),
          SizedBox(height: vars.spacing3),
          Wrap(
            spacing: vars.spacing1,
            runSpacing: vars.spacing1,
            children: [
              for (final item in controller.items)
                Badge(
                  variant: BadgeVariant.tinted,
                  tint: controller.isFloating(item.id)
                      ? BadgeTint.info
                      : BadgeTint.neutral,
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    spacing: vars.spacing1,
                    children: [
                      Icon(
                        controller.isFloating(item.id)
                            ? FluentIcons.open_16_regular
                            : FluentIcons.pin_16_regular,
                        size: vars.iconSmall,
                      ),
                      Text(
                        controller.isFloating(item.id)
                            ? '${item.title}: floating'
                            : '${item.title}: docked ${controller.slotOf(item.id)}',
                      ),
                    ],
                  ),
                ),
            ],
          ),
          SizedBox(height: vars.spacing4),
          const SectionLabel('Activity'),
          SizedBox(height: vars.spacing15),
          Expanded(
            child: Card(
              variant: CardVariant.sunken,
              size: WidgetSize.small,
              child: ValueListenableBuilder<List<String>>(
                valueListenable: log,
                builder: (context, entries, _) => entries.isEmpty
                    ? Center(
                        child: Text(
                          'Nothing yet — try dragging a panel header.',
                          style: vars.muted,
                        ),
                      )
                    : ListView.builder(
                        itemCount: entries.length,
                        itemBuilder: (context, i) => Text(
                          entries[i],
                          style: vars.mono.copyWith(color: vars.colorContent),
                        ),
                      ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
