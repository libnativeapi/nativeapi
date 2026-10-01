import 'dart:io';

import 'package:dazzui_host/dazzui_host.dart';

import '../icon_animations.dart';
import '../icon_animator.dart';
import '../tray_controller.dart';

/// Magnified view of the selected tray icon.
///
/// It paints [IconAnimator.lastFrame] — the very image that was just handed to
/// the tray — so the window and the tray can be compared frame by frame.
class LivePreview extends StatelessWidget {
  const LivePreview({super.key, required this.controller});

  final TrayController controller;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final entry = controller.selected;

    return Container(
      height: 112,
      padding: EdgeInsets.all(vars.spacing25),
      color: vars.colorSurfaceSunken,
      child: entry == null
          ? Center(
              child: Text(
                'No tray icon. Add one to start.',
                style: vars.bodyMedium.copyWith(color: vars.colorContentSubtle),
              ),
            )
          : ListenableBuilder(
              listenable: entry.animator,
              builder: (context, _) => Row(
                children: [
                  Stack(
                    children: [
                      // Live widgets being screenshotted for "Any widget".
                      // They have to be painted, so they sit under the
                      // (opaque) preview box instead of being Offstage.
                      for (final other in controller.entries)
                        if (other.animator.animation == IconAnimation.widget)
                          Positioned(
                            left: 30,
                            top: 30,
                            child: RepaintBoundary(
                              key: other.animator.captureKey,
                              child: CounterBadge(
                                time: other.animator.time,
                                color: other.animator.color,
                                size: kIconPoints,
                              ),
                            ),
                          ),
                      _frameBox(entry.animator, vars),
                    ],
                  ),
                  SizedBox(width: vars.spacing25),
                  Expanded(child: _details(entry, vars)),
                ],
              ),
            ),
    );
  }

  Widget _frameBox(IconAnimator animator, ThemeVariables vars) {
    final frame = animator.lastFrame;
    // macOS uses the image as a template: only alpha counts and the menu bar
    // picks the tint, so here the ink does. Other platforms show the pixels
    // as they are.
    final tint = Platform.isMacOS ? vars.colorContent : null;
    return SizedBox(
      width: 84,
      height: 84,
      child: Card(
        variant: CardVariant.sunken,
        size: WidgetSize.small,
        child: Center(
          child: frame == null
              ? null
              : RawImage(
                  image: frame,
                  width: 56,
                  height: 56,
                  fit: BoxFit.contain,
                  // Show the real pixels; smoothing would hide the resolution.
                  filterQuality: FilterQuality.none,
                  color: tint,
                  colorBlendMode: tint == null ? null : BlendMode.srcIn,
                ),
        ),
      ),
    );
  }

  Widget _details(TrayEntry entry, ThemeVariables vars) {
    final animator = entry.animator;
    final name = animator.animation?.label ?? '${_stillLabel(entry)} icon';
    final state = !animator.isPlaying
        ? 'still'
        : animator.paused
        ? 'paused'
        : 'playing';
    final px = animator.lastFrame?.width ?? animator.pixelSize;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('$name · $state', style: vars.titleSmall),
        if (animator.isPlaying) ...[
          Text(
            'frame ${animator.frames} · '
            '${animator.measuredFps.toStringAsFixed(1)} fps',
            style: vars.mono,
          ),
          Text(
            'render ${animator.renderMs.toStringAsFixed(1)} ms · $px×$px px',
            style: vars.mono,
          ),
          Text(
            'dropped ${animator.dropped} · same frame as tray',
            style: vars.mono,
          ),
        ] else ...[
          Text('$px×$px px · same image as tray', style: vars.mono),
          Text('pick an animation below to play it', style: vars.mono),
        ],
        const Spacer(),
        Wrap(
          spacing: vars.spacing1,
          children: [
            ActionChip(
              label: animator.paused ? 'Resume' : 'Pause',
              onTap: animator.isPlaying ? animator.togglePaused : null,
            ),
            ActionChip(
              label: 'Step',
              onTap: animator.isPlaying ? animator.step : null,
            ),
            ActionChip(
              label: 'Stop',
              // Back to the asset icon; a scene also gives back the title
              // and tooltip it took.
              onTap: !animator.isPlaying
                  ? null
                  : entry.scene != null
                  ? controller.resetScene
                  : () => controller.setStill(StillIcon.asset),
            ),
          ],
        ),
      ],
    );
  }

  String _stillLabel(TrayEntry entry) => switch (entry.still) {
    StillIcon.asset => 'Asset',
    StillIcon.drawn => 'Drawn',
    StillIcon.base64 => 'Base64',
    null => 'Last frame',
  };
}
