import 'dart:async';
import 'dart:io' show Platform;

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart';

void main() {
  runApp(const VisualEffectApp());
}

/// macOS draws its title bar over the content, so a material would stop at the
/// bar; letting the content take the bar in makes it transparent and keeps the
/// window buttons on it, which is why the panel below starts clear of them.
/// Elsewhere the call does nothing and none is needed: Windows 11 draws the
/// material across its caption by itself.
final bool _contentUnderTitleBar = Window.isContentUnderTitleBarSupported();

/// The shared host, transparent: whatever the page leaves unpainted is the
/// material, and what it paints sits on top of it.
class VisualEffectApp extends StatelessWidget {
  const VisualEffectApp({super.key});

  @override
  Widget build(BuildContext context) => const Host(
    title: 'Visual effect',
    transparent: true,
    home: VisualEffectPage(),
  );
}

class VisualEffectPage extends StatefulWidget {
  const VisualEffectPage({super.key});

  @override
  State<VisualEffectPage> createState() => _VisualEffectPageState();
}

class _VisualEffectPageState extends State<VisualEffectPage> {
  Window? _window;

  /// A plain red window behind this one, so that there is something to see
  /// through the material wherever the example happens to be on the desktop.
  Window? _backdrop;

  String _note = 'Pick an effect';

  @override
  void initState() {
    super.initState();
    final window = WindowManager.instance.getCurrent();
    if (window == null) return;
    _window = window;
    window.title = 'Visual effect';
    window.contentSize = const Size(560, 480).toNative();
    window.center();
    window.setContentUnderTitleBar(true);
    if (Platform.environment['VISUAL_EFFECT_AUTOPLAY'] == '1') {
      unawaited(_autoplay());
    }
  }

  /// Walks through the effects without being asked, over the backdrop, and
  /// says on the console when each one is on screen. This is how
  /// tools/gui/flutter_visual_effect_test.py in the workspace repository
  /// watches the example without touching the mouse.
  Future<void> _autoplay() async {
    const step = Duration(milliseconds: 2500);
    await Future<void>.delayed(const Duration(seconds: 4));
    _toggleBackdrop();
    debugPrint('STEP backdrop shown');
    for (final effect in [
      ...VisualEffect.values.where((e) => e != VisualEffect.none),
      VisualEffect.none,
    ]) {
      await Future<void>.delayed(step);
      if (!mounted) return;
      _apply(effect);
      debugPrint('STEP ${effect.name} ${_note.split(' ').first.toLowerCase()}');
    }
  }

  @override
  void dispose() {
    _backdrop?.dispose();
    super.dispose();
  }

  void _apply(VisualEffect effect) {
    final window = _window;
    if (window == null) return;
    final applied = window.setVisualEffect(effect);
    setState(() {
      _note = applied ? 'Applied ${effect.name}' : 'Refused ${effect.name}';
    });
  }

  void _toggleBackdrop() {
    final window = _window;
    if (window == null) return;
    final existing = _backdrop;
    if (existing != null) {
      window.setParentWindow(null);
      existing.hide();
      existing.dispose();
      setState(() => _backdrop = null);
      return;
    }
    final backdrop = Window.create();
    if (backdrop == null) return;
    final frame = window.bounds;
    backdrop.title = 'Backdrop';
    // The backdrop is a test pattern, not part of the design: plain red makes
    // any see-through obvious, on screen and to the GUI test that samples it.
    backdrop.backgroundColor = const Color(0xFFFF0000).toNative();
    backdrop.bounds = frame.toRect().inflate(80).toNative();
    backdrop.show();
    // A child stays above its parent, whichever of the two is brought forward.
    window.setParentWindow(backdrop);
    window.focus();
    setState(() => _backdrop = backdrop);
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final current = _window?.visualEffect ?? VisualEffect.none;
    final hasEffect = current != VisualEffect.none;
    final refused = _note.startsWith('Refused');
    // A material stands in for the window's background, so it shows only where this
    // app paints nothing: with no effect the window paints the theme's canvas, with
    // one it paints only the panel and leaves the rest of the window bare. The
    // controls keep a panel of their own because a material takes its colour from
    // whatever is behind the window, which can be anything.
    return ColoredBox(
      color: hasEffect ? const Color(0x00000000) : vars.colorCanvas,
      child: Padding(
        padding: EdgeInsets.fromLTRB(
          vars.spacing5,
          _contentUnderTitleBar ? 46 : vars.spacing5,
          vars.spacing5,
          vars.spacing5,
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Card(
              variant: CardVariant.raised,
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      Expanded(
                        child: Text(
                          'Effect: ${current.name}',
                          style: vars.titleLarge,
                        ),
                      ),
                      Badge(
                        size: WidgetSize.small,
                        variant: BadgeVariant.tinted,
                        tint: refused
                            ? BadgeTint.danger
                            : hasEffect
                            ? BadgeTint.success
                            : BadgeTint.neutral,
                        child: Text(_note),
                      ),
                    ],
                  ),
                  SizedBox(height: vars.spacing3),
                  const SectionLabel('Window.setVisualEffect'),
                  SizedBox(height: vars.spacing15),
                  Wrap(
                    spacing: vars.spacing1,
                    runSpacing: vars.spacing1,
                    children: [
                      for (final effect in VisualEffect.values)
                        OptionChip(
                          label: effect.name,
                          selected: effect == current,
                          onTap: Window.isVisualEffectSupported(effect)
                              ? () => _apply(effect)
                              : null,
                        ),
                    ],
                  ),
                  SizedBox(height: vars.spacing15),
                  const Hint('Greyed out effects are not available here.'),
                  SizedBox(height: vars.spacing2),
                  const Divider(),
                  PreferenceRow(
                    title: 'Backdrop',
                    subtitle:
                        'A plain red window behind this one, to see through',
                    trailing: Switch(
                      value: _backdrop != null,
                      onChanged: (_) => _toggleBackdrop(),
                    ),
                  ),
                ],
              ),
            ),
            // Bare window below the panel: this is where the material shows.
            const Spacer(),
          ],
        ),
      ),
    );
  }
}
