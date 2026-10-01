import 'dart:async';

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    hide Button, Menu, MenuItem, Preferences;

import 'event_log.dart';
import 'sections/appearance_section.dart';
import 'sections/behaviour_section.dart';
import 'sections/canvas_section.dart';
import 'sections/events_section.dart';
import 'sections/geometry_section.dart';
import 'sections/pane.dart';
import 'sections/state_section.dart';

// A control panel for the Window API, drawn with DazzUI over the shared host
// (dazzui_host). The shell here tracks the windows and the log; each page of
// the sidebar is one file:
//
//   sections/canvas_section.dart      map of displays and windows, read-outs
//   sections/state_section.dart       show / hide, maximize / minimize, focus
//   sections/geometry_section.dart    size, position, limits, drag and resize
//   sections/appearance_section.dart  title bar, shadow, opacity, effects
//   sections/behaviour_section.dart   stacking, capabilities, platform flags
//   sections/events_section.dart      the WindowManager event log

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'Window Example', home: WindowManagerPage());
}

enum _Page {
  canvas('Canvas', FluentIcons.grid_20_regular),
  state('State', FluentIcons.window_20_regular),
  geometry('Geometry', FluentIcons.resize_20_regular),
  appearance('Appearance', FluentIcons.paint_brush_20_regular),
  behaviour('Behaviour', FluentIcons.options_20_regular),
  events('Events', FluentIcons.text_bullet_list_ltr_20_regular);

  const _Page(this.label, this.icon);

  final String label;
  final IconData icon;

  /// The pages about the selected window.
  bool get needsWindow =>
      this == state ||
      this == geometry ||
      this == appearance ||
      this == behaviour;
}

class WindowManagerPage extends StatefulWidget {
  const WindowManagerPage({super.key});

  @override
  State<WindowManagerPage> createState() => _WindowManagerPageState();
}

class _WindowManagerPageState extends State<WindowManagerPage> {
  // --- data ---
  List<Window> _windows = [];
  List<na.Display> _displays = [];
  Window? _selectedWindow;
  bool _isLoading = true;
  String? _errorMessage;
  Timer? _updateTimer;
  final List<int> _windowListenerIds = [];

  // --- event log ---
  final EventLog _eventLog = EventLog();

  // --- navigation ---
  _Page _page = _Page.canvas;

  // --- action feedback ---
  int? _toastId;

  @override
  void initState() {
    super.initState();
    _loadDisplays();
    _loadWindows();
    _startTracking();
  }

  @override
  void dispose() {
    _updateTimer?.cancel();
    for (final id in _windowListenerIds) {
      WindowManager.instance.removeListener(id);
    }
    _windowListenerIds.clear();
    super.dispose();
  }

  // -----------------------------------------------------------------------
  // Feedback
  // -----------------------------------------------------------------------

  /// One toast at a time: a new one replaces the last. Also rebuilds, so the
  /// controls show what the window's getters return after the action.
  void _showFeedback(String message, {bool ok = true}) {
    if (!mounted) return;
    final toaster = Toaster.of(context);
    if (_toastId != null) toaster.close(_toastId);
    _toastId = toaster.add(
      ToastOptions(
        title: message,
        tint: ok ? ToastTint.success : ToastTint.warning,
        timeout: const Duration(seconds: 3),
      ),
    );
    setState(() {});
  }

  void _addLog(
    String message, {
    LogTone tone = LogTone.neutral,
    String? replaceTag,
  }) {
    setState(() => _eventLog.add(message, tone: tone, replaceTag: replaceTag));
  }

  // -----------------------------------------------------------------------
  // Tracking
  // -----------------------------------------------------------------------
  void _startTracking() {
    _updateTimer = Timer.periodic(const Duration(milliseconds: 500), (_) {
      if (mounted) _updateWindows();
    });

    _windowListenerIds.add(
      WindowManager.instance.addListener((event) {
        if (event is WindowFocusedEvent) {
          _addLog('Window #${event.windowId} focused');
          _updateWindows();
        }
        if (event is WindowBlurredEvent) {
          _addLog('Window #${event.windowId} blurred');
          _updateWindows();
        }
        if (event is WindowMinimizedEvent) {
          _addLog('Window #${event.windowId} minimized');
          _updateWindows();
        }
        if (event is WindowMaximizedEvent) {
          _addLog('Window #${event.windowId} maximized');
          _updateWindows();
        }
        if (event is WindowRestoredEvent) {
          _addLog('Window #${event.windowId} restored');
          _updateWindows();
        }
        if (event is WindowEnteredFullScreenEvent) {
          _addLog('Window #${event.windowId} entered full screen');
          _updateWindows();
        }
        if (event is WindowExitedFullScreenEvent) {
          _addLog('Window #${event.windowId} exited full screen');
          _updateWindows();
        }
        if (event is WindowCreatedEvent) {
          _addLog('Window #${event.windowId} created (first shown)');
          _updateWindows();
        }
        if (event is WindowClosedEvent) {
          _addLog('Window #${event.windowId} closed');
          _updateWindows();
        }
        if (event is WindowMovedEvent) {
          final p = event.newPosition;
          _addLog(
            'Window #${event.windowId} moved to '
            '${p.x.round()}, ${p.y.round()}',
            replaceTag: 'moved-${event.windowId}',
          );
        }
        if (event is WindowResizedEvent) {
          final s = event.newSize;
          _addLog(
            'Window #${event.windowId} resized to '
            '${s.width.round()} x ${s.height.round()}',
            replaceTag: 'resized-${event.windowId}',
          );
        }
      }),
    );
  }

  void _updateWindows() {
    try {
      final windows = WindowManager.instance.getAll();
      if (!mounted) return;
      setState(() {
        _windows = windows;
        if (_selectedWindow != null) {
          final updated = windows.where((w) => w.id == _selectedWindow!.id);
          _selectedWindow = updated.isNotEmpty ? updated.first : null;
        }
      });
    } catch (_) {}
  }

  // -----------------------------------------------------------------------
  // Loading
  // -----------------------------------------------------------------------
  Future<void> _loadDisplays() async {
    try {
      final displays = DisplayManager.instance.getAll();
      if (mounted) setState(() => _displays = displays);
    } catch (_) {}
  }

  Future<void> _loadWindows() async {
    setState(() {
      _isLoading = true;
      _errorMessage = null;
    });
    try {
      final windows = WindowManager.instance.getAll();
      if (mounted) {
        setState(() {
          _windows = windows;
          _isLoading = false;
          // Start on this app's own window, so every page has a subject.
          if (_selectedWindow == null && windows.isNotEmpty) {
            final current = WindowManager.instance.getCurrent();
            _selectedWindow = windows.firstWhere(
              (w) => w.id == current?.id,
              orElse: () => windows.first,
            );
          }
        });
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          _isLoading = false;
          _errorMessage = 'Failed to load windows: $e';
        });
      }
    }
  }

  // -----------------------------------------------------------------------
  // Selection
  // -----------------------------------------------------------------------
  void _selectWindow(Window w) {
    setState(() => _selectedWindow = w);
  }

  void _forAll(void Function(Window w) action) {
    for (final w in _windows) {
      try {
        action(w);
      } catch (_) {}
    }
  }

  // -----------------------------------------------------------------------
  // Build
  // -----------------------------------------------------------------------
  @override
  Widget build(BuildContext context) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _sidebar(context.vars),
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              _toolbar(context.vars),
              const Divider(),
              Expanded(child: _body()),
              // The Events page shows the whole log instead.
              if (_page != _Page.events)
                EventFooter(
                  headline: _eventLog.isEmpty
                      ? 'No events yet'
                      : _eventLog.entries.first.message,
                  lines: _eventLog.entries.skip(1).map((e) => e.message),
                  onClear: () => setState(_eventLog.clear),
                  visibleLines: 2,
                  height: 76,
                ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _sidebar(ThemeVariables vars) {
    NavItem item(_Page page, {Widget? trailing}) => NavItem(
      label: page.label,
      icon: page.icon,
      current: _page == page,
      trailing: trailing,
      onPressed: () => setState(() => _page = page),
    );

    return Sidebar(
      header: Row(
        spacing: vars.spacing2,
        children: [
          Icon(
            FluentIcons.window_multiple_20_regular,
            size: vars.iconMedium,
            color: vars.colorContent,
          ),
          Expanded(
            child: Text(
              'Windows',
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.titleSmall.copyWith(color: vars.colorContent),
            ),
          ),
          if (_windows.isNotEmpty)
            Badge(
              size: WidgetSize.small,
              tint: BadgeTint.primary,
              child: Text('${_windows.length}'),
            ),
        ],
      ),
      children: [
        SidebarGroup(children: [item(_Page.canvas)]),
        SidebarGroup(
          label: 'Selected window',
          children: [
            item(_Page.state),
            item(_Page.geometry),
            item(_Page.appearance),
            item(_Page.behaviour),
          ],
        ),
        SidebarGroup(
          label: 'Log',
          children: [
            item(
              _Page.events,
              trailing: _eventLog.isEmpty
                  ? null
                  : Text(
                      '${_eventLog.length}',
                      style: vars.labelSmall.copyWith(
                        color: _page == _Page.events
                            ? vars.colorOnAccent
                            : vars.colorContentFaint,
                      ),
                    ),
            ),
          ],
        ),
      ],
    );
  }

  Widget _toolbar(ThemeVariables vars) {
    final selected = _selectedWindow;
    String windowLabel(Window w) {
      final title = w.title ?? '';
      return title.isEmpty ? 'Untitled (#${w.id})' : title;
    }

    return Container(
      height: vars.frameTitlebarSize,
      padding: EdgeInsets.symmetric(horizontal: vars.spacing4),
      child: Row(
        spacing: vars.spacing2,
        children: [
          Expanded(
            child: Text(
              _page.label,
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.titleMedium.copyWith(color: vars.colorContent),
            ),
          ),
          if (_windows.isNotEmpty)
            SizedBox(
              width: 180,
              child: Select<int>(
                size: WidgetSize.small,
                placeholder: 'Select a window',
                options: [
                  for (final w in _windows)
                    SelectOption(value: w.id, label: windowLabel(w)),
                ],
                value: selected?.id,
                onChanged: (id) {
                  final w = _windows.where((w) => w.id == id).firstOrNull;
                  if (w != null) _selectWindow(w);
                },
              ),
            ),
          Menu(
            trigger: (context, state) => Tooltip(
              label: 'All windows',
              child: IconButton(
                icon: const Icon(FluentIcons.more_horizontal_20_regular),
                semanticsLabel: 'All windows',
                onPressed: state.toggle,
              ),
            ),
            items: [
              MenuItem(
                label: 'Minimize All',
                onSelect: () {
                  _forAll((w) => w.minimize());
                  _addLog(
                    'Action: minimize all windows',
                    tone: LogTone.warning,
                  );
                  _showFeedback('Minimized all windows');
                },
              ),
              MenuItem(
                label: 'Restore All',
                onSelect: () {
                  _forAll((w) => w.restore());
                  _addLog('Action: restore all windows', tone: LogTone.info);
                  _showFeedback('Restored all windows');
                },
              ),
              MenuItem(
                label: 'Show All',
                onSelect: () {
                  _forAll((w) => w.show());
                  _addLog('Action: show all windows', tone: LogTone.success);
                  _showFeedback('Showed all windows');
                },
              ),
              MenuItem(
                label: 'Hide All',
                onSelect: () {
                  _forAll((w) => w.hide());
                  _addLog('Action: hide all windows');
                  _showFeedback('Hidden all windows');
                },
              ),
            ],
          ),
          Tooltip(
            label: 'Refresh Windows',
            child: IconButton(
              icon: const Icon(FluentIcons.arrow_clockwise_20_regular),
              semanticsLabel: 'Refresh Windows',
              onPressed: _loadWindows,
            ),
          ),
        ],
      ),
    );
  }

  Widget _body() {
    if (_isLoading) {
      return Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          spacing: context.vars.spacing3,
          children: [
            const Spinner(),
            Text('Loading windows...', style: context.vars.muted),
          ],
        ),
      );
    }

    if (_errorMessage != null) {
      return Center(
        child: Padding(
          padding: EdgeInsets.all(context.vars.spacing6),
          child: Callout(
            tint: CalloutTint.danger,
            title: const Text('Could not list the windows'),
            message: Text(_errorMessage!),
            actions: [
              Button(
                variant: ButtonVariant.filled,
                onPressed: _loadWindows,
                child: const Text('Retry'),
              ),
            ],
          ),
        ),
      );
    }

    if (_page == _Page.events) {
      return EventsSection(
        log: _eventLog,
        onClear: () => setState(_eventLog.clear),
      );
    }

    if (_windows.isEmpty) {
      return const Center(
        child: EmptyState(
          title:
              'No windows found.\nOpen or create a window to begin exploring '
              'the API.',
        ),
      );
    }

    final window = _selectedWindow;
    final pane = window == null
        ? null
        : Pane(window: window, feedback: _showFeedback, log: _addLog);

    if (_page == _Page.canvas) {
      return CanvasSection(
        windows: _windows,
        displays: _displays,
        pane: pane,
        onSelect: _selectWindow,
        onMoreActions: () => setState(() => _page = _Page.state),
      );
    }

    if (pane == null) {
      return Center(
        child: EmptyState(
          title: 'Select a window on the Canvas page',
          actions: [
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => setState(() => _page = _Page.canvas),
              child: const Text('Open Canvas'),
            ),
          ],
        ),
      );
    }

    assert(_page.needsWindow);
    // Keyed by window, so a page's own state (a slider mid-drag) does not
    // carry over to another window.
    final key = ValueKey(pane.window.id);
    return switch (_page) {
      _Page.state => StateSection(key: key, pane: pane),
      _Page.geometry => GeometrySection(key: key, pane: pane),
      _Page.appearance => AppearanceSection(key: key, pane: pane),
      _Page.behaviour => BehaviourSection(key: key, pane: pane),
      _Page.canvas || _Page.events => const SizedBox.shrink(),
    };
  }
}
