import 'dart:async';

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi/nativeapi.dart' as na;

import 'capture_controller.dart';

void main() => runApp(const CaptureApp());

class CaptureApp extends StatelessWidget {
  const CaptureApp({super.key, this.controller});
  final CaptureController? controller;

  @override
  Widget build(BuildContext context) => Host(
    title: 'Screen Capture',
    home: CapturePage(controller: controller),
  );
}

class CapturePage extends StatefulWidget {
  const CapturePage({super.key, this.controller});
  final CaptureController? controller;
  @override
  State<CapturePage> createState() => _CapturePageState();
}

class _CapturePageState extends State<CapturePage> {
  late final CaptureController capture;
  final cropFields = [
    TextEditingController(text: '0'),
    TextEditingController(text: '0'),
    TextEditingController(text: '640'),
    TextEditingController(text: '480'),
  ];
  bool includeCursor = false, cropEnabled = false;
  double frameRate = 30;
  int bitrate = 0;

  @override
  void initState() {
    super.initState();
    capture = widget.controller ?? CaptureController();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted) unawaited(capture.initialize());
    });
  }

  @override
  void dispose() {
    for (final field in cropFields) {
      field.dispose();
    }
    capture.dispose();
    super.dispose();
  }

  na.ScreenCaptureOptions? options() {
    final values = cropFields
        .map((field) => int.tryParse(field.text.trim()))
        .toList();
    if (cropEnabled &&
        (values.any((value) => value == null) ||
            values[0]! < 0 ||
            values[1]! < 0 ||
            values[2]! <= 0 ||
            values[3]! <= 0)) {
      capture.reportInvalidOptions(
        'Crop needs whole pixel values: X/Y ≥ 0 and Width/Height > 0.',
      );
      return null;
    }
    return na.ScreenCaptureOptions(
      includeCursor: includeCursor,
      cropEnabled: cropEnabled,
      crop: na.Rectangle(
        x: (values[0] ?? 0).toDouble(),
        y: (values[1] ?? 0).toDouble(),
        width: (values[2] ?? 0).toDouble(),
        height: (values[3] ?? 0).toDouble(),
      ),
    );
  }

  void takeScreenshot() {
    final value = options();
    if (value != null) unawaited(capture.capture(value));
  }

  void startRecording() {
    final value = options();
    if (value != null) {
      unawaited(
        capture.start(
          na.ScreenRecordingOptions(
            capture: value,
            frameRate: frameRate,
            videoBitrate: bitrate,
          ),
        ),
      );
    }
  }

  Widget action(
    String label,
    VoidCallback? callback, {
    bool primary = false,
    bool danger = false,
  }) => Button(
    variant: primary ? ButtonVariant.filled : ButtonVariant.normal,
    tint: danger
        ? ButtonTint.danger
        : (primary ? ButtonTint.primary : ButtonTint.neutral),
    onPressed: callback,
    child: Text(label),
  );

  Widget section(String title, List<Widget> children) => Padding(
    padding: const EdgeInsets.only(bottom: 20),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Text(title, style: context.vars.titleMedium),
        const SizedBox(height: 10),
        ...children,
      ],
    ),
  );

  @override
  Widget build(BuildContext context) => ListenableBuilder(
    listenable: capture,
    builder: (context, _) {
      final vars = context.vars;
      return Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
            child: Row(
              children: [
                Icon(
                  FluentIcons.record_24_regular,
                  color: vars.colorPrimary[600],
                  size: 28,
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text('Screen Capture', style: vars.titleLarge),
                      Text(
                        'Screenshots & silent recordings',
                        style: vars.muted,
                      ),
                    ],
                  ),
                ),
                Badge(
                  variant: BadgeVariant.tinted,
                  tint: capture.active ? BadgeTint.danger : BadgeTint.neutral,
                  child: Text(
                    capture.active
                        ? '${capture.recordingState.name} · ${capture.elapsed.toStringAsFixed(1)} s'
                        : capture.recordingState.name,
                  ),
                ),
              ],
            ),
          ),
          const Divider(),
          Expanded(
            child: LayoutBuilder(
              builder: (context, constraints) {
                if (constraints.maxWidth < 880) {
                  return ListView(
                    padding: const EdgeInsets.all(20),
                    children: [
                      controls(),
                      SizedBox(height: 450, child: preview()),
                      SizedBox(height: 220, child: results()),
                    ],
                  );
                }
                return Row(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    SizedBox(
                      width: 350,
                      child: SingleChildScrollView(
                        padding: const EdgeInsets.all(24),
                        child: controls(),
                      ),
                    ),
                    const VerticalDivider(),
                    Expanded(
                      child: Padding(
                        padding: const EdgeInsets.all(24),
                        child: Column(
                          children: [
                            Expanded(child: preview()),
                            const SizedBox(height: 16),
                            SizedBox(height: 188, child: results()),
                          ],
                        ),
                      ),
                    ),
                  ],
                );
              },
            ),
          ),
          const Divider(),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 12),
            color: capture.error == null
                ? vars.colorSurfaceSunken
                : vars.colorDanger[50],
            child: Text(
              capture.status,
              maxLines: 3,
              overflow: TextOverflow.ellipsis,
              style: capture.error == null
                  ? vars.bodyMedium
                  : vars.bodyMedium.copyWith(color: vars.colorDanger[700]),
            ),
          ),
        ],
      );
    },
  );

  Widget controls() {
    final enabled = !capture.locked && capture.captureSupported;
    final canCapture = enabled && capture.hasSource;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        if (capture.ready && !capture.captureSupported)
          const Padding(
            padding: EdgeInsets.only(bottom: 16),
            child: Text(
              'Capture requires macOS 14+ or Windows 10 2004+ with a supported desktop session.',
            ),
          ),
        section('1  Choose a source', [
          action(
            'System picker…',
            enabled && capture.pickerSupported ? capture.selectSource : null,
            primary: true,
          ),
          const SizedBox(height: 8),
          Wrap(
            spacing: 6,
            runSpacing: 6,
            children: [
              action('This window', enabled ? capture.selectThisWindow : null),
              for (var i = 0; i < capture.displays.length; i++)
                action(
                  'Display ${i + 1}${capture.displays[i].isPrimary ? ' · primary' : ''}',
                  enabled
                      ? () => capture.selectDisplay(capture.displays[i])
                      : null,
                ),
            ],
          ),
          const SizedBox(height: 10),
          Text(
            capture.sourceLabel,
            style: context.vars.bodyMedium,
            maxLines: 2,
            overflow: TextOverflow.ellipsis,
          ),
          if (capture.hasSource)
            Align(
              alignment: Alignment.centerLeft,
              child: action(
                'Release source',
                !capture.locked ? capture.releaseSource : null,
              ),
            ),
        ]),
        section('2  Capture options', [
          Wrap(
            spacing: 6,
            runSpacing: 6,
            children: [
              OptionChip(
                label: 'Include cursor',
                selected: includeCursor,
                onTap: !capture.locked
                    ? () => setState(() => includeCursor = !includeCursor)
                    : null,
              ),
              OptionChip(
                label: 'Crop region',
                selected: cropEnabled,
                onTap: !capture.locked
                    ? () => setState(() => cropEnabled = !cropEnabled)
                    : null,
              ),
            ],
          ),
          if (cropEnabled) ...[
            const SizedBox(height: 10),
            Row(
              children: [
                for (var i = 0; i < 4; i++)
                  Expanded(
                    child: Padding(
                      padding: EdgeInsets.only(right: i == 3 ? 0 : 6),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(
                            ['X', 'Y', 'Width', 'Height'][i],
                            style: context.vars.muted,
                          ),
                          const SizedBox(height: 4),
                          TextField(
                            controller: cropFields[i],
                            enabled: !capture.locked,
                            size: WidgetSize.small,
                          ),
                        ],
                      ),
                    ),
                  ),
              ],
            ),
            const SizedBox(height: 6),
            Text(
              'Source pixels, not desktop coordinates. The region must fit inside the captured image.',
              style: context.vars.muted,
            ),
          ],
          const SizedBox(height: 12),
          Wrap(
            spacing: 6,
            runSpacing: 6,
            children: [
              for (final fps in [15.0, 30.0, 60.0])
                OptionChip(
                  label: '${fps.toInt()} fps',
                  selected: frameRate == fps,
                  onTap: !capture.locked
                      ? () => setState(() => frameRate = fps)
                      : null,
                ),
            ],
          ),
          const SizedBox(height: 6),
          Wrap(
            spacing: 6,
            runSpacing: 6,
            children: [
              for (final mbps in [0, 4, 8, 16])
                OptionChip(
                  label: mbps == 0 ? 'Auto bitrate' : '$mbps Mbps',
                  selected: bitrate == mbps * 1000000,
                  onTap: !capture.locked
                      ? () => setState(() => bitrate = mbps * 1000000)
                      : null,
                ),
            ],
          ),
        ]),
        section('3  Capture & record', [
          Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              action(
                'Take screenshot',
                canCapture ? takeScreenshot : null,
                primary: true,
              ),
              action(
                'Save PNG',
                !capture.locked && capture.hasImage
                    ? capture.saveScreenshot
                    : null,
              ),
            ],
          ),
          const SizedBox(height: 10),
          if (!capture.active)
            action(
              'Start recording',
              canCapture && capture.recordingSupported ? startRecording : null,
              primary: true,
            )
          else
            Wrap(
              spacing: 8,
              runSpacing: 8,
              children: [
                action(
                  'Stop & save',
                  capture.recordingState == na.ScreenRecordingState.recording
                      ? capture.stop
                      : null,
                  primary: true,
                ),
                action(
                  'Discard',
                  capture.recordingState == na.ScreenRecordingState.starting ||
                          capture.recordingState ==
                              na.ScreenRecordingState.recording
                      ? capture.cancel
                      : null,
                  danger: true,
                ),
              ],
            ),
          const SizedBox(height: 8),
          Text(
            'MP4 · H.264 · SDR · no audio. Wait for “saved” before opening a recording.',
            style: context.vars.muted,
          ),
        ]),
        section('Output folder', [
          Text(
            capture.outputDirectory.isEmpty
                ? 'Preparing…'
                : capture.outputDirectory,
            style: context.vars.mono,
            maxLines: 3,
            overflow: TextOverflow.ellipsis,
          ),
          const SizedBox(height: 8),
          Wrap(
            spacing: 8,
            runSpacing: 8,
            children: [
              action(
                'Choose folder…',
                !capture.locked ? capture.chooseOutputDirectory : null,
              ),
              action(
                'Open folder',
                capture.outputDirectory.isNotEmpty
                    ? () => capture.openPath(
                        capture.outputDirectory,
                        directory: true,
                      )
                    : null,
              ),
            ],
          ),
          const SizedBox(height: 8),
          Text(
            'Starts in a temporary folder. Choose another folder to keep your captures.',
            style: context.vars.muted,
          ),
        ]),
        Text(
          'On macOS, allow Screen & System Audio Recording in System Settings if direct capture is denied. Reopen the app if macOS asks you to.',
          style: context.vars.muted,
        ),
      ],
    );
  }

  Widget preview() => Card(
    variant: CardVariant.sunken,
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Row(
          children: [
            Text('Screenshot preview', style: context.vars.titleMedium),
            const Spacer(),
            if (capture.hasImage)
              Text(
                '${capture.imageWidth} × ${capture.imageHeight} px',
                style: context.vars.mono,
              ),
          ],
        ),
        const SizedBox(height: 14),
        Expanded(
          child: ClipRect(
            child: capture.preview == null
                ? Center(
                    child: Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        Icon(
                          FluentIcons.image_24_regular,
                          size: 48,
                          color: context.vars.colorContentMuted,
                        ),
                        const SizedBox(height: 14),
                        const Text('Your next screenshot appears here'),
                        const SizedBox(height: 6),
                        Text(
                          'Choose “This window” for a quick first capture.',
                          style: context.vars.muted,
                        ),
                      ],
                    ),
                  )
                : InteractiveViewer(
                    minScale: 0.5,
                    maxScale: 5,
                    child: Center(
                      child: Image.memory(
                        capture.preview!,
                        fit: BoxFit.contain,
                        gaplessPlayback: true,
                        errorBuilder: (_, _, _) => const Center(
                          child: Text(
                            'Preview could not be decoded. You can still save the PNG.',
                          ),
                        ),
                      ),
                    ),
                  ),
          ),
        ),
        const SizedBox(height: 10),
        Text(
          capture.preview == null ? 'PNG screenshots · physical pixels' : 'Scroll to zoom · drag to pan · this is a still image, not a live feed',
          style: context.vars.muted,
        ),
      ],
    ),
  );

  Widget results() => Column(
    crossAxisAlignment: CrossAxisAlignment.stretch,
    children: [
      Text(
        'Saved captures (${capture.artifacts.length})',
        style: context.vars.titleMedium,
      ),
      const SizedBox(height: 8),
      if (capture.artifacts.isEmpty)
        Text(
          'Saved PNGs and completed MP4s appear here.',
          style: context.vars.muted,
        )
      else
        Expanded(
          child: ListView.separated(
            itemCount: capture.artifacts.length,
            separatorBuilder: (_, _) => const Divider(),
            itemBuilder: (context, index) {
              final artifact = capture.artifacts[index];
              return Padding(
                padding: const EdgeInsets.symmetric(vertical: 9),
                child: Row(
                  children: [
                    Icon(
                      artifact.isVideo
                          ? FluentIcons.video_24_regular
                          : FluentIcons.image_24_regular,
                    ),
                    const SizedBox(width: 10),
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Text(artifact.name, overflow: TextOverflow.ellipsis),
                          Text(
                            artifact.isVideo
                                ? 'MP4 · ${artifact.duration!.toStringAsFixed(1)} s'
                                : 'PNG · ${artifact.width} × ${artifact.height} px',
                            style: context.vars.muted,
                          ),
                        ],
                      ),
                    ),
                    action(
                      artifact.isVideo ? 'Play' : 'Open',
                      () => capture.openPath(artifact.path),
                    ),
                  ],
                ),
              );
            },
          ),
        ),
    ],
  );
}
