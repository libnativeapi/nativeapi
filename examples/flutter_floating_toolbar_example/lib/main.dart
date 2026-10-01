// ignore_for_file: invalid_use_of_internal_member, implementation_imports

import 'dart:io' show Platform;
import 'dart:ui' show AppExitType;

import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:nativeapi_flutter/nativeapi_flutter.dart' as na;
import 'package:nativeapi_flutter/windowing.dart';

void main() {
  // The stable channel does not offer `flutter config --enable-windowing`,
  // so turn the experimental windowing API on before the binding starts.
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  runWidget(const FloatingToolbarApp());
}

const Size _mainWindowSize = Size(720, 520);
const Size _toolbarSize = Size(380, 64);

/// How far the toolbar floats above the top edge of the main window.
const double _toolbarGap = 10;

/// The colours the toolbar offers: ramps of the theme, so each window
/// resolves the same pick in its own palette.
enum Swatch {
  indigo('Primary'),
  green('Green'),
  amber('Amber'),
  red('Red');

  const Swatch(this.label);

  final String label;

  Color resolve(ThemeVariables vars) => switch (this) {
    Swatch.indigo => vars.colorPrimary,
    Swatch.green => vars.colorSuccess,
    Swatch.amber => vars.colorWarning,
    Swatch.red => vars.colorDanger,
  }[500]!;
}

/// On Wayland an application can neither place its top-level windows nor find out
/// where they are, so nothing here can make the toolbar follow: it stays where the
/// desktop puts it, and the user drags it by the pill instead.
final bool canPlaceWindows =
    !(Platform.isLinux &&
        Platform.environment.containsKey('WAYLAND_DISPLAY') &&
        Platform.environment['GDK_BACKEND'] != 'x11');

/// What both windows show. They run in one isolate, so a plain [ChangeNotifier]
/// is all the "communication between windows" there is.
class ToolbarModel extends ChangeNotifier {
  Swatch color = Swatch.values.first;
  int stamps = 0;
  bool attached = true;
  bool toolbarVisible = true;
  final List<String> log = [];

  void pick(Swatch value) {
    color = value;
    notifyListeners();
  }

  void stamp() {
    stamps++;
    notifyListeners();
  }

  void note(String message) {
    debugPrint('[floating_toolbar] $message');
    log.insert(0, message);
    if (log.length > 40) log.removeLast();
    notifyListeners();
  }

  void clearLog() {
    log.clear();
    notifyListeners();
  }

  void update(VoidCallback change) {
    change();
    notifyListeners();
  }
}

class _CloseDelegate with fw.RegularWindowControllerDelegate {
  _CloseDelegate(this.onCloseRequested);

  final VoidCallback onCloseRequested;

  @override
  void onWindowCloseRequested(fw.RegularWindowController controller) =>
      onCloseRequested();
}

class FloatingToolbarApp extends StatefulWidget {
  const FloatingToolbarApp({super.key});

  @override
  State<FloatingToolbarApp> createState() => _FloatingToolbarAppState();
}

class _FloatingToolbarAppState extends State<FloatingToolbarApp> {
  final _model = ToolbarModel();

  late final fw.RegularWindowController _mainController =
      fw.RegularWindowController(
        size: _mainWindowSize,
        constraints: const BoxConstraints(minWidth: 480, minHeight: 360),
        title: 'Floating toolbar',
        delegate: _CloseDelegate(_closeEverything),
      );

  late final fw.RegularWindowController _toolbarController =
      fw.RegularWindowController(
        size: _toolbarSize,
        title: 'Toolbar',
        // The toolbar has no close button; closing the main window closes it.
        delegate: _CloseDelegate(() {}),
      );

  na.Window? get _main => _mainController.nativeWindow;
  na.Window? get _toolbar => _toolbarController.nativeWindow;

  int? _listenerId;
  bool _closing = false;

  @override
  void initState() {
    super.initState();

    final main = _main;
    final area = na.DisplayManager.instance.getPrimary()?.workArea.toRect();
    if (main != null && area != null) {
      // Leave room above the window for the toolbar.
      main.position = Offset(
        area.left + (area.width - _mainWindowSize.width) / 2,
        area.top + 120,
      ).toNative();
    }

    final toolbar = _toolbar;
    if (toolbar != null) {
      // Everything that makes a window a floating toolbar, from Dart.
      toolbar.titleBarStyle = na.TitleBarStyle.hidden;
      toolbar.backgroundColor = const Color(0x00000000).toNative();
      toolbar.hasShadow = false;
      toolbar.isResizable = false;
      toolbar.isMovable = false;
      toolbar.isVisibleInTaskbar = false;
      // Hiding the title bar keeps the frame; give the content its size back.
      toolbar.contentSize = _toolbarSize.toNative();
      _attach();
    }

    _listenerId = na.WindowManager.instance.addListener(_onWindowEvent);
  }

  void _attach() {
    final ok = _toolbar?.setParentWindow(_main) ?? false;
    if (ok) _placeToolbar();
    _model.update(() => _model.attached = ok);
    _model.note(
      ok ? 'Toolbar attached to the main window' : 'setParentWindow failed',
    );
  }

  void _detach() {
    _toolbar?.setParentWindow(null);
    _model.update(() => _model.attached = false);
    _model.note('Toolbar detached: it no longer follows');
  }

  /// Centres the toolbar above the main window.
  ///
  /// On macOS a child window already moves with its parent; elsewhere this is
  /// what makes it follow. It also keeps the toolbar centred when the main
  /// window is resized, which no platform does by itself.
  void _placeToolbar() {
    final main = _main;
    final toolbar = _toolbar;
    if (main == null || toolbar == null) return;
    final frame = main.bounds.toRect();
    final size = toolbar.bounds.toRect().size;
    toolbar.position = Offset(
      frame.left + (frame.width - size.width) / 2,
      frame.top - size.height - _toolbarGap,
    ).toNative();
  }

  void _onWindowEvent(na.WindowEvent event) {
    if (_closing) return;
    final mainId = _main?.id;
    final toolbarId = _toolbar?.id;
    final name = event.windowId == mainId
        ? 'main'
        : event.windowId == toolbarId
        ? 'toolbar'
        : '#${event.windowId}';
    switch (event) {
      case na.WindowMovedEvent() || na.WindowResizedEvent():
        if (event.windowId == mainId && _model.attached) _placeToolbar();
      case na.WindowCreatedEvent():
        _model.note('created: $name');
      case na.WindowClosedEvent():
        _model.note('closed: $name');
      case na.WindowMinimizedEvent():
        _model.note('minimized: $name');
      case na.WindowRestoredEvent():
        _model.note('restored: $name');
        if (event.windowId == mainId && _model.attached) _placeToolbar();
      default:
        break;
    }
  }

  void _setToolbarVisible(bool visible) {
    final toolbar = _toolbar;
    if (toolbar == null) return;
    if (visible) {
      if (_model.attached) _placeToolbar();
      toolbar.showInactive();
    } else {
      toolbar.hide();
    }
    _model.update(() => _model.toolbarVisible = visible);
    _model.note(visible ? 'Toolbar shown' : 'Toolbar hidden');
  }

  /// Children first, then the parent: what closing a parent does to its
  /// children differs between platforms, closing them yourself does not.
  void _closeEverything() {
    if (_closing) return;
    _closing = true;
    final listenerId = _listenerId;
    if (listenerId != null) {
      na.WindowManager.instance.removeListener(listenerId);
    }
    _toolbar?.setParentWindow(null);
    setState(() {});
    // Destroy only once the frame that removes the views has been built.
    SchedulerBinding.instance.addPostFrameCallback((_) {
      _toolbarController.destroy();
      _mainController.destroy();
      ServicesBinding.instance.exitApplication(AppExitType.required);
    });
  }

  @override
  Widget build(BuildContext context) {
    if (_closing) return const ViewCollection(views: []);
    return ViewCollection(
      views: [
        fw.RegularWindow(
          controller: _mainController,
          child: Host(
            title: 'Floating toolbar',
            home: MainPage(
              model: _model,
              onAttach: _attach,
              onDetach: _detach,
              onToolbarVisible: _setToolbarVisible,
            ),
          ),
        ),
        fw.RegularWindow(
          controller: _toolbarController,
          // Not a Host: that paints the canvas under everything, and here
          // nothing opaque may sit between the pill and the desktop.
          child: Host(
            transparent: true,
            title: 'Toolbar',
            home: ToolbarPage(
              model: _model,
              // Where the app cannot place the toolbar, the user can.
              onDrag: canPlaceWindows ? null : () => _toolbar?.startDragging(),
            ),
          ),
        ),
      ],
    );
  }
}

class MainPage extends StatelessWidget {
  const MainPage({
    super.key,
    required this.model,
    required this.onAttach,
    required this.onDetach,
    required this.onToolbarVisible,
  });

  final ToolbarModel model;
  final VoidCallback onAttach;
  final VoidCallback onDetach;
  final ValueChanged<bool> onToolbarVisible;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return ListenableBuilder(
      listenable: model,
      builder: (context, _) => Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Expanded(
            child: SingleChildScrollView(
              padding: EdgeInsets.all(vars.spacing4),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  Text(
                    'The pill above this window is a second Flutter window: '
                    'transparent, frameless, and a child of this one. Move, '
                    'resize or minimize this window and it comes along.',
                    style: vars.bodyMedium.copyWith(
                      color: vars.colorContentSubtle,
                    ),
                  ),
                  if (!canPlaceWindows) ...[
                    SizedBox(height: vars.spacing3),
                    const Callout(
                      tint: CalloutTint.warning,
                      icon: Icon(FluentIcons.warning_20_regular),
                      title: Text('Wayland'),
                      message: Text(
                        'Applications cannot place their windows here, so the '
                        'pill cannot follow this window. It stays above it and '
                        'shares its state; drag the pill to put it where you '
                        'want it.',
                      ),
                    ),
                  ],
                  SizedBox(height: vars.spacing4),
                  _sharedState(vars),
                  SizedBox(height: vars.spacing4),
                  _controls(vars),
                ],
              ),
            ),
          ),
          EventFooter(
            headline: model.log.isEmpty ? 'No events yet' : model.log.first,
            lines: model.log.skip(1),
            visibleLines: 8,
            height: 184,
            onClear: model.log.isEmpty ? null : model.clearLog,
          ),
        ],
      ),
    );
  }

  /// What the toolbar changes, read back in this window.
  Widget _sharedState(ThemeVariables vars) {
    return Card(
      variant: CardVariant.raised,
      child: Row(
        children: [
          AnimatedContainer(
            duration: vars.motionDuration,
            curve: vars.motionEasing,
            width: 56,
            height: 56,
            decoration: BoxDecoration(
              color: model.color.resolve(vars),
              borderRadius: BorderRadius.circular(vars.radiusMedium),
            ),
          ),
          SizedBox(width: vars.spacing4),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                const SectionLabel('Set from the toolbar'),
                SizedBox(height: vars.spacing05),
                Text(
                  'Stamps: ${model.stamps}',
                  key: const ValueKey('stamps'),
                  style: vars.titleLarge,
                ),
                Text('Colour: ${model.color.label}', style: vars.muted),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _controls(ThemeVariables vars) {
    Widget status(bool on, String yes, String no) => Badge(
      size: WidgetSize.small,
      variant: on ? BadgeVariant.tinted : BadgeVariant.outlined,
      tint: on ? BadgeTint.success : BadgeTint.neutral,
      child: Text(on ? yes : no),
    );

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        const SectionLabel('Toolbar window'),
        SizedBox(height: vars.spacing2),
        Row(
          children: [
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: model.attached ? onDetach : onAttach,
              child: Row(
                mainAxisSize: MainAxisSize.min,
                spacing: vars.spacing15,
                children: [
                  Icon(
                    model.attached
                        ? FluentIcons.link_dismiss_20_regular
                        : FluentIcons.link_20_regular,
                  ),
                  Text(model.attached ? 'Detach toolbar' : 'Attach toolbar'),
                ],
              ),
            ),
            SizedBox(width: vars.spacing2),
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => onToolbarVisible(!model.toolbarVisible),
              child: Row(
                mainAxisSize: MainAxisSize.min,
                spacing: vars.spacing15,
                children: [
                  Icon(
                    model.toolbarVisible
                        ? FluentIcons.eye_off_20_regular
                        : FluentIcons.eye_20_regular,
                  ),
                  Text(model.toolbarVisible ? 'Hide toolbar' : 'Show toolbar'),
                ],
              ),
            ),
            const Spacer(),
            status(model.attached, 'attached', 'detached'),
            SizedBox(width: vars.spacing1),
            status(model.toolbarVisible, 'shown', 'hidden'),
          ],
        ),
      ],
    );
  }
}

/// The floating pill: the subject of the example, drawn here from the theme,
/// with DazzUI controls on it.
class ToolbarPage extends StatelessWidget {
  const ToolbarPage({super.key, required this.model, this.onDrag});

  final ToolbarModel model;

  /// Starts a window drag from the pill; null where the app places the toolbar itself.
  final VoidCallback? onDrag;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return ListenableBuilder(
      listenable: model,
      builder: (context, _) => Center(
        child: GestureDetector(
          onPanStart: onDrag == null ? null : (_) => onDrag!(),
          child: Container(
            height: 48,
            padding: EdgeInsets.symmetric(horizontal: vars.spacing2),
            decoration: BoxDecoration(
              color: vars.colorSurfaceRaised,
              border: Border.all(
                color: vars.colorBorderStrong,
                width: context.hairlineWidth,
              ),
              borderRadius: BorderRadius.circular(vars.radiusFull),
              // The window has no shadow of its own (it would outline the
              // whole transparent rectangle), so the pill carries a small one.
              boxShadow: vars.shadowXs,
            ),
            child: Row(
              mainAxisSize: MainAxisSize.min,
              spacing: vars.spacing05,
              children: [
                for (final swatch in Swatch.values)
                  Toggle(
                    size: WidgetSize.small,
                    pressed: model.color == swatch,
                    semanticsLabel: swatch.label,
                    onPressedChanged: (_) => model.pick(swatch),
                    child: Icon(
                      FluentIcons.circle_20_filled,
                      color: swatch.resolve(vars),
                    ),
                  ),
                SizedBox(
                  height: vars.spacing5,
                  child: VerticalDivider(width: vars.spacing3),
                ),
                Button(
                  variant: ButtonVariant.filled,
                  onPressed: model.stamp,
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    spacing: vars.spacing15,
                    children: const [
                      Icon(FluentIcons.ribbon_star_20_regular),
                      Text('Stamp'),
                    ],
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
