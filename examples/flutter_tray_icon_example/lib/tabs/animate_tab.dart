import 'dart:io';

import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/scheduler.dart';

import '../icon_animations.dart';
import '../tray_controller.dart';

/// The gallery: every tile is alive, one click plays it in the tray.
class AnimateTab extends StatefulWidget {
  const AnimateTab({super.key, required this.controller});

  final TrayController controller;

  @override
  State<AnimateTab> createState() => _AnimateTabState();
}

class _AnimateTabState extends State<AnimateTab>
    with SingleTickerProviderStateMixin {
  // One clock for all tiles. They only preview the look; the tray frames come
  // from the selected icon's IconAnimator.
  final ValueNotifier<double> _time = ValueNotifier(0);
  late final Ticker _ticker;

  static const _colors = <String, Color?>{
    'Auto': null,
    'Blue': Color(0xFF2F7DE1),
    'Amber': Color(0xFFF0A020),
    'Red': Color(0xFFE5484D),
  };

  @override
  void initState() {
    super.initState();
    _ticker = createTicker((elapsed) {
      _time.value = elapsed.inMicroseconds / 1000000.0;
    })..start();
  }

  @override
  void dispose() {
    _ticker.dispose();
    _time.dispose();
    super.dispose();
  }

  Widget _tile(IconAnimation animation, IconAnimation? playing) => SizedBox(
    height: 64,
    child: _Tile(
      animation: animation,
      time: _time,
      selected: playing == animation,
      onTap: () => widget.controller.play(animation),
    ),
  );

  @override
  Widget build(BuildContext context) {
    final controller = widget.controller;
    final entry = controller.selected;
    if (entry == null) return const SizedBox.shrink();
    final animator = entry.animator;
    final vars = context.vars;

    return ListenableBuilder(
      listenable: animator,
      builder: (context, _) => ListView(
        children: [
          Padding(
            padding: EdgeInsets.only(left: vars.spacing25, top: vars.spacing25),
            child: const SectionLabel('Animations'),
          ),
          Padding(
            padding: EdgeInsets.all(vars.spacing25),
            // Plain rows rather than a GridView: a UI probe reads box offsets,
            // and sliver grids keep their children's elsewhere.
            child: Column(
              children: [
                for (var row = 0; row < 2; row++) ...[
                  if (row > 0) SizedBox(height: vars.spacing15),
                  Row(
                    children: [
                      for (var column = 0; column < 4; column++) ...[
                        if (column > 0) SizedBox(width: vars.spacing15),
                        Expanded(
                          child: _tile(
                            IconAnimation.values[row * 4 + column],
                            animator.animation,
                          ),
                        ),
                      ],
                    ],
                  ),
                ],
              ],
            ),
          ),
          OptionRow(
            label: 'Icon presets',
            children: [
              OptionChip(
                label: 'Download',
                selected: entry.scene == Scene.download,
                onTap: () => entry.scene == Scene.download
                    ? controller.resetScene()
                    : controller.playScene(Scene.download),
              ),
              OptionChip(
                label: 'Recording',
                selected: entry.scene == Scene.recording,
                onTap: () => entry.scene == Scene.recording
                    ? controller.resetScene()
                    : controller.playScene(Scene.recording),
              ),
              OptionChip(
                label: 'Syncing',
                selected: entry.scene == Scene.syncing,
                onTap: () => entry.scene == Scene.syncing
                    ? controller.resetScene()
                    : controller.playScene(Scene.syncing),
              ),
              ActionChip(
                label: 'Three icons',
                onTap: controller.playThreeAtOnce,
              ),
            ],
          ),
          const Divider(),
          Padding(
            padding: EdgeInsets.only(left: vars.spacing25, top: vars.spacing25),
            child: const SectionLabel('Frames'),
          ),
          OptionRow(
            label: 'Still icon',
            children: [
              for (final kind in StillIcon.values)
                OptionChip(
                  label: switch (kind) {
                    StillIcon.asset => 'Asset',
                    StillIcon.drawn => 'Drawn',
                    StillIcon.base64 => 'Base64',
                  },
                  selected: entry.still == kind,
                  onTap: () => controller.setStill(kind),
                ),
            ],
          ),
          OptionRow(
            label: 'Rate',
            children: [
              for (final fps in const [10, 24, 30, 60])
                OptionChip(
                  label: '$fps fps',
                  selected: animator.fps == fps,
                  onTap: () => controller.setFps(fps),
                ),
            ],
          ),
          OptionRow(
            label: 'Resolution',
            children: [
              for (final scale in const [1, 2, 3])
                OptionChip(
                  label: '${scale}x',
                  selected: animator.scale == scale,
                  onTap: () => controller.setScale(scale),
                ),
              Hint('${animator.pixelSize} px per side'),
            ],
          ),
          OptionRow(
            label: 'Color',
            children: [
              for (final MapEntry(:key, :value) in _colors.entries)
                OptionChip(
                  label: key,
                  selected: animator.color == (value ?? controller.autoColor),
                  onTap: () =>
                      controller.setColor(value ?? controller.autoColor),
                ),
              if (Platform.isMacOS) const Hint('macOS tints it itself'),
            ],
          ),
        ],
      ),
    );
  }
}

/// One gallery tile: an [OptionCard] whose title is the live drawing over
/// the animation's name, so a chosen tile is drawn the way the system draws
/// any chosen card.
class _Tile extends StatelessWidget {
  const _Tile({
    required this.animation,
    required this.time,
    required this.selected,
    required this.onTap,
  });

  final IconAnimation animation;
  final ValueNotifier<double> time;
  final bool selected;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return OptionCard(
      title: animation.label,
      selected: selected,
      onPressed: onTap,
      padding: EdgeInsets.symmetric(
        vertical: vars.spacing1,
        horizontal: vars.spacing2,
      ),
      titleContent: Builder(
        builder: (context) {
          // The card's own label colour, for the drawing too.
          final color = DefaultTextStyle.of(context).style.color!;
          return Center(
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                animation == IconAnimation.widget
                    ? CounterBadge(time: time, color: color, size: 28)
                    : RepaintBoundary(
                        child: CustomPaint(
                          size: const Size.square(28),
                          painter: IconAnimationPainter(animation, time, color),
                        ),
                      ),
                SizedBox(height: vars.spacing1),
                Text(animation.label),
              ],
            ),
          );
        },
      ),
    );
  }
}
