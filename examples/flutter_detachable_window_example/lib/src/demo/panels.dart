import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/scheduler.dart';

import '../detachable/detachable.dart';

/// Chrome shared by the demo panels: a header to grab, and proof that the
/// panel's `State` is the same object wherever it is shown.
class PanelFrame extends StatelessWidget {
  const PanelFrame({
    super.key,
    required this.itemId,
    required this.title,
    required this.icon,
    required this.stateLabel,
    required this.child,
  });

  final String itemId;
  final String title;
  final IconData icon;
  final String stateLabel;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final controller = DetachScope.of(context);
    final floating = controller.isFloating(itemId);
    final vars = context.vars;

    return ColoredBox(
      color: vars.colorSurface,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // The header is the grip the example is about, so it is drawn
          // here; a floating panel's header takes the info wash.
          DetachHandle(
            itemId: itemId,
            child: Container(
              height: 44,
              padding: EdgeInsets.only(
                left: vars.spacing25,
                right: vars.spacing15,
              ),
              decoration: BoxDecoration(
                color: floating
                    ? vars.colorInfo[500]!.withValues(alpha: vars.washEdge)
                    : vars.colorSurfaceChrome,
                border: Border(
                  bottom: BorderSide(
                    color: vars.colorBorder,
                    width: vars.strokeHairline,
                  ),
                ),
              ),
              child: Row(
                children: [
                  Icon(
                    FluentIcons.re_order_dots_vertical_20_regular,
                    size: vars.iconMedium,
                    color: vars.colorContentFaint,
                  ),
                  SizedBox(width: vars.spacing1),
                  Icon(icon, size: vars.iconMedium, color: vars.colorContent),
                  SizedBox(width: vars.spacing2),
                  Expanded(
                    child: Text(
                      title,
                      overflow: TextOverflow.ellipsis,
                      style: vars.titleSmall,
                    ),
                  ),
                  if (floating)
                    Tooltip(
                      label: 'Dock back',
                      child: IconButton(
                        semanticsLabel: 'Dock back',
                        size: WidgetSize.small,
                        icon: const Icon(FluentIcons.arrow_enter_20_regular),
                        onPressed: () => controller.dockAnywhere(itemId),
                      ),
                    )
                  else
                    Tooltip(
                      label: 'Open in a window',
                      child: IconButton(
                        semanticsLabel: 'Open in a window',
                        size: WidgetSize.small,
                        icon: const Icon(FluentIcons.open_20_regular),
                        onPressed: () => controller.float(itemId),
                      ),
                    ),
                ],
              ),
            ),
          ),
          Padding(
            padding: EdgeInsets.fromLTRB(
              vars.spacing25,
              vars.spacing15,
              vars.spacing25,
              0,
            ),
            // Wraps rather than truncates: the move count at the end is the
            // point of the line.
            child: Text(stateLabel, style: vars.mono),
          ),
          Expanded(child: child),
        ],
      ),
    );
  }
}

/// Tracks which window a `State` is currently shown in, to count the moves.
mixin WindowMoveCounter<T extends StatefulWidget> on State<T> {
  static int _nextInstance = 1;

  final int instance = _nextInstance++;
  final DateTime createdAt = DateTime.now();
  int windowMoves = 0;
  int? _viewId;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    final viewId = View.of(context).viewId;
    if (_viewId != null && _viewId != viewId) windowMoves++;
    _viewId = viewId;
  }

  String get stateLabel {
    String two(int v) => v.toString().padLeft(2, '0');
    final t = createdAt;
    return 'State #$instance · created ${two(t.hour)}:${two(t.minute)}:${two(t.second)}'
        ' · moved between windows $windowMoves×';
  }
}

class InspectorPanel extends StatefulWidget {
  const InspectorPanel({super.key, required this.itemId});

  final String itemId;

  @override
  State<InspectorPanel> createState() => _InspectorPanelState();
}

class _InspectorPanelState extends State<InspectorPanel>
    with WindowMoveCounter {
  final _name = TextEditingController(text: 'Untitled layer');
  final _scroll = ScrollController();
  int _counter = 0;
  double _opacity = 0.8;
  int _selected = 3;
  bool _visible = true;

  @override
  void dispose() {
    _name.dispose();
    _scroll.dispose();
    super.dispose();
  }

  // Keeps the layer list (and its scroll position) the same element when the
  // layout switches between the narrow and the wide arrangement.
  final _layersKey = GlobalKey(debugLabel: 'layers');

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final controls = <Widget>[
      FormField(
        label: 'Layer name',
        child: TextField(controller: _name, size: WidgetSize.small),
      ),
      SizedBox(height: vars.spacing3),
      Row(
        children: [
          Text('Clicks: $_counter', style: vars.titleSmall),
          const Spacer(),
          Button(
            variant: ButtonVariant.tinted,
            onPressed: () => setState(() => _counter++),
            child: const Text('+1'),
          ),
        ],
      ),
      PreferenceRow(
        title: 'Visible',
        trailing: Switch(
          size: WidgetSize.small,
          value: _visible,
          onChanged: (v) => setState(() => _visible = v),
        ),
      ),
      SizedBox(height: vars.spacing1),
      Text('Opacity ${(_opacity * 100).round()}%', style: vars.labelSmall),
      Slider(
        size: WidgetSize.small,
        values: [_opacity * 100],
        semanticsLabel: 'Opacity',
        onChanged: (values) => setState(() => _opacity = values.first / 100),
      ),
    ];
    final layers = Card(
      key: _layersKey,
      variant: CardVariant.sunken,
      size: WidgetSize.small,
      child: ListView.builder(
        controller: _scroll,
        itemCount: 40,
        itemBuilder: (context, i) => NavItem(
          size: WidgetSize.small,
          label: 'Layer ${i + 1}',
          icon: FluentIcons.layer_20_regular,
          current: i == _selected,
          onPressed: () => setState(() => _selected = i),
        ),
      ),
    );

    return PanelFrame(
      itemId: widget.itemId,
      title: 'Inspector',
      icon: FluentIcons.options_20_regular,
      stateLabel: stateLabel,
      child: LayoutBuilder(
        builder: (context, constraints) {
          if (constraints.maxWidth >= 480) {
            // Wide and short, e.g. a bottom or top strip: side by side.
            return Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                SizedBox(
                  width: 280,
                  child: ListView(
                    padding: EdgeInsets.all(vars.spacing25),
                    children: controls,
                  ),
                ),
                Expanded(
                  child: Padding(
                    padding: EdgeInsets.fromLTRB(
                      0,
                      vars.spacing25,
                      vars.spacing25,
                      vars.spacing25,
                    ),
                    child: layers,
                  ),
                ),
              ],
            );
          }
          return ListView(
            padding: EdgeInsets.all(vars.spacing25),
            children: [
              ...controls,
              SizedBox(height: vars.spacing2),
              const SectionLabel('Layers (scroll position is kept too)'),
              SizedBox(height: vars.spacing1),
              SizedBox(height: 220, child: layers),
            ],
          );
        },
      ),
    );
  }
}

class StopwatchPanel extends StatefulWidget {
  const StopwatchPanel({super.key, required this.itemId});

  final String itemId;

  @override
  State<StopwatchPanel> createState() => _StopwatchPanelState();
}

class _StopwatchPanelState extends State<StopwatchPanel>
    with SingleTickerProviderStateMixin, WindowMoveCounter {
  late final Ticker _ticker = createTicker((_) => setState(() {}));
  final _stopwatch = Stopwatch();
  final _laps = <Duration>[];

  @override
  void initState() {
    super.initState();
    _stopwatch.start();
    _ticker.start();
  }

  @override
  void dispose() {
    _ticker.dispose();
    super.dispose();
  }

  void _toggle() {
    setState(() {
      if (_stopwatch.isRunning) {
        _stopwatch.stop();
        _ticker.stop();
      } else {
        _stopwatch.start();
        _ticker.start();
      }
    });
  }

  static String _format(Duration d) {
    String two(int v) => v.toString().padLeft(2, '0');
    final centis = (d.inMilliseconds % 1000) ~/ 10;
    return '${two(d.inMinutes)}:${two(d.inSeconds % 60)}.${two(centis)}';
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final clock = FittedBox(
      fit: BoxFit.scaleDown,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            _format(_stopwatch.elapsed),
            style: vars.headlineLarge.copyWith(
              fontFeatures: const [FontFeature.tabularFigures()],
            ),
          ),
          SizedBox(height: vars.spacing05),
          Text('Keeps running while the panel moves', style: vars.muted),
          SizedBox(height: vars.spacing3),
          Row(
            mainAxisSize: MainAxisSize.min,
            spacing: vars.spacing2,
            children: [
              Button(
                variant: ButtonVariant.filled,
                onPressed: _toggle,
                child: Text(_stopwatch.isRunning ? 'Pause' : 'Start'),
              ),
              Button(
                variant: ButtonVariant.normal,
                tint: ButtonTint.neutral,
                onPressed: () =>
                    setState(() => _laps.insert(0, _stopwatch.elapsed)),
                child: const Text('Lap'),
              ),
              Button(
                variant: ButtonVariant.plain,
                tint: ButtonTint.neutral,
                onPressed: () => setState(() {
                  _stopwatch.reset();
                  _laps.clear();
                }),
                child: const Text('Reset'),
              ),
            ],
          ),
        ],
      ),
    );
    final laps = _laps.isEmpty
        ? Center(child: Text('No laps yet', style: vars.muted))
        : Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              const TableHead(
                children: [
                  TableCell(head: true, width: 56, child: Text('#')),
                  TableCell(head: true, child: Text('Time')),
                ],
              ),
              Expanded(
                child: ListView.builder(
                  itemCount: _laps.length,
                  itemBuilder: (context, i) => TableRow(
                    children: [
                      TableCell(
                        width: 56,
                        child: Text('#${_laps.length - i}', style: vars.mono),
                      ),
                      TableCell(
                        child: Text(
                          _format(_laps[i]),
                          style: vars.bodySmall.copyWith(
                            fontFeatures: const [FontFeature.tabularFigures()],
                          ),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ],
          );

    return PanelFrame(
      itemId: widget.itemId,
      title: 'Stopwatch',
      icon: FluentIcons.timer_20_regular,
      stateLabel: stateLabel,
      child: LayoutBuilder(
        builder: (context, constraints) {
          if (constraints.maxWidth > constraints.maxHeight * 1.6) {
            // Wide and short: clock on the left, laps on the right.
            return Row(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Expanded(
                  flex: 3,
                  child: Padding(
                    padding: EdgeInsets.all(vars.spacing25),
                    child: Center(child: clock),
                  ),
                ),
                const VerticalDivider(),
                Expanded(flex: 2, child: laps),
              ],
            );
          }
          return Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Padding(
                padding: EdgeInsets.fromLTRB(
                  vars.spacing25,
                  vars.spacing4,
                  vars.spacing25,
                  vars.spacing3,
                ),
                child: clock,
              ),
              const Divider(),
              Expanded(child: laps),
            ],
          );
        },
      ),
    );
  }
}
