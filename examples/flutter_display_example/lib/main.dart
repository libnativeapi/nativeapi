import 'dart:async';

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show
        DisplayManager,
        NativeSizeToSize,
        PointToOffset,
        RectangleToRect,
        SizeToNative,
        Window,
        WindowFocusedEvent,
        WindowManager,
        WindowMovedEvent,
        WindowResizedEvent;

// Every display DisplayManager reports, laid out the way the system arranges
// them — with this window and the cursor drawn on top, live — then the same
// displays as a table, and the selected one's read-outs on the right.

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'Display Example', home: DisplayManagerPage());
}

class DisplayManagerPage extends StatefulWidget {
  const DisplayManagerPage({super.key});

  @override
  State<DisplayManagerPage> createState() => _DisplayManagerPageState();
}

class _DisplayManagerPageState extends State<DisplayManagerPage> {
  List<na.Display> _displays = [];
  na.Display? _selectedDisplay;
  bool _isLoading = true;
  String? _errorMessage;
  Window? _currentWindow;
  Offset _cursorPosition = Offset.zero;
  Timer? _updateTimer;
  final List<int> _windowListenerIds = [];

  @override
  void initState() {
    super.initState();
    _loadDisplays();
    _startTracking();
    // A desktop tool's size, whatever the runner's default is.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      WindowManager.instance.getCurrent()?.contentSize = const Size(
        980,
        660,
      ).toNative();
    });
  }

  @override
  void dispose() {
    _updateTimer?.cancel();
    for (final listenerId in _windowListenerIds) {
      WindowManager.instance.removeListener(listenerId);
    }
    _windowListenerIds.clear();
    super.dispose();
  }

  void _startTracking() {
    // Update cursor position and current window periodically
    _updateTimer = Timer.periodic(const Duration(milliseconds: 100), (timer) {
      if (mounted) {
        _updateCursorAndWindow();
      }
    });

    // One listener per emitter now, with the concrete event carried in the
    // payload rather than selected by a type argument.
    _windowListenerIds.add(
      WindowManager.instance.addListener((event) {
        if (event is! WindowFocusedEvent &&
            event is! WindowMovedEvent &&
            event is! WindowResizedEvent) {
          return;
        }
        if (mounted) {
          _updateCurrentWindow();
        }
      }),
    );
  }

  void _updateCursorAndWindow() {
    final cursorPos = DisplayManager.instance.getCursorPosition();

    final currentWindow = WindowManager.instance.getCurrent();

    if (mounted) {
      setState(() {
        _cursorPosition = cursorPos.toOffset();
        _currentWindow = currentWindow;
      });
    }
  }

  void _updateCurrentWindow() {
    final currentWindow = WindowManager.instance.getCurrent();

    if (mounted) {
      setState(() {
        _currentWindow = currentWindow;
      });
    }
  }

  Future<void> _loadDisplays({bool announce = false}) async {
    setState(() {
      _isLoading = true;
      _errorMessage = null;
    });

    try {
      final displays = DisplayManager.instance.getAll();

      setState(() {
        _displays = displays;
        _isLoading = false;
        // Keep the selection across a refresh when that display is still
        // there; otherwise auto-select the primary display.
        final selectedId = _selectedDisplay?.id;
        _selectedDisplay = null;
        if (displays.isNotEmpty) {
          _selectedDisplay = displays.firstWhere(
            (d) => d.id == selectedId,
            orElse: () => displays.firstWhere(
              (d) => d.isPrimary,
              orElse: () => displays.first,
            ),
          );
        }
      });
      if (announce && mounted) {
        Toaster.of(context).add(
          ToastOptions(
            title:
                'Found ${displays.length} display${displays.length != 1 ? 's' : ''}',
            tint: ToastTint.success,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _isLoading = false;
        _errorMessage = 'Failed to load displays: $e';
      });
    }
  }

  void _selectDisplay(na.Display display) {
    setState(() {
      _selectedDisplay = display;
    });
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _toolbar(vars),
        const Divider(),
        Expanded(child: _buildBody(vars)),
        _statusBar(vars),
      ],
    );
  }

  Widget _toolbar(ThemeVariables vars) {
    return Container(
      height: vars.frameTitlebarSize,
      padding: EdgeInsets.symmetric(horizontal: vars.spacing4),
      child: Row(
        spacing: vars.spacing2,
        children: [
          Icon(
            FluentIcons.desktop_20_regular,
            size: vars.iconLarge,
            color: vars.colorContentSecondary,
          ),
          Text('Display Example', style: vars.titleMedium),
          if (_displays.isNotEmpty)
            Badge(
              size: WidgetSize.small,
              variant: BadgeVariant.tinted,
              tint: BadgeTint.primary,
              child: Text(
                '${_displays.length} display${_displays.length != 1 ? 's' : ''}',
              ),
            ),
          const Spacer(),
          Tooltip(
            label: 'Refresh Displays',
            child: Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => _loadDisplays(announce: true),
              child: Row(
                mainAxisSize: MainAxisSize.min,
                spacing: vars.spacing1,
                children: const [
                  Icon(FluentIcons.arrow_clockwise_20_regular),
                  Text('Refresh'),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildBody(ThemeVariables vars) {
    if (_isLoading) {
      return Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          spacing: vars.spacing3,
          children: [
            const Spinner(),
            Text('Loading displays...', style: vars.muted),
          ],
        ),
      );
    }

    if (_errorMessage != null) {
      return Padding(
        padding: EdgeInsets.all(vars.spacing4),
        child: Align(
          alignment: Alignment.topCenter,
          child: Callout(
            tint: CalloutTint.danger,
            icon: const Icon(FluentIcons.error_circle_20_regular),
            title: const Text('Could not read the displays'),
            message: Text(_errorMessage!),
            actions: [
              Button(
                variant: ButtonVariant.normal,
                tint: ButtonTint.neutral,
                onPressed: _loadDisplays,
                child: const Text('Retry'),
              ),
            ],
          ),
        ),
      );
    }

    if (_displays.isEmpty) {
      return Center(
        child: EmptyState(
          title: 'No displays found\nPlease check your system configuration',
          actions: [
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: _loadDisplays,
              child: const Text('Retry'),
            ),
          ],
        ),
      );
    }

    return Row(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Expanded(
                child: Padding(
                  padding: EdgeInsets.all(vars.spacing4),
                  child: Card(
                    variant: CardVariant.sunken,
                    size: WidgetSize.small,
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.stretch,
                      children: [
                        Expanded(
                          child: DisplayCanvas(
                            displays: _displays,
                            selectedDisplay: _selectedDisplay,
                            onDisplayTap: _selectDisplay,
                            currentWindow: _currentWindow,
                            cursorPosition: _cursorPosition,
                          ),
                        ),
                        SizedBox(height: vars.spacing2),
                        const _Legend(),
                      ],
                    ),
                  ),
                ),
              ),
              DisplayTable(
                displays: _displays,
                selectedDisplay: _selectedDisplay,
                onDisplayTap: _selectDisplay,
              ),
            ],
          ),
        ),
        const VerticalDivider(),
        if (_selectedDisplay != null)
          SizedBox(
            width: 312,
            child: DisplayDetails(display: _selectedDisplay!),
          ),
      ],
    );
  }

  /// The two live read-outs the canvas draws, as numbers.
  Widget _statusBar(ThemeVariables vars) {
    final cursor =
        '(${_cursorPosition.dx.toInt()}, ${_cursorPosition.dy.toInt()})';
    String window = 'none';
    final current = _currentWindow;
    if (current != null) {
      try {
        final b = current.bounds.toRect();
        window =
            '(${b.left.toInt()}, ${b.top.toInt()}) '
            '${b.width.toInt()} × ${b.height.toInt()}';
      } catch (_) {
        // Window might have been destroyed.
      }
    }
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        const Divider(),
        Container(
          color: vars.colorSurfaceSunken,
          padding: EdgeInsets.symmetric(
            horizontal: vars.spacing4,
            vertical: vars.spacing2,
          ),
          child: Row(
            spacing: vars.spacing2,
            children: [
              Icon(
                FluentIcons.cursor_20_regular,
                size: vars.iconMedium,
                color: vars.colorContentMuted,
              ),
              Text('Cursor $cursor', style: vars.mono),
              SizedBox(width: vars.spacing4),
              Icon(
                FluentIcons.window_20_regular,
                size: vars.iconMedium,
                color: vars.colorContentMuted,
              ),
              Expanded(
                child: Text(
                  'This window $window',
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: vars.mono,
                ),
              ),
              Text('updates every 100 ms', style: vars.muted),
            ],
          ),
        ),
      ],
    );
  }
}

/// The colours the arrangement is drawn in, from the theme.
class _CanvasColors {
  _CanvasColors(ThemeVariables vars)
    : // Ink over the card, so the reserved strips read in light and dark.
      bezel = Color.alphaBlend(
        vars.colorContent.withValues(alpha: 0.14),
        vars.colorSurfaceMuted,
      ),
      bezelEdge = vars.colorBorderStrong,
      work = vars.colorSurfaceRaised,
      selected = vars.colorPrimary[600]!,
      window = vars.colorWarning[600]!,
      windowInk = vars.colorWarning[700]!,
      cursor = vars.colorDanger[600]!,
      cursorRing = vars.colorSurfaceRaised;

  final Color bezel;
  final Color bezelEdge;
  final Color work;
  final Color selected;
  final Color window;
  final Color windowInk;
  final Color cursor;
  final Color cursorRing;
}

/// The subject of the example: the displays in desktop coordinates, each
/// with its work area, plus this window and the cursor. Drawn here rather
/// than with a kit component — it is a diagram, not a control.
class DisplayCanvas extends StatelessWidget {
  final List<na.Display> displays;
  final na.Display? selectedDisplay;
  final void Function(na.Display) onDisplayTap;
  final Window? currentWindow;
  final Offset cursorPosition;

  const DisplayCanvas({
    super.key,
    required this.displays,
    required this.selectedDisplay,
    required this.onDisplayTap,
    this.currentWindow,
    this.cursorPosition = Offset.zero,
  });

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    if (displays.isEmpty) {
      return Center(child: Text('No displays available', style: vars.muted));
    }

    return LayoutBuilder(
      builder: (context, constraints) =>
          _buildDisplayLayout(constraints, vars, _CanvasColors(vars)),
    );
  }

  Widget _buildDisplayLayout(
    BoxConstraints constraints,
    ThemeVariables vars,
    _CanvasColors colors,
  ) {
    // Calculate the bounding box of all displays
    final bounds = _calculateDisplayBounds();

    // Calculate scale to fit all displays in the canvas
    final scaleX = constraints.maxWidth / bounds.width;
    final scaleY = constraints.maxHeight / bounds.height;
    final scale = (scaleX < scaleY ? scaleX : scaleY) * 0.9;

    return Center(
      child: SizedBox(
        width: bounds.width * scale,
        height: bounds.height * scale,
        child: Stack(
          clipBehavior: Clip.none,
          children: [
            for (final display in displays)
              _buildDisplay(display, bounds, scale, vars, colors),
            if (currentWindow != null)
              _buildWindow(currentWindow!, bounds, scale, vars, colors),
            _buildCursor(bounds, scale, colors),
          ],
        ),
      ),
    );
  }

  Rect _calculateDisplayBounds() {
    double minX = double.infinity;
    double minY = double.infinity;
    double maxX = double.negativeInfinity;
    double maxY = double.negativeInfinity;

    for (final display in displays) {
      final position = display.position.toOffset();
      final size = display.size;
      minX = minX < position.dx ? minX : position.dx;
      minY = minY < position.dy ? minY : position.dy;
      maxX = maxX > (position.dx + size.width)
          ? maxX
          : (position.dx + size.width);
      maxY = maxY > (position.dy + size.height)
          ? maxY
          : (position.dy + size.height);
    }

    return Rect.fromLTWH(minX, minY, maxX - minX, maxY - minY);
  }

  Widget _buildDisplay(
    na.Display display,
    Rect bounds,
    double scale,
    ThemeVariables vars,
    _CanvasColors colors,
  ) {
    final workArea = display.workArea.toRect();
    final position = display.position.toOffset();
    final size = display.size;
    final isSelected = selectedDisplay?.id == display.id;

    // Calculate display position and size relative to the bounding box.
    // A hair of inset keeps neighbouring displays' edges apart.
    const gap = 1.5;
    final displayLeft = (position.dx - bounds.left) * scale + gap;
    final displayTop = (position.dy - bounds.top) * scale + gap;
    final displayWidth = size.width * scale - gap * 2;
    final displayHeight = size.height * scale - gap * 2;

    // Calculate work area position relative to the display
    final workAreaLeft = (workArea.left - position.dx) * scale;
    final workAreaTop = (workArea.top - position.dy) * scale;
    final workAreaWidth = workArea.width * scale - gap * 2;
    final workAreaHeight = workArea.height * scale - gap * 2;

    final radius = BorderRadius.circular(vars.radiusSmall);
    return Positioned(
      left: displayLeft,
      top: displayTop,
      width: displayWidth,
      height: displayHeight,
      child: MouseRegion(
        cursor: SystemMouseCursors.click,
        child: GestureDetector(
          onTap: () => onDisplayTap(display),
          child: AnimatedContainer(
            duration: vars.motionDuration,
            curve: vars.motionEasing,
            clipBehavior: Clip.antiAlias,
            decoration: BoxDecoration(
              // The bezel shows where the work area is not: menu bar,
              // dock, taskbar.
              color: colors.bezel,
              borderRadius: radius,
              border: Border.all(
                color: isSelected ? colors.selected : colors.bezelEdge,
                width: isSelected ? 2 : 1,
              ),
              boxShadow: isSelected ? vars.shadowSm : null,
            ),
            child: Stack(
              children: [
                Positioned(
                  left: workAreaLeft,
                  top: workAreaTop,
                  width: workAreaWidth.clamp(0.0, double.infinity),
                  height: workAreaHeight.clamp(0.0, double.infinity),
                  child: ColoredBox(
                    color: isSelected
                        ? Color.alphaBlend(
                            colors.selected.withValues(alpha: 0.10),
                            colors.work,
                          )
                        : colors.work,
                    child: _buildDisplayContent(
                      display,
                      workAreaHeight,
                      isSelected,
                      vars,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildDisplayContent(
    na.Display display,
    double height,
    bool isSelected,
    ThemeVariables vars,
  ) {
    final iconSize = (height * 0.2).clamp(14.0, 28.0);
    final ink = isSelected ? vars.colorPrimary[700]! : vars.colorContent;

    return Padding(
      padding: EdgeInsets.all(vars.spacing2),
      child: FittedBox(
        fit: BoxFit.scaleDown,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          spacing: vars.spacing05,
          children: [
            Icon(
              display.isPrimary
                  ? FluentIcons.desktop_mac_20_regular
                  : FluentIcons.desktop_20_regular,
              size: iconSize,
              color: isSelected ? ink : vars.colorContentMuted,
            ),
            Text(
              display.name ?? '',
              style: vars.labelLarge.copyWith(color: ink, height: 1.3),
              textAlign: TextAlign.center,
              overflow: TextOverflow.ellipsis,
              maxLines: 1,
            ),
            Text(
              '${display.size.width.toInt()}×${display.size.height.toInt()} '
              '@${_trim(display.scaleFactor)}x',
              style: vars.mono,
            ),
            if (display.isPrimary && height > 50)
              Padding(
                padding: EdgeInsets.only(top: vars.spacing05),
                child: Badge(
                  size: WidgetSize.small,
                  variant: BadgeVariant.tinted,
                  tint: BadgeTint.success,
                  child: const Text('Primary'),
                ),
              ),
          ],
        ),
      ),
    );
  }

  Widget _buildWindow(
    Window window,
    Rect bounds,
    double scale,
    ThemeVariables vars,
    _CanvasColors colors,
  ) {
    try {
      final windowBounds = window.bounds.toRect();
      final windowLeft = (windowBounds.left - bounds.left) * scale;
      final windowTop = (windowBounds.top - bounds.top) * scale;
      final windowWidth = windowBounds.width * scale;
      final windowHeight = windowBounds.height * scale;

      // Only draw if window is visible within bounds
      if (windowLeft + windowWidth < 0 ||
          windowTop + windowHeight < 0 ||
          windowLeft > bounds.width * scale ||
          windowTop > bounds.height * scale) {
        return const SizedBox.shrink();
      }

      final title = (window.title?.isNotEmpty ?? false)
          ? window.title!
          : 'Window';
      return Positioned(
        left: windowLeft,
        top: windowTop,
        width: windowWidth,
        height: windowHeight,
        child: IgnorePointer(
          child: Container(
            clipBehavior: Clip.antiAlias,
            decoration: BoxDecoration(
              color: colors.window.withValues(alpha: vars.washSurface * 2),
              border: Border.all(color: colors.window, width: 1.5),
              borderRadius: BorderRadius.circular(vars.radiusTiny),
            ),
            child: Align(
              alignment: Alignment.topLeft,
              child: Container(
                height: (20 * scale).clamp(10.0, 18.0),
                width: double.infinity,
                color: colors.window.withValues(alpha: vars.washEdge),
                padding: EdgeInsets.symmetric(horizontal: vars.spacing1),
                child: Row(
                  spacing: vars.spacing05,
                  children: [
                    Icon(
                      FluentIcons.window_20_regular,
                      size: vars.iconSmall,
                      color: colors.windowInk,
                    ),
                    Expanded(
                      child: Text(
                        title,
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                        style: vars.labelSmall.copyWith(
                          color: colors.windowInk,
                          fontSize: (10 * scale).clamp(8.0, 11.0),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
      );
    } catch (e) {
      // Window might have been destroyed, return empty widget
      return const SizedBox.shrink();
    }
  }

  Widget _buildCursor(Rect bounds, double scale, _CanvasColors colors) {
    final cursorLeft = (cursorPosition.dx - bounds.left) * scale;
    final cursorTop = (cursorPosition.dy - bounds.top) * scale;

    // Only draw if cursor is within bounds
    if (cursorLeft < 0 ||
        cursorTop < 0 ||
        cursorLeft > bounds.width * scale ||
        cursorTop > bounds.height * scale) {
      return const SizedBox.shrink();
    }

    return Positioned(
      left: cursorLeft - 6,
      top: cursorTop - 6,
      child: IgnorePointer(
        child: Container(
          width: 12,
          height: 12,
          decoration: BoxDecoration(
            color: colors.cursor,
            shape: BoxShape.circle,
            border: Border.all(color: colors.cursorRing, width: 2),
            boxShadow: [
              BoxShadow(
                color: colors.cursor.withValues(alpha: 0.4),
                blurRadius: 4,
                spreadRadius: 1,
              ),
            ],
          ),
        ),
      ),
    );
  }
}

/// What the marks on the canvas mean.
class _Legend extends StatelessWidget {
  const _Legend();

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final colors = _CanvasColors(vars);

    Widget swatch(Color fill, Color edge, String label, {bool round = false}) {
      return Row(
        mainAxisSize: MainAxisSize.min,
        spacing: vars.spacing1,
        children: [
          Container(
            width: 12,
            height: 12,
            decoration: BoxDecoration(
              color: fill,
              shape: round ? BoxShape.circle : BoxShape.rectangle,
              borderRadius: round
                  ? null
                  : BorderRadius.circular(vars.radiusTiny / 2),
              border: Border.all(color: edge),
            ),
          ),
          Text(label, style: vars.muted),
        ],
      );
    }

    return Wrap(
      spacing: vars.spacing4,
      runSpacing: vars.spacing1,
      alignment: WrapAlignment.center,
      children: [
        swatch(colors.work, colors.bezelEdge, 'Work area'),
        swatch(colors.bezel, colors.bezelEdge, 'Menu bar, dock, taskbar'),
        swatch(
          colors.window.withValues(alpha: vars.washEdge),
          colors.window,
          'This window',
        ),
        swatch(colors.cursor, colors.cursorRing, 'Cursor', round: true),
        Text('Click a display to inspect it', style: vars.muted),
      ],
    );
  }
}

/// The displays side by side, one row each.
class DisplayTable extends StatelessWidget {
  final List<na.Display> displays;
  final na.Display? selectedDisplay;
  final void Function(na.Display) onDisplayTap;

  const DisplayTable({
    super.key,
    required this.displays,
    required this.selectedDisplay,
    required this.onDisplayTap,
  });

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    TableCell head(String label, {int flex = 1}) =>
        TableCell(head: true, flex: flex, child: Text(label));
    TableCell mono(String value, {int flex = 1}) => TableCell(
      flex: flex,
      child: Text(
        value,
        maxLines: 1,
        overflow: TextOverflow.ellipsis,
        style: vars.mono.copyWith(color: vars.colorContentSecondary),
      ),
    );

    return Table(
      children: [
        TableHead(
          children: [
            head('Display', flex: 3),
            head('Resolution', flex: 2),
            head('Position', flex: 2),
            head('Scale'),
            head('Refresh'),
          ],
        ),
        for (final display in displays)
          TableRow(
            active: selectedDisplay?.id == display.id,
            onPressed: () => onDisplayTap(display),
            children: [
              TableCell(
                flex: 3,
                child: Row(
                  spacing: vars.spacing2,
                  children: [
                    Flexible(
                      child: Text(
                        display.name ?? '',
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      ),
                    ),
                    if (display.isPrimary)
                      Badge(
                        size: WidgetSize.small,
                        variant: BadgeVariant.tinted,
                        tint: BadgeTint.success,
                        child: const Text('Primary'),
                      ),
                  ],
                ),
              ),
              mono(
                '${display.size.width.toInt()} × ${display.size.height.toInt()}',
                flex: 2,
              ),
              mono(
                '(${display.position.x.toInt()}, ${display.position.y.toInt()})',
                flex: 2,
              ),
              mono('${_trim(display.scaleFactor)}×'),
              mono('${display.refreshRate} Hz'),
            ],
          ),
      ],
    );
  }
}

/// The selected display's read-outs.
class DisplayDetails extends StatelessWidget {
  final na.Display display;

  const DisplayDetails({super.key, required this.display});

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return SingleChildScrollView(
      padding: EdgeInsets.all(vars.spacing4),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        spacing: vars.spacing2,
        children: [
          _buildHeader(vars),
          SizedBox(height: vars.spacing1),
          ..._buildDetailSections(vars),
        ],
      ),
    );
  }

  Widget _buildHeader(ThemeVariables vars) {
    return Row(
      spacing: vars.spacing3,
      children: [
        Icon(
          display.isPrimary
              ? FluentIcons.desktop_mac_20_regular
              : FluentIcons.desktop_20_regular,
          size: 28,
          color: vars.colorContentSecondary,
        ),
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            spacing: vars.spacing1,
            children: [
              Text(
                display.name ?? '',
                maxLines: 2,
                overflow: TextOverflow.ellipsis,
                style: vars.titleMedium,
              ),
              Wrap(
                spacing: vars.spacing1,
                runSpacing: vars.spacing1,
                children: [
                  if (display.isPrimary)
                    Badge(
                      size: WidgetSize.small,
                      variant: BadgeVariant.tinted,
                      tint: BadgeTint.success,
                      child: const Text('Primary'),
                    ),
                  Badge(
                    size: WidgetSize.small,
                    variant: BadgeVariant.outlined,
                    tint: BadgeTint.neutral,
                    child: Text(
                      '${display.size.width.toInt()}×${display.size.height.toInt()}',
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ],
    );
  }

  List<Widget> _buildDetailSections(ThemeVariables vars) {
    return [
      _buildSection(vars, 'Basic Information', [
        _DetailItem(
          FluentIcons.number_symbol_20_regular,
          'ID',
          display.id.toString(),
        ),
        _DetailItem(FluentIcons.tag_20_regular, 'Name', display.name ?? ''),
        _DetailItem(
          FluentIcons.star_20_regular,
          'Primary',
          display.isPrimary ? 'Yes' : 'No',
        ),
      ]),

      _buildSection(vars, 'Hardware', [
        _DetailItem(
          FluentIcons.zoom_in_20_regular,
          'Scale Factor',
          '${display.scaleFactor}×',
        ),
        _DetailItem(
          FluentIcons.top_speed_20_regular,
          'Refresh Rate',
          '${display.refreshRate} Hz',
        ),
        _DetailItem(
          FluentIcons.color_20_regular,
          'Bit Depth',
          '${display.bitDepth} bit',
        ),
        _DetailItem(
          FluentIcons.arrow_rotate_clockwise_20_regular,
          'Orientation',
          _getOrientationName(),
        ),
      ]),

      _buildSection(vars, 'Geometry', [
        _DetailItem(
          FluentIcons.location_20_regular,
          'Position',
          _formatPosition(),
        ),
        _DetailItem(
          FluentIcons.full_screen_maximize_20_regular,
          'Full Size',
          _formatSize(display.size.toSize()),
        ),
        _DetailItem(
          FluentIcons.crop_20_regular,
          'Work Area Size',
          _formatSize(Size(display.workArea.width, display.workArea.height)),
        ),
        _DetailItem(
          FluentIcons.ruler_20_regular,
          'Work Area Position',
          _formatWorkAreaPosition(),
        ),
        _DetailItem(
          FluentIcons.border_outside_20_regular,
          'System Margins',
          _calculateSystemMargins(),
        ),
      ]),
    ];
  }

  Widget _buildSection(
    ThemeVariables vars,
    String title,
    List<_DetailItem> items,
  ) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      spacing: vars.spacing15,
      children: [
        Padding(
          padding: EdgeInsets.only(top: vars.spacing2),
          child: SectionLabel(title),
        ),
        Card(
          variant: CardVariant.raised,
          size: WidgetSize.small,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            spacing: vars.spacing2,
            children: [for (final item in items) _buildDetailRow(vars, item)],
          ),
        ),
      ],
    );
  }

  Widget _buildDetailRow(ThemeVariables vars, _DetailItem item) {
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      spacing: vars.spacing2,
      children: [
        Icon(item.icon, size: vars.iconMedium, color: vars.colorContentMuted),
        SizedBox(
          width: 116,
          child: Text(
            item.label,
            style: vars.bodySmall.copyWith(color: vars.colorContentSecondary),
          ),
        ),
        Expanded(
          child: Text(
            item.value,
            style: vars.bodySmall
                .inFace(vars.fontCode)
                .copyWith(color: vars.colorContent),
          ),
        ),
      ],
    );
  }

  String _getOrientationName() {
    return display.orientation.toString().split('.').last.toUpperCase();
  }

  String _formatPosition() {
    return '(${display.position.x.toInt()}, ${display.position.y.toInt()})';
  }

  String _formatSize(Size size) {
    return '${size.width.toInt()} × ${size.height.toInt()} px';
  }

  String _formatWorkAreaPosition() {
    final workArea = display.workArea.toRect();
    return '(${workArea.left.toInt()}, ${workArea.top.toInt()})';
  }

  /// How far the work area stays off each edge, relative to the display
  /// (the work area is in desktop coordinates, like the display's position).
  String _calculateSystemMargins() {
    final position = display.position.toOffset();
    final size = display.size;
    final workArea = display.workArea.toRect();

    final topMargin = workArea.top - position.dy;
    final bottomMargin = position.dy + size.height - workArea.bottom;
    final leftMargin = workArea.left - position.dx;
    final rightMargin = position.dx + size.width - workArea.right;

    List<String> margins = [];
    if (topMargin > 0) margins.add('T:${topMargin.toInt()}');
    if (bottomMargin > 0) margins.add('B:${bottomMargin.toInt()}');
    if (leftMargin > 0) margins.add('L:${leftMargin.toInt()}');
    if (rightMargin > 0) margins.add('R:${rightMargin.toInt()}');

    return margins.isEmpty ? 'None' : margins.join(' ');
  }
}

class _DetailItem {
  final IconData icon;
  final String label;
  final String value;

  const _DetailItem(this.icon, this.label, this.value);
}

/// 2.0 → "2", 1.25 → "1.25".
String _trim(double value) => value == value.roundToDouble()
    ? value.toInt().toString()
    : value.toStringAsFixed(2).replaceFirst(RegExp(r'0+$'), '');
