import 'dart:async';

import 'package:dazzui_host/dazzui_host.dart';

import 'tabs_controller.dart';

/// The colour that marks [tab], from one of the theme's ramps, so it follows
/// light, dark and the Omarchy palette like everything else.
Color tabColor(ThemeVariables vars, BrowserTab tab) {
  final ramps = [
    vars.colorPrimary,
    vars.colorSuccess,
    vars.colorWarning,
    vars.colorDanger,
    vars.colorInfo,
    vars.colorNeutral,
  ];
  return ramps[tab.hue % ramps.length][500]!;
}

/// A stand-in for a web page, with state that would be lost if it were
/// rebuilt: an edited address, a counter, a scroll position, and a timer that
/// keeps running.
class TabPage extends StatefulWidget {
  const TabPage({super.key, required this.tab});

  final BrowserTab tab;

  @override
  State<TabPage> createState() => _TabPageState();
}

class _TabPageState extends State<TabPage> {
  static int _nextInstance = 1;

  final int _instance = _nextInstance++;
  late final TextEditingController _address = TextEditingController(
    text: 'https://example.com/tab-${widget.tab.id}',
  );
  final _scroll = ScrollController();
  final _openedAt = DateTime.now();
  late final Timer _timer;
  int _likes = 0;
  int _windowMoves = 0;
  int? _viewId;

  @override
  void initState() {
    super.initState();
    _timer = Timer.periodic(const Duration(seconds: 1), (_) => setState(() {}));
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    final viewId = View.of(context).viewId;
    if (_viewId != null && viewId != _viewId) _windowMoves++;
    _viewId = viewId;
  }

  @override
  void dispose() {
    _timer.cancel();
    _address.dispose();
    _scroll.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final tab = widget.tab;
    final vars = context.vars;
    final color = tabColor(vars, tab);
    final seconds = DateTime.now().difference(_openedAt).inSeconds;

    return ColoredBox(
      color: vars.colorSurface,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // A browser toolbar. Back, forward and reload have nowhere to go on
          // a stand-in page; the address is a real field whose edits are
          // part of the state that moves with the tab.
          Padding(
            padding: EdgeInsets.symmetric(
              horizontal: vars.spacing25,
              vertical: vars.spacing15,
            ),
            child: Row(
              spacing: vars.spacing05,
              children: [
                const IconButton(
                  semanticsLabel: 'Back',
                  icon: Icon(FluentIcons.arrow_left_20_regular),
                  onPressed: null,
                ),
                const IconButton(
                  semanticsLabel: 'Forward',
                  icon: Icon(FluentIcons.arrow_right_20_regular),
                  onPressed: null,
                ),
                const IconButton(
                  semanticsLabel: 'Reload',
                  icon: Icon(FluentIcons.arrow_clockwise_20_regular),
                  onPressed: null,
                ),
                SizedBox(width: vars.spacing1),
                Expanded(
                  child: TextField(
                    controller: _address,
                    size: WidgetSize.small,
                    prefix: Padding(
                      padding: EdgeInsets.only(left: vars.spacing2),
                      child: Icon(
                        FluentIcons.lock_closed_16_regular,
                        size: vars.iconSmall,
                        color: vars.colorContentMuted,
                      ),
                    ),
                  ),
                ),
              ],
            ),
          ),
          const Divider(),
          Expanded(
            child: ListView.builder(
              controller: _scroll,
              padding: EdgeInsets.all(vars.spacing6),
              itemCount: 30,
              itemBuilder: (context, i) {
                if (i == 0) {
                  return Padding(
                    padding: EdgeInsets.only(bottom: vars.spacing6),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(tab.title, style: vars.headlineSmall),
                        SizedBox(height: vars.spacing1),
                        Text(
                          'Page state #$_instance · open for ${seconds}s · '
                          'moved between windows $_windowMoves×',
                          style: vars.mono,
                        ),
                        SizedBox(height: vars.spacing4),
                        Row(
                          children: [
                            Button(
                              variant: ButtonVariant.tinted,
                              onPressed: () => setState(() => _likes++),
                              child: Row(
                                mainAxisSize: MainAxisSize.min,
                                spacing: vars.spacing15,
                                children: [
                                  const Icon(FluentIcons.thumb_like_20_regular),
                                  Text('Like ($_likes)'),
                                ],
                              ),
                            ),
                            SizedBox(width: vars.spacing3),
                            Expanded(
                              child: Text(
                                'Drag the tab to reorder, pull it down to tear '
                                'it off, drop it on another strip to merge.',
                                style: vars.muted,
                              ),
                            ),
                          ],
                        ),
                      ],
                    ),
                  );
                }
                // Placeholder paragraphs in the tab's colour, so the page
                // (and its scroll position) is recognisable in any window.
                return Container(
                  height: 64,
                  margin: EdgeInsets.only(bottom: vars.spacing3),
                  padding: EdgeInsets.symmetric(horizontal: vars.spacing4),
                  alignment: Alignment.centerLeft,
                  decoration: BoxDecoration(
                    color: color.withValues(
                      alpha: vars.washSurface + (i % 3) * 0.05,
                    ),
                    borderRadius: BorderRadius.circular(vars.radiusLarge),
                    border: Border.all(
                      color: color.withValues(alpha: vars.washEdge),
                      width: vars.strokeHairline,
                    ),
                  ),
                  child: Text('${tab.title} · paragraph $i'),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}
