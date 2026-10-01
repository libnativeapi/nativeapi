import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart' hide Button;

/// Thickness of the resize handles and their distance from the window edge.
///
/// The handles are inset so that they are clearly the widget's, not the native
/// window frame's. tools/gui/flutter_window_drag_areas_test.* in the workspace
/// repository presses in the middle of these bands; keep the numbers in sync.
const double kResizeEdgeSize = 12;
const double kResizeEdgeInset = 16;

/// Height of the move bar. The Windows test activates the window with a click
/// at y = 110, which has to land on bare panel below the bar.
const double _barHeight = 44;

void main() {
  runApp(const DragAreasApp());
}

class DragAreasApp extends StatelessWidget {
  const DragAreasApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'nativeapi · Drag areas', home: DragAreasPage());
}

class DragAreasPage extends StatefulWidget {
  const DragAreasPage({super.key});

  @override
  State<DragAreasPage> createState() => _DragAreasPageState();
}

class _DragAreasPageState extends State<DragAreasPage> {
  static const _limitedEdges = [
    ResizeEdge.right,
    ResizeEdge.bottom,
    ResizeEdge.bottomRight,
  ];

  int _clicks = 0;
  bool _limited = false;

  @override
  void initState() {
    super.initState();
    // Custom chrome: no native title bar or buttons, so moving and resizing
    // is left to the two drag areas.
    final window = WindowManager.instance.getCurrent();
    if (window == null) return;
    window.title = 'nativeapi · Drag areas';
    window.titleBarStyle = TitleBarStyle.hidden;
    window.minimumSize = const Size(480, 320).toNative();
    window.contentSize = const Size(720, 480).toNative();
    window.center();
  }

  @override
  Widget build(BuildContext context) {
    // The window's size as laid out rather than MediaQuery's: the resize in
    // initState lands while the first frame is being built, and MediaQuery
    // would report the runner's default size until something else rebuilt.
    return LayoutBuilder(
      builder: (context, constraints) => _page(context, constraints.biggest),
    );
  }

  Widget _page(BuildContext context, Size size) {
    final vars = context.vars;
    final accent = vars.colorPrimary[600]!;
    return DragToResizeArea(
      resizeEdgeSize: kResizeEdgeSize,
      resizeEdgeMargin: const EdgeInsets.all(kResizeEdgeInset),
      // The handles are the subject: the accent's edge wash, so they show on
      // the canvas without shouting.
      resizeEdgeColor: accent.withValues(alpha: vars.washEdge),
      enableResizeEdges: _limited ? _limitedEdges : null,
      child: Padding(
        padding: const EdgeInsets.all(kResizeEdgeInset + kResizeEdgeSize),
        child: Container(
          clipBehavior: Clip.antiAlias,
          decoration: BoxDecoration(
            color: vars.colorSurface,
            border: Border.all(
              color: vars.colorBorder,
              width: context.hairlineWidth,
            ),
            borderRadius: BorderRadius.circular(vars.radiusLarge),
            boxShadow: vars.shadowSm,
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              _moveBar(vars),
              const Divider(),
              Expanded(child: _body(vars, size)),
            ],
          ),
        ),
      ),
    );
  }

  /// The custom title bar: a DragToMoveArea.
  Widget _moveBar(ThemeVariables vars) {
    final accent = vars.colorPrimary[600]!;
    return DragToMoveArea(
      child: Container(
        height: _barHeight,
        color: accent.withValues(alpha: vars.washSurface),
        padding: EdgeInsets.symmetric(horizontal: vars.spacing3),
        child: Stack(
          alignment: Alignment.center,
          children: [
            Align(
              alignment: Alignment.centerLeft,
              child: Icon(
                FluentIcons.arrow_move_20_regular,
                size: vars.iconMedium,
                color: accent,
              ),
            ),
            Text(
              'Drag here to move',
              style: vars.labelStrong.copyWith(color: vars.colorContent),
            ),
          ],
        ),
      ),
    );
  }

  Widget _body(ThemeVariables vars, Size size) {
    return Center(
      child: SingleChildScrollView(
        padding: EdgeInsets.all(vars.spacing4),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Text(
              'Size: ${size.width.round()} x ${size.height.round()}',
              style: vars.titleLarge,
            ),
            SizedBox(height: vars.spacing1),
            Text(
              'Double-click the bar to maximize, drag the tinted frame to '
              'resize.',
              textAlign: TextAlign.center,
              style: vars.bodyMedium.copyWith(color: vars.colorContentSubtle),
            ),
            SizedBox(height: vars.spacing5),
            IntrinsicHeight(
              child: Row(
                mainAxisSize: MainAxisSize.min,
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  _card(
                    vars,
                    label: 'Pass-through',
                    note: 'The middle of the resize area lets clicks through.',
                    value: 'Clicks: $_clicks',
                    action: Button(
                      variant: ButtonVariant.filled,
                      onPressed: () => setState(() => _clicks++),
                      child: const Text('+1'),
                    ),
                  ),
                  SizedBox(width: vars.spacing3),
                  _card(
                    vars,
                    label: 'Resize handles',
                    note: 'enableResizeEdges: every handle, or only three.',
                    value: _limited ? '3 of 8 handles' : '8 of 8 handles',
                    action: Button(
                      variant: ButtonVariant.normal,
                      tint: ButtonTint.neutral,
                      onPressed: () => setState(() => _limited = !_limited),
                      child: Text(
                        _limited ? 'Edges: right and bottom' : 'Edges: all',
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _card(
    ThemeVariables vars, {
    required String label,
    required String note,
    required String value,
    required Widget action,
  }) {
    return SizedBox(
      width: 184,
      child: Card(
        variant: CardVariant.sunken,
        size: WidgetSize.small,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            SectionLabel(label),
            SizedBox(height: vars.spacing1),
            Text(value, style: vars.titleMedium),
            SizedBox(height: vars.spacing05),
            Text(note, style: vars.muted),
            SizedBox(height: vars.spacing3),
            const Spacer(),
            action,
          ],
        ),
      ),
    );
  }
}
