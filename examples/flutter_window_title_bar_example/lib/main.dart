import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart' hide Button;

void main() {
  runApp(const TitleBarApp());
}

/// The strip this example draws where a title bar would be. It is always there,
/// whatever the title bar is doing, so that a window with no title bar and no
/// close button can still be moved and quit.
const double _stripHeight = 44;

/// How far the strip's own controls start from the left, so that they do not end
/// up under the macOS window buttons when those are on top of the content.
const double _buttonsInset = 78;

class TitleBarApp extends StatelessWidget {
  const TitleBarApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const Host(title: 'nativeapi · Title bar', home: TitleBarPage());
  }
}

class TitleBarPage extends StatefulWidget {
  const TitleBarPage({super.key});

  @override
  State<TitleBarPage> createState() => _TitleBarPageState();
}

class _TitleBarPageState extends State<TitleBarPage> {
  Window? _window;
  String _note = 'Try the three states and watch the strip at the top';
  final List<String> _log = [];
  int? _listenerId;
  int? _closeListenerId;
  bool _keepOpen = false;
  int _cancelledCloses = 0;

  @override
  void initState() {
    super.initState();
    final window = WindowManager.instance.getCurrent();
    if (window == null) return;
    _window = window;
    window.title = 'nativeapi · Title bar';
    window.minimumSize = const Size(520, 420).toNative();
    window.contentSize = const Size(620, 520).toNative();
    window.center();
    // The strip's Maximize / Restore follows the window however it changes:
    // its own button, the snap layouts, a double-click, a shortcut.
    // Every way of closing the window asks first: its close button, the
    // system menu, Alt+F4, the taskbar. Keep open cancels the request.
    _closeListenerId = window.addListener((event) {
      if (event is! WindowCloseRequestedEvent || !_keepOpen) return;
      if (!event.request.isCancelable) return; // The system insists.
      event.request.cancel();
      if (mounted) {
        setState(() {
          _cancelledCloses++;
          _note = 'Close cancelled — choose Allow to let it close';
        });
      }
    });
    _listenerId = WindowManager.instance.addListener((event) {
      if (event.windowId != window.id) return;
      if (event is WindowMaximizedEvent || event is WindowRestoredEvent) {
        if (mounted) {
          setState(
            () => _note = event is WindowMaximizedEvent
                ? 'Maximized'
                : 'Restored',
          );
        }
      }
    });
  }

  @override
  void dispose() {
    final id = _listenerId;
    if (id != null) WindowManager.instance.removeListener(id);
    final closeId = _closeListenerId;
    if (closeId != null) _window?.removeListener(closeId);
    super.dispose();
  }

  void _toggleMaximized() {
    final window = _window;
    if (window == null) return;
    window.isMaximized ? window.unmaximize() : window.maximize();
  }

  void _act(String what, void Function(Window window) change) {
    final window = _window;
    if (window == null) return;
    change(window);
    final size = window.contentSize.toSize();
    setState(() {
      _note = what;
      _log.insert(
        0,
        'titleBarStyle=${window.titleBarStyle.name} '
        'under=${window.isContentUnderTitleBar} '
        'buttons=${window.isWindowControlButtonsVisible} '
        'contentSize=${size.width.round()}×${size.height.round()}',
      );
      if (_log.length > 20) _log.removeLast();
    });
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final window = _window;
    final style = window?.titleBarStyle ?? TitleBarStyle.normal;
    final under = window?.isContentUnderTitleBar ?? false;
    final buttons = window?.isWindowControlButtonsVisible ?? false;
    final supported = Window.isContentUnderTitleBarSupported();
    final size = window?.contentSize.toSize() ?? Size.zero;

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _Strip(
          underTitleBar: style == TitleBarStyle.normal && under,
          // Hidden takes the system's buttons away; the strip brings its own.
          maximized: style == TitleBarStyle.hidden ? window?.isMaximized : null,
          onToggleMaximized: _toggleMaximized,
          onQuit: () => Application.instance.quit(0),
        ),
        OptionRow(
          label: 'Title bar',
          children: [
            OptionChip(
              label: 'Normal',
              selected: style == TitleBarStyle.normal && !under,
              onTap: () => _act('Standard title bar', (w) {
                w.setContentUnderTitleBar(false);
                w.titleBarStyle = TitleBarStyle.normal;
              }),
            ),
            OptionChip(
              label: 'Content under title bar',
              selected: style == TitleBarStyle.normal && under,
              onTap: supported
                  ? () => _act(
                      'The bar is a transparent overlay; its buttons stay',
                      (w) {
                        w.titleBarStyle = TitleBarStyle.normal;
                        w.setContentUnderTitleBar(true);
                      },
                    )
                  : null,
            ),
            OptionChip(
              label: 'Hidden',
              selected: style == TitleBarStyle.hidden,
              onTap: () => _act(
                'No title bar and no window buttons — move me by the strip',
                (w) => w.titleBarStyle = TitleBarStyle.hidden,
              ),
            ),
          ],
        ),
        OptionRow(
          label: 'Buttons',
          children: [
            OptionChip(
              label: 'Show buttons',
              selected: buttons,
              onTap: () => _act(
                'Buttons shown',
                (w) => w.isWindowControlButtonsVisible = true,
              ),
            ),
            OptionChip(
              label: 'Hide buttons',
              selected: !buttons,
              onTap: () => _act(
                'Buttons hidden',
                (w) => w.isWindowControlButtonsVisible = false,
              ),
            ),
            const Hint('A style resets these; set them after it'),
          ],
        ),
        OptionRow(
          label: 'Closing',
          children: [
            OptionChip(
              label: 'Allow',
              selected: !_keepOpen,
              onTap: () => setState(() {
                _keepOpen = false;
                _note = 'Closing the window closes it';
              }),
            ),
            OptionChip(
              label: 'Keep open',
              selected: _keepOpen,
              onTap: () => setState(() {
                _keepOpen = true;
                _note = 'Close requests are cancelled; try the × or the menu';
              }),
            ),
            Hint('Cancelled closes: $_cancelledCloses'),
          ],
        ),
        Expanded(
          child: SingleChildScrollView(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                Table(
                  children: [
                    const TableHead(
                      children: [
                        TableCell(head: true, flex: 3, child: Text('Now')),
                        TableCell(head: true, flex: 2, child: Text('Value')),
                      ],
                    ),
                    _row('titleBarStyle', style.name),
                    _row('isContentUnderTitleBar', '$under'),
                    _row('isWindowControlButtonsVisible', '$buttons'),
                    _row('isContentUnderTitleBarSupported()', '$supported'),
                    _row(
                      'contentSize',
                      '${size.width.round()} × ${size.height.round()}',
                    ),
                  ],
                ),
                Padding(
                  padding: EdgeInsets.all(vars.spacing4),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      const SectionLabel('What to look for'),
                      SizedBox(height: vars.spacing2),
                      const _Bullet(
                        'Hidden leaves no title bar and no window buttons on '
                        'every platform. Move the window by the strip above; '
                        'Quit ends the application (core has no '
                        'Window.close() yet).',
                      ),
                      const _Bullet(
                        'Content under title bar keeps the bar and its '
                        'buttons but stops it drawing: the strip runs to the '
                        'top edge behind them. macOS only — elsewhere the '
                        'chip is greyed out and the call returns false.',
                      ),
                      const _Bullet(
                        'Switching states keeps the window where it is and '
                        'the same size; only contentSize above changes, by '
                        'the height of the title bar.',
                      ),
                      const _Bullet(
                        'Hidden brings a Maximize / Restore chip into the '
                        'strip. It is marked as the window\'s maximize '
                        'button, so on Windows 11 resting the pointer on it '
                        'opens the snap layouts, as on the system\'s own.',
                      ),
                      const _Bullet(
                        'Hidden also stops the system moving the window when '
                        'you drag the top of the content — that is what the '
                        'strip is for.',
                      ),
                      const _Bullet(
                        'isContentUnderTitleBar stays as it was set while '
                        'Hidden is on: with no title bar there is nothing for '
                        'it to do, and it takes effect again on Normal.',
                      ),
                    ],
                  ),
                ),
              ],
            ),
          ),
        ),
        EventFooter(
          headline: _note,
          lines: _log,
          onClear: _log.isEmpty ? null : () => setState(_log.clear),
        ),
      ],
    );
  }

  Widget _row(String name, String value) => Builder(
    builder: (context) {
      final vars = context.vars;
      return TableRow(
        children: [
          TableCell(flex: 3, child: Text(name, style: vars.mono)),
          TableCell(flex: 2, child: Text(value, style: vars.labelStrong)),
        ],
      );
    },
  );
}

/// The example's own title bar — the subject of the demo, so it is drawn here
/// rather than taken from the kit, in the theme's chrome colours. Under a real
/// title bar it is just a strip; with the content under the title bar it runs
/// up behind the window buttons, which is why its controls start clear of
/// them.
class _Strip extends StatelessWidget {
  const _Strip({
    required this.underTitleBar,
    required this.maximized,
    required this.onToggleMaximized,
    required this.onQuit,
  });

  final bool underTitleBar;

  /// Whether the window is maximized, or null to leave out the strip's own
  /// Maximize / Restore button (the system's title bar has one).
  final bool? maximized;
  final VoidCallback onToggleMaximized;
  final VoidCallback onQuit;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return DragToMoveArea(
      child: DecoratedBox(
        decoration: BoxDecoration(
          color: vars.colorSurfaceChrome,
          border: Border(
            bottom: BorderSide(
              color: vars.colorBorder,
              width: vars.strokeHairline,
            ),
          ),
        ),
        child: SizedBox(
          height: _stripHeight,
          child: Padding(
            padding: EdgeInsets.fromLTRB(
              underTitleBar ? _buttonsInset : vars.spacing4,
              0,
              vars.spacing25,
              0,
            ),
            child: Row(
              children: [
                Icon(
                  FluentIcons.re_order_dots_vertical_20_regular,
                  size: vars.iconMedium,
                  color: vars.colorContentMuted,
                ),
                SizedBox(width: vars.spacing15),
                Expanded(
                  child: Text(
                    'Drag this strip to move the window',
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: vars.labelStrong.copyWith(color: vars.colorContent),
                  ),
                ),
                if (maximized != null) ...[
                  // Marked as the window's maximize button: on Windows 11,
                  // resting the pointer on it opens the snap layouts.
                  MaximizeButtonArea(
                    child: ActionChip(
                      label: maximized! ? 'Restore' : 'Maximize',
                      onTap: onToggleMaximized,
                    ),
                  ),
                  SizedBox(width: vars.spacing15),
                ],
                ActionChip(label: 'Quit', onTap: onQuit),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class _Bullet extends StatelessWidget {
  const _Bullet(this.text);

  final String text;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final style = vars.bodySmall.copyWith(color: vars.colorContentMuted);
    return Padding(
      padding: EdgeInsets.only(bottom: vars.spacing2),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text('·  ', style: style),
          Expanded(child: Text(text, style: style)),
        ],
      ),
    );
  }
}
