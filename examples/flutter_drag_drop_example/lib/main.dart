import 'dart:io';

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

/// Drag and drop with the two widgets of `nativeapi`:
///
/// - the left pane is a [DropRegion]: drop files or text on it;
/// - the cards on the right are [DragOutArea]s: drag them into a file manager,
///   an editor, or onto the left pane.
///
/// tools/gui/flutter_drag_drop_test.* in the workspace repository drives this
/// example by its texts; keep them in sync.
void main() {
  runApp(const DragDropApp());
}

class DragDropApp extends StatelessWidget {
  const DragDropApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'nativeapi · Drag and drop', home: DragDropPage());
}

class DragDropPage extends StatefulWidget {
  const DragDropPage({super.key});

  @override
  State<DragDropPage> createState() => _DragDropPageState();
}

class _DragDropPageState extends State<DragDropPage> {
  late final File _file = _createFile();
  bool _hovering = false;
  Offset? _hoverPosition;
  int _drops = 0;
  List<String> _droppedFiles = const [];
  String? _droppedText;
  String _lastDragResult = '-';

  static File _createFile() {
    final file = File(
      '${Directory.systemTemp.path}${Platform.pathSeparator}'
      'nativeapi-drag-drop-note.txt',
    );
    file.writeAsStringSync('A note dragged out of drag_drop_example.\n');
    return file;
  }

  @override
  void initState() {
    super.initState();
    final window = WindowManager.instance.getCurrent();
    if (window == null) return;
    window.title = 'nativeapi · Drag and drop';
    window.contentSize = const Size(760, 460).toNative();
    window.center();
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Row(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Expanded(
          child: Padding(
            padding: EdgeInsets.all(vars.spacing4),
            child: _buildDropPanel(vars),
          ),
        ),
        const VerticalDivider(),
        SizedBox(
          width: 260,
          child: ColoredBox(
            color: vars.colorSurfaceSunken,
            child: Padding(
              padding: EdgeInsets.all(vars.spacing4),
              child: _buildDragPanel(vars),
            ),
          ),
        ),
      ],
    );
  }

  /// The drop target: drawn here, since it is what the example is about.
  Widget _buildDropPanel(ThemeVariables vars) {
    final accent = vars.colorPrimary[600]!;
    final position = _hoverPosition;
    return DropRegion(
      onDragEntered: (position) => setState(() {
        _hovering = true;
        _hoverPosition = position;
      }),
      onDragUpdated: (position) => setState(() => _hoverPosition = position),
      onDragExited: () => setState(() {
        _hovering = false;
        _hoverPosition = null;
      }),
      onDropped: (details) => setState(() {
        _hovering = false;
        _hoverPosition = null;
        _drops++;
        _droppedFiles = details.filePaths;
        _droppedText = details.text;
      }),
      child: AnimatedContainer(
        duration: vars.motionDuration,
        curve: vars.motionEasing,
        decoration: BoxDecoration(
          color: _hovering
              ? accent.withValues(alpha: vars.washSurface)
              : vars.colorSurface,
          borderRadius: BorderRadius.circular(vars.radiusLarge),
          border: Border.all(
            color: _hovering ? accent : vars.colorBorderStrong,
            width: _hovering ? 2 : context.hairlineWidth,
          ),
        ),
        padding: EdgeInsets.all(vars.spacing4),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(
                  FluentIcons.arrow_download_20_regular,
                  size: vars.iconMedium,
                  color: _hovering ? accent : vars.colorContentSubtle,
                ),
                SizedBox(width: vars.spacing2),
                Expanded(
                  child: Text(
                    _hovering ? 'Release to drop' : 'Drop files or text here',
                    style: vars.titleMedium,
                  ),
                ),
                Badge(
                  size: WidgetSize.small,
                  variant: _drops > 0
                      ? BadgeVariant.tinted
                      : BadgeVariant.outlined,
                  tint: _drops > 0 ? BadgeTint.primary : BadgeTint.neutral,
                  child: Text('Drops: $_drops'),
                ),
              ],
            ),
            SizedBox(height: vars.spacing1),
            Text(
              position == null
                  ? 'Supported: ${DropRegion.isSupported}'
                  : 'At ${position.dx.round()}, ${position.dy.round()}',
              style: vars.mono,
            ),
            SizedBox(height: vars.spacing3),
            const Divider(),
            SizedBox(height: vars.spacing2),
            const SectionLabel('Last drop'),
            SizedBox(height: vars.spacing1),
            Expanded(child: _buildDropList(vars)),
          ],
        ),
      ),
    );
  }

  Widget _buildDropList(ThemeVariables vars) {
    final text = _droppedText;
    if (_droppedFiles.isEmpty && text == null) {
      return Center(
        child: Text(
          _drops == 0 ? 'Nothing dropped yet' : 'The drop was empty',
          style: vars.muted,
        ),
      );
    }
    return ListView(
      children: [
        for (final path in _droppedFiles)
          PreferenceRow(
            icon: const Icon(FluentIcons.document_20_regular),
            title: path.split(Platform.pathSeparator).last,
            // The full path is its own text: the GUI test looks for it.
            subtitle: path,
          ),
        if (text != null)
          PreferenceRow(
            icon: const Icon(FluentIcons.text_description_20_regular),
            title: 'Text: $text',
          ),
      ],
    );
  }

  Widget _buildDragPanel(ThemeVariables vars) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text('Drag out', style: vars.titleMedium),
        SizedBox(height: vars.spacing05),
        Text(
          'Into a file manager, an editor, or the pane on the left.',
          style: vars.muted,
        ),
        SizedBox(height: vars.spacing3),
        _DragCard(
          icon: FluentIcons.document_text_20_regular,
          title: 'Drag this note',
          subtitle: _file.path.split(Platform.pathSeparator).last,
          child: (card) => DragOutArea(
            filePaths: [_file.path],
            onDragEnded: _onDragEnded,
            child: card,
          ),
        ),
        SizedBox(height: vars.spacing2),
        _DragCard(
          icon: FluentIcons.text_description_20_regular,
          title: 'Drag this text',
          subtitle: 'Hello from nativeapi',
          child: (card) => DragOutArea(
            text: 'Hello from nativeapi',
            onDragEnded: _onDragEnded,
            child: card,
          ),
        ),
        SizedBox(height: vars.spacing4),
        Row(
          children: [
            Expanded(
              child: Text(
                'Last drag: $_lastDragResult',
                style: vars.labelStrong.copyWith(color: vars.colorContent),
              ),
            ),
            if (_lastDragResult != '-')
              Badge(
                size: WidgetSize.small,
                variant: BadgeVariant.tinted,
                tint: _lastDragResult == 'none'
                    ? BadgeTint.neutral
                    : BadgeTint.success,
                child: Text(_lastDragResult == 'none' ? 'not taken' : 'taken'),
              ),
          ],
        ),
        const Spacer(),
        Text('Supported: ${DragOutArea.isSupported}', style: vars.mono),
      ],
    );
  }

  void _onDragEnded(DragOperation operation) {
    setState(() => _lastDragResult = operation.name);
  }
}

/// A drag source: a raised card with a grab cursor, drawn here since it is
/// half of what the example is about.
class _DragCard extends StatelessWidget {
  const _DragCard({
    required this.icon,
    required this.title,
    required this.subtitle,
    required this.child,
  });

  final IconData icon;
  final String title;
  final String subtitle;
  final Widget Function(Widget card) child;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return MouseRegion(
      cursor: SystemMouseCursors.grab,
      child: child(
        Card(
          variant: CardVariant.raised,
          size: WidgetSize.small,
          child: Row(
            children: [
              Icon(icon, size: vars.iconMedium, color: vars.colorPrimary[600]),
              SizedBox(width: vars.spacing25),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      title,
                      style: vars.labelStrong.copyWith(
                        color: vars.colorContent,
                      ),
                    ),
                    Text(
                      subtitle,
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: vars.mono,
                    ),
                  ],
                ),
              ),
              Icon(
                FluentIcons.re_order_dots_vertical_20_regular,
                size: vars.iconSmall,
                color: vars.colorContentFaint,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
