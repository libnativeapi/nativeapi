import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi/nativeapi.dart' as na;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    hide Button, Menu, MenuItem, Preferences;

/// A to-scale map of every display (its work area inside) and every window
/// (its content bounds inside), zoomable. Tap a window to select it.
///
/// This is what the example demonstrates, so it is drawn here rather than
/// built from kit components; its colours all come from the theme.
class WindowCanvas extends StatefulWidget {
  const WindowCanvas({
    super.key,
    required this.windows,
    required this.displays,
    required this.selectedWindow,
    required this.onWindowTap,
  });

  final List<Window> windows;
  final List<na.Display> displays;
  final Window? selectedWindow;
  final ValueChanged<Window> onWindowTap;

  @override
  State<WindowCanvas> createState() => _WindowCanvasState();
}

class _WindowCanvasState extends State<WindowCanvas> {
  final TransformationController _transformationController =
      TransformationController();
  double _baseScale = 1.0;

  @override
  void initState() {
    super.initState();
    _transformationController.addListener(_onTransformationChanged);
  }

  @override
  void dispose() {
    _transformationController.removeListener(_onTransformationChanged);
    _transformationController.dispose();
    super.dispose();
  }

  void _onTransformationChanged() {
    final s = _transformationController.value.getMaxScaleOnAxis();
    if (s != _baseScale) setState(() => _baseScale = s);
  }

  void _zoomIn() {
    final s = (_transformationController.value.getMaxScaleOnAxis() * 1.3).clamp(
      0.5,
      5.0,
    );
    _transformationController.value = Matrix4.diagonal3Values(s, s, 1);
  }

  void _zoomOut() {
    final s = (_transformationController.value.getMaxScaleOnAxis() / 1.3).clamp(
      0.5,
      5.0,
    );
    _transformationController.value = Matrix4.diagonal3Values(s, s, 1);
  }

  void _resetZoom() {
    _transformationController.value = Matrix4.identity();
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    if (widget.windows.isEmpty && widget.displays.isEmpty) {
      return const Center(
        child: EmptyState(title: 'No windows or displays available'),
      );
    }

    Widget zoomButton(String label, IconData icon, VoidCallback onPressed) =>
        Tooltip(
          label: label,
          child: IconButton(
            icon: Icon(icon),
            semanticsLabel: label,
            onPressed: onPressed,
          ),
        );

    return ClipRRect(
      borderRadius: BorderRadius.circular(vars.radiusLarge),
      child: DecoratedBox(
        decoration: BoxDecoration(
          color: vars.colorSurfaceSunken,
          border: Border.all(
            color: vars.colorBorder,
            width: context.hairlineWidth,
          ),
          borderRadius: BorderRadius.circular(vars.radiusLarge),
        ),
        child: Stack(
          children: [
            Padding(
              padding: EdgeInsets.all(vars.spacing4),
              child: LayoutBuilder(
                builder: (context, constraints) => InteractiveViewer(
                  transformationController: _transformationController,
                  minScale: 0.5,
                  maxScale: 5.0,
                  boundaryMargin: const EdgeInsets.all(20),
                  child: _buildWindowLayout(vars, constraints),
                ),
              ),
            ),
            // Zoom controls
            Positioned(
              top: vars.spacing2,
              right: vars.spacing2,
              child: Container(
                padding: EdgeInsets.all(vars.spacing05),
                decoration: BoxDecoration(
                  color: vars.colorSurfaceRaised,
                  borderRadius: BorderRadius.circular(vars.radiusMedium),
                  border: Border.all(
                    color: vars.colorBorder,
                    width: context.hairlineWidth,
                  ),
                  boxShadow: vars.shadowXs,
                ),
                child: Row(
                  mainAxisSize: MainAxisSize.min,
                  spacing: vars.spacing05,
                  children: [
                    zoomButton(
                      'Zoom Out',
                      FluentIcons.zoom_out_20_regular,
                      _zoomOut,
                    ),
                    zoomButton(
                      'Zoom In',
                      FluentIcons.zoom_in_20_regular,
                      _zoomIn,
                    ),
                    zoomButton(
                      'Fit to Screen',
                      FluentIcons.full_screen_maximize_20_regular,
                      _resetZoom,
                    ),
                  ],
                ),
              ),
            ),
            // Scale indicator
            Positioned(
              bottom: vars.spacing2,
              left: vars.spacing2,
              child: Badge(
                size: WidgetSize.small,
                variant: BadgeVariant.raised,
                tint: BadgeTint.neutral,
                child: Text('${(_baseScale * 100).toStringAsFixed(0)}%'),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildWindowLayout(ThemeVariables vars, BoxConstraints constraints) {
    final bounds = _calculateBounds();
    if (bounds.isEmpty) {
      return const Center(
        child: EmptyState(title: 'No displays or windows available'),
      );
    }

    final scaleX = constraints.maxWidth / bounds.width;
    final scaleY = constraints.maxHeight / bounds.height;
    final scale = (scaleX < scaleY ? scaleX : scaleY) * 0.9;

    return SizedBox(
      width: bounds.width * scale,
      height: bounds.height * scale,
      child: Stack(
        children: [
          ...widget.displays.map((d) => _buildDisplay(vars, d, bounds, scale)),
          ...widget.windows.map((w) => _buildWindow(vars, w, bounds, scale)),
        ],
      ),
    );
  }

  Rect _calculateBounds() {
    double minX = double.infinity;
    double minY = double.infinity;
    double maxX = double.negativeInfinity;
    double maxY = double.negativeInfinity;

    for (final display in widget.displays) {
      final pos = display.position.toOffset();
      final size = display.size.toSize();
      minX = minX < pos.dx ? minX : pos.dx;
      minY = minY < pos.dy ? minY : pos.dy;
      maxX = maxX > pos.dx + size.width ? maxX : pos.dx + size.width;
      maxY = maxY > pos.dy + size.height ? maxY : pos.dy + size.height;
    }

    for (final window in widget.windows) {
      try {
        final b = window.bounds.toRect();
        minX = minX < b.left ? minX : b.left;
        minY = minY < b.top ? minY : b.top;
        maxX = maxX > b.right ? maxX : b.right;
        maxY = maxY > b.bottom ? maxY : b.bottom;
      } catch (_) {
        continue;
      }
    }

    if (minX == double.infinity) return Rect.zero;
    const pad = 50.0;
    return Rect.fromLTWH(
      minX - pad,
      minY - pad,
      maxX - minX + pad * 2,
      maxY - minY + pad * 2,
    );
  }

  Widget _buildDisplay(
    ThemeVariables vars,
    na.Display display,
    Rect bounds,
    double scale,
  ) {
    final pos = display.position.toOffset();
    final size = display.size.toSize();
    final work = display.workArea.toRect();

    return Positioned(
      left: (pos.dx - bounds.left) * scale,
      top: (pos.dy - bounds.top) * scale,
      child: SizedBox(
        width: size.width * scale,
        height: size.height * scale,
        child: Stack(
          children: [
            // Display bezel
            Container(
              decoration: BoxDecoration(
                color: vars.colorNeutral[700],
                border: Border.all(color: vars.colorNeutral[500]!, width: 1.5),
                borderRadius: BorderRadius.circular(vars.radiusTiny),
              ),
            ),
            // Work area
            Positioned(
              left: (work.left - pos.dx) * scale,
              top: (work.top - pos.dy) * scale,
              child: Container(
                width: work.width * scale,
                height: work.height * scale,
                decoration: BoxDecoration(
                  color: vars.colorSurfaceMuted,
                  border: Border.all(color: vars.colorBorder, width: 0.5),
                ),
              ),
            ),
            // Display label
            if ((display.name ?? '').isNotEmpty)
              Positioned(
                top: vars.spacing1,
                left: vars.spacing1,
                child: Badge(
                  size: WidgetSize.small,
                  variant: BadgeVariant.filled,
                  tint: BadgeTint.neutral,
                  child: Text(display.name!),
                ),
              ),
          ],
        ),
      ),
    );
  }

  Widget _buildWindow(
    ThemeVariables vars,
    Window window,
    Rect bounds,
    double scale,
  ) {
    try {
      final wb = window.bounds.toRect();
      final cb = window.contentBounds.toRect();

      final left = (wb.left - bounds.left) * scale;
      final top = (wb.top - bounds.top) * scale;
      final w = wb.width * scale;
      final h = wb.height * scale;

      final cLeft = (cb.left - wb.left) * scale;
      final cTop = (cb.top - wb.top) * scale;
      final cW = cb.width * scale;
      final cH = cb.height * scale;

      final selected = widget.selectedWindow?.id == window.id;
      final accent = selected
          ? vars.colorPrimary[600]!
          : vars.colorWarning[600]!;
      final contentInk = vars.colorSuccess[600]!;
      final titleBarHeight = (28 * scale).clamp(12.0, 28.0);

      if (left + w < 0 ||
          top + h < 0 ||
          left > bounds.width * scale ||
          top > bounds.height * scale) {
        return const SizedBox.shrink();
      }

      return Positioned(
        left: left,
        top: top,
        child: GestureDetector(
          onTap: () => widget.onWindowTap(window),
          child: AnimatedContainer(
            duration: vars.motionDuration,
            width: w,
            height: h,
            decoration: BoxDecoration(
              border: Border.all(color: accent, width: selected ? 3 : 1.5),
              boxShadow: selected ? vars.shadowMd : vars.shadowXs,
            ),
            child: Stack(
              children: [
                // Window fill
                Container(color: vars.colorSurface),
                Container(color: accent.withValues(alpha: 0.08)),
                // Title bar
                Positioned(
                  top: 0,
                  left: 0,
                  right: 0,
                  child: Container(
                    height: titleBarHeight,
                    decoration: BoxDecoration(
                      color: accent.withValues(alpha: 0.25),
                      border: Border(
                        bottom: BorderSide(color: accent, width: 1),
                      ),
                    ),
                    padding: EdgeInsets.symmetric(
                      horizontal: (6 * scale).clamp(3.0, 6.0),
                    ),
                    child: Row(
                      children: [
                        Icon(
                          FluentIcons.window_20_regular,
                          size: (10 * scale).clamp(7.0, 10.0),
                          color: accent,
                        ),
                        SizedBox(width: (4 * scale).clamp(2.0, 4.0)),
                        Expanded(
                          child: Text(
                            _windowLabel(window),
                            maxLines: 1,
                            overflow: TextOverflow.ellipsis,
                            style: vars.labelSmall.copyWith(
                              fontSize: (9 * scale).clamp(6.0, 9.0),
                              color: accent,
                            ),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
                // Content bounds
                Positioned(
                  left: cLeft,
                  top: cTop,
                  child: Container(
                    width: cW,
                    height: cH,
                    decoration: BoxDecoration(
                      color: contentInk.withValues(alpha: 0.04),
                      border: Border.all(
                        color: contentInk.withValues(alpha: 0.6),
                        width: 1.5,
                      ),
                    ),
                    child: Center(
                      child: Text(
                        'Content',
                        style: vars.labelSmall.copyWith(
                          fontSize: (7 * scale).clamp(5.0, 9.0),
                          color: contentInk,
                        ),
                      ),
                    ),
                  ),
                ),
                // Size label
                Positioned(
                  left: 4,
                  top: titleBarHeight + 4,
                  child: _buildLabel(
                    vars,
                    '${wb.width.toInt()}×${wb.height.toInt()}',
                    '(${wb.left.toInt()}, ${wb.top.toInt()})',
                    accent,
                    scale,
                  ),
                ),
                // Content size label, in the content's bottom-left corner
                // so it never covers the window's own label.
                Positioned(
                  left: cLeft + 4,
                  bottom: h - cTop - cH + 4,
                  child: _buildLabel(
                    vars,
                    '${cb.width.toInt()}×${cb.height.toInt()}',
                    '',
                    contentInk,
                    scale,
                  ),
                ),
              ],
            ),
          ),
        ),
      );
    } catch (_) {
      return const SizedBox.shrink();
    }
  }

  String _windowLabel(Window w) {
    final title = w.title;
    if (title != null && title.isNotEmpty) return title;
    return 'Window #${w.id}';
  }

  Widget _buildLabel(
    ThemeVariables vars,
    String line1,
    String line2,
    Color color,
    double scale,
  ) {
    final fontSize = (7 * scale).clamp(5.0, 10.0);
    return Container(
      padding: EdgeInsets.all((3 * scale).clamp(1.5, 4.0)),
      decoration: BoxDecoration(
        color: vars.colorSurface.withValues(alpha: 0.85),
        border: Border.all(color: color.withValues(alpha: 0.6), width: 1),
        borderRadius: BorderRadius.circular(vars.radiusTiny),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            line1,
            style: vars.labelSmall.copyWith(fontSize: fontSize, color: color),
          ),
          if (line2.isNotEmpty)
            Text(
              line2,
              style: vars.captionSmall.copyWith(
                fontSize: fontSize * 0.85,
                color: color.withValues(alpha: 0.7),
              ),
            ),
        ],
      ),
    );
  }
}
