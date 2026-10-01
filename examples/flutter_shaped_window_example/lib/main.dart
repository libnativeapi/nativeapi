// ignore_for_file: invalid_use_of_internal_member, implementation_imports

import 'dart:async';
import 'dart:ui' show AppExitType;

import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/foundation.dart' show defaultTargetPlatform;
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:flutter/src/foundation/_features.dart' show isWindowingEnabled;
import 'package:flutter/src/widgets/_window.dart' as fw;
import 'package:nativeapi_flutter/nativeapi_flutter.dart' as na;
import 'package:nativeapi_flutter/windowing.dart';

import 'shape_geometry.dart';
import 'widgets/shape_art.dart';

void main() {
  isWindowingEnabled = true;
  WidgetsFlutterBinding.ensureInitialized();
  runWidget(const ShapeDemo());
}

class _CloseDelegate with fw.RegularWindowControllerDelegate {
  _CloseDelegate(this.close);
  final VoidCallback close;
  @override
  void onWindowCloseRequested(fw.RegularWindowController controller) => close();
}

class ShapeDemo extends StatefulWidget {
  const ShapeDemo({super.key});
  @override
  State<ShapeDemo> createState() => _ShapeDemoState();
}

class _ShapeDemoState extends State<ShapeDemo>
    with WidgetsBindingObserver, TickerProviderStateMixin {
  late final _main = fw.RegularWindowController(
    size: const Size(480, 680),
    title: 'Window shapes',
    delegate: _CloseDelegate(_close),
  );
  static const _resizeChannel = MethodChannel('shape_demo/resize');
  bool get _keepsSurfaceDuringTransition =>
      defaultTargetPlatform == TargetPlatform.macOS ||
      defaultTargetPlatform == TargetPlatform.windows ||
      defaultTargetPlatform == TargetPlatform.linux;
  late final fw.RegularWindowController _demo = fw.RegularWindowController(
    size: const Size(320, 320),
    title: 'Shape preview',
    delegate: _CloseDelegate(_hidePreview),
  );

  void _hidePreview() {
    _stopTransition();
    _nativePreview?.hide();
  }

  late final na.Window? _nativePreview = _demo.nativeWindow;
  DemoShape _shape = DemoShape.circle;
  double _size = 320;
  double _nativeSize = 320;
  double _fromSize = 320;
  double _toSize = 320;
  Timer? _transitionTimer;
  double? _pendingProgress;
  bool _applyingTransition = false;
  int _transitionRevision = 0;
  Timer? _shadowFrameTimer;
  late final AnimationController _shadowAnimation = AnimationController(
    vsync: this,
    duration: const Duration(milliseconds: 220),
  )..addListener(_scheduleShadowFrame);
  late (double, double, double, double, Color) _shadowFrom;
  late (double, double, double, double, Color) _shadowTo;
  bool _shadowFadeOut = false;

  void _scheduleShadowFrame() {
    _shadowFrameTimer ??= Timer(Duration.zero, () {
      _shadowFrameTimer = null;
      if (!mounted || _closing) return;
      final t = Curves.easeInOut.transform(_shadowAnimation.value);
      double mix(double a, double b) => a + (b - a) * t;
      setState(() {
        _shadowOpacity = mix(_shadowFrom.$1, _shadowTo.$1);
        _shadowBlur = mix(_shadowFrom.$2, _shadowTo.$2);
        _shadowX = mix(_shadowFrom.$3, _shadowTo.$3);
        _shadowY = mix(_shadowFrom.$4, _shadowTo.$4);
        _shadowColor = Color.lerp(_shadowFrom.$5, _shadowTo.$5, t)!;
      });
      _applyShadowParameters();
      if (_shadowAnimation.value == 1 && _shadowFadeOut) {
        setState(() => _shadowEnabled = false);
        _nativePreview?.hasShadow = false;
      }
    });
  }

  void _stopShadowAnimation() {
    _shadowAnimation.stop();
    _shadowFrameTimer?.cancel();
    _shadowFrameTimer = null;
  }

  int _count = 0;
  bool _editingShadow = false;
  ShapeLook get _look => ShapeLook.forShape(_shape);
  bool _closing = false;
  String _status = 'Preparing preview…';
  // Linux clips content in Flutter; core alone renders and positions its shadow.
  // Shape coordinates and content size never include the native shadow gutter.
  late final bool _usesInputShape = na.Window.isInputShapeSupported();
  bool _shadowEnabled = true;
  Color _shadowColor = const Color(0xFF000000);
  double _shadowOpacity = 0.3;
  double _shadowBlur = 18;
  double _shadowX = 0;
  double _shadowY = 6;
  String? _shadowError;
  bool _rectangleRestored = false;
  bool _applyPending = false;
  bool _previewReady = false;
  late List<Offset> _points = morphContours(_size)[_shape]!;
  List<Offset> _from = [];
  List<Offset> _to = [];
  bool _targetRectangle = false;
  late final AnimationController _morph = AnimationController(
    vsync: this,
    duration: const Duration(milliseconds: 450),
  )..addListener(_animateFrame);

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
    // Configure the native handle after Flutter has attached both window views.
    // On Windows it is not ready while this State is being initialized.
    SchedulerBinding.instance.addPostFrameCallback((_) {
      if (mounted && !_closing) _configurePreview();
    });
  }

  void _configurePreview() {
    final window = _nativePreview;
    if (window != null) {
      window.titleBarStyle = na.TitleBarStyle.hidden;
      window.backgroundColor = const Color(0x00000000).toNative();
      window.hasShadow = _shadowEnabled;
      if (!_applyShadowParameters()) return;
      window.isResizable = false;
      // Wayland ignores absolute positioning. Keep the preview above its own
      // controller window instead of letting the latter cover the silhouette.
      if (_usesInputShape) window.setParentWindow(_main.nativeWindow);
      window.contentSize = Size.square(_size).toNative();
      final area = na.DisplayManager.instance.getPrimary()?.workArea.toRect();
      if (area != null) {
        _main.nativeWindow?.position = Offset(
          area.left + 60,
          area.top + 100,
        ).toNative();
        window.position = Offset(area.left + 590, area.top + 150).toNative();
      }
    }
    _previewReady = true;
    _scheduleApply();
  }

  @override
  void dispose() {
    _transitionTimer?.cancel();
    _shadowFrameTimer?.cancel();
    _shadowAnimation.dispose();
    _morph.dispose();
    _nativePreview?.dispose();
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeMetrics() {
    // Active animation frames already replace the contour after resizing.
    // Reapplying it from every metrics callback duplicates native/shadow work.
    if (_previewReady && !_rectangleRestored && !_applyingTransition) {
      _scheduleApply();
    }
  }

  void _scheduleApply() {
    if (_applyPending || _closing) return;
    _applyPending = true;
    SchedulerBinding.instance.addPostFrameCallback((_) {
      _applyPending = false;
      if (!_rectangleRestored) {
        if (_morph.isAnimating) {
          _applyPoints(_points, report: false);
        } else {
          _apply();
        }
      }
    });
    SchedulerBinding.instance.ensureVisualUpdate();
  }

  void _apply() {
    _applyPoints(_points);
  }

  bool _applyPoints(List<Offset> points, {bool report = true}) {
    if (!mounted || _closing) return false;
    final window = _nativePreview;
    if (window == null) return false;
    final shape = na.WindowShape.create();
    if (shape == null) {
      setState(() => _status = 'Could not allocate a shape.');
      return false;
    }
    try {
      for (final point in points) {
        if (!shape.addPoint(point.toNative())) {
          setState(() => _status = 'Invalid polygon.');
          return false;
        }
      }
      final ok = _usesInputShape
          ? window.setInputShape(shape)
          : window.setShape(shape);
      setState(() {
        if (ok) {
          _points = points;
          _rectangleRestored = false;
        }
        _status = ok
            ? _usesInputShape
                  ? '${_shape.name}: Flutter clip + native input region (${shape.pointCount} vertices)'
                  : '${_shape.name}: native shape active (${shape.pointCount} vertices)'
            : 'Could not apply shape. Keeping the previous contour.';
      });
      if (report) {
        debugPrint(
          '[shape] $_status; isShaped=${window.isShaped}; '
          'isInputShaped=${window.isInputShaped}',
        );
        if (!window.isVisible) window.show();
      }
      return ok;
    } finally {
      // The window copied the points and does not retain the builder.
      shape.dispose();
    }
  }

  void _stopTransition() {
    _morph.stop();
    _transitionTimer?.cancel();
    _transitionTimer = null;
    _pendingProgress = null;
    _transitionRevision++;
  }

  void _animateFrame() {
    _pendingProgress = Curves.easeInOutCubic.transform(_morph.value);
    _scheduleTransitionFrame();
  }

  void _scheduleTransitionFrame() {
    if (_transitionTimer != null || _applyingTransition) return;
    // Native resize can synchronously pump Flutter's event loop. Run at idle,
    // outside handleBeginFrame, and serialize/coalesce any reentrant ticks.
    _transitionTimer = Timer(Duration.zero, () async {
      _transitionTimer = null;
      final progress = _pendingProgress;
      _pendingProgress = null;
      if (!mounted || _closing || progress == null) return;
      _applyingTransition = true;
      try {
        await _applyTransitionFrame(progress);
      } on PlatformException catch (error) {
        _stopTransition();
        if (mounted && !_closing) {
          setState(
            () => _status = 'Could not resize preview: ${error.message}',
          );
        }
      } finally {
        _applyingTransition = false;
        if (_pendingProgress != null) _scheduleTransitionFrame();
      }
    });
  }

  Future<void> _applyTransitionFrame(double t) async {
    final revision = _transitionRevision;
    // Keep window geometry on whole logical pixels.
    final nextSize = (_fromSize + (_toSize - _fromSize) * t).roundToDouble();
    if (_keepsSurfaceDuringTransition) {
      // Keep a stable backing surface during the animation. Reallocating the
      // Flutter surface for every pixel step makes the native host wait for raster.
      final capacity = _fromSize > _toSize ? _fromSize : _toSize;
      if (_nativeSize < capacity) await _resizePreviewSurface(capacity);
    } else if (nextSize != _size) {
      _nativePreview?.contentSize = Size.square(nextSize).toNative();
    }
    if (revision != _transitionRevision || !mounted || _closing) return;
    final points = interpolateContour(_from, _to, t);
    if (_usesInputShape) {
      // Submit the Flutter clip before publishing its matching native contour.
      // Applying the shadow at idle first lets GTK paint the new silhouette over
      // the previous Flutter frame. Serialize this pair before consuming a tick.
      setState(() {
        _size = nextSize;
        _points = points;
      });
      await WidgetsBinding.instance.endOfFrame;
      if (revision != _transitionRevision || !mounted || _closing) return;
    } else if (nextSize != _size) {
      setState(() => _size = nextSize);
    }
    if (!_applyPoints(points, report: false)) {
      _stopTransition();
      return;
    }
    if (t == 1) {
      if (_keepsSurfaceDuringTransition && _nativeSize != _toSize) {
        // Finish the endpoint layout before tightening the native bounds.
        // This waits for framework layout, not for GPU completion.
        await WidgetsBinding.instance.endOfFrame;
        if (revision != _transitionRevision || !mounted || _closing) return;
        await _resizePreviewSurface(_toSize);
        if (revision != _transitionRevision || !mounted || _closing) return;
      }
      if (_targetRectangle) {
        _clearShape();
      } else {
        _apply();
      }
    }
  }

  Future<void> _resizePreviewSurface(double size) async {
    if (defaultTargetPlatform == TargetPlatform.macOS) {
      await _resizeChannel.invokeMethod<void>('setContentSize', {
        'window': _nativePreview!.nativeObject.address,
        'size': size,
      });
    } else {
      _nativePreview?.contentSize = Size.square(size).toNative();
    }
    _nativeSize = size;
  }

  void _transitionTo(DemoShape? shape, {double? targetSize}) {
    // Retarget from the displayed frame, including when clicks interrupt a morph.
    _stopTransition();
    _from = List.of(_points);
    _fromSize = _size;
    _toSize = targetSize ?? _toSize;
    _to = morphContours(_toSize)[shape]!;
    _targetRectangle = shape == null;
    _rectangleRestored = false;
    if (WidgetsBinding
        .instance
        .platformDispatcher
        .accessibilityFeatures
        .disableAnimations) {
      // Apply directly even when the controller already finished at value 1.
      _pendingProgress = 1;
      _scheduleTransitionFrame();
    } else {
      _morph.forward(from: 0);
    }
  }

  void selectShape(DemoShape shape) {
    setState(() => _shape = shape);
    _transitionTo(shape);
  }

  void resizePreview() {
    // Toggle the destination, not a potentially fractional in-flight size.
    // A restored rectangle stays rectangular; interrupted shape morphs keep
    // their intended silhouette while retargeting from the current frame.
    _transitionTo(
      _rectangleRestored || _targetRectangle ? null : _shape,
      targetSize: _toSize == 320 ? 400 : 320,
    );
  }

  void restoreRectangle() => _transitionTo(null);

  bool _applyShadowParameters() {
    final window = _nativePreview;
    final shadow = na.WindowShadow.create();
    String? error;
    try {
      if (window == null || shadow == null) {
        error = 'Could not configure the preview shadow.';
      } else {
        shadow.color = _shadowColor
            .withValues(alpha: _shadowOpacity)
            .toNative();
        final valid =
            shadow.setBlurRadius(_shadowBlur) &&
            shadow.setOffset(Offset(_shadowX, _shadowY).toNative());
        if (!valid || !window.setCustomShadow(shadow)) {
          error = 'Could not apply the shadow parameters.';
        }
      }
    } finally {
      shadow?.dispose();
    }
    if (mounted) setState(() => _shadowError = error);
    return error == null;
  }

  void _changeShadow(VoidCallback change) {
    _stopShadowAnimation();
    setState(change);
    _applyShadowParameters();
  }

  void _resetShadowParameters() => _changeShadow(() {
    _shadowColor = const Color(0xFF000000);
    _shadowOpacity = 0.3;
    _shadowBlur = 18;
    _shadowX = 0;
    _shadowY = 6;
  });

  void toggleShadow() {
    _stopShadowAnimation();
    setState(() => _shadowEnabled = !_shadowEnabled);
    _nativePreview?.hasShadow = _shadowEnabled;
    debugPrint('[shadow] hasShadow=${_nativePreview?.hasShadow}');
  }

  void _clearShape() {
    final window = _nativePreview;
    final ok =
        (_usesInputShape
            ? window?.setInputShape(null)
            : window?.setShape(null)) ??
        false;
    setState(() {
      if (ok) {
        _rectangleRestored = true;
      }
      _status = ok ? 'Rectangle restored' : 'Could not restore shape';
    });
    debugPrint('[shape] $_status');
  }

  void _close() {
    if (_closing) return;
    _stopTransition();
    _stopShadowAnimation();
    setState(() => _closing = true);
    SchedulerBinding.instance.addPostFrameCallback((_) {
      _demo.destroy();
      _main.destroy();
      ServicesBinding.instance.exitApplication(AppExitType.required);
    });
  }

  static const _shadowPresets = <String, (double, double, double, Color)>{
    'Soft': (.30, 18, 6, Color(0xFF000000)),
    'Float': (.32, 32, 14, Color(0xFF000000)),
    'Sharp': (.40, 3, 5, Color(0xFF000000)),
    'Glow': (.55, 28, 0, Color(0xFF9864EF)),
  };

  void _selectShadowPreset(String name) {
    _stopShadowAnimation();
    if (name == 'None' && !_shadowEnabled) return;
    _shadowFrom = (
      _shadowEnabled ? _shadowOpacity : 0,
      _shadowBlur,
      _shadowX,
      _shadowY,
      _shadowColor,
    );
    _shadowFadeOut = name == 'None';
    if (_shadowFadeOut) {
      _shadowTo = (0, _shadowBlur, _shadowX, _shadowY, _shadowColor);
    } else {
      final p = _shadowPresets[name]!;
      _shadowTo = (p.$1, p.$2, 0, p.$3, p.$4);
      if (!_shadowEnabled) {
        setState(() {
          _shadowOpacity = 0;
          _shadowEnabled = true;
        });
        _applyShadowParameters();
        _nativePreview?.hasShadow = true;
      }
    }
    if (WidgetsBinding
        .instance
        .platformDispatcher
        .accessibilityFeatures
        .disableAnimations) {
      _shadowAnimation.value = 1;
      _scheduleShadowFrame();
    } else {
      _shadowAnimation.forward(from: 0);
    }
  }

  String get _selectedShadowPreset {
    if (!_shadowEnabled) return 'None';
    for (final entry in _shadowPresets.entries) {
      final p = entry.value;
      if (_shadowOpacity == p.$1 &&
          _shadowBlur == p.$2 &&
          _shadowY == p.$3 &&
          _shadowX == 0 &&
          _shadowColor == p.$4) {
        return entry.key;
      }
    }
    return 'Custom';
  }

  // -- the preview window --------------------------------------------------

  // The preview cannot sit in a [Host]: its surface has to stay transparent
  // outside the silhouette (Linux clips it in Flutter), and it has no pages,
  // toasts or text fields. It keeps a bare WidgetsApp with the host's theme.
  Widget _buildPreviewWindow() {
    return fw.RegularWindow(
      controller: _demo,
      child: WidgetsApp(
        debugShowCheckedModeBanner: false,
        color: const Color(0x00000000),
        builder: (context, _) =>
            HostTheme(child: Builder(builder: _previewContent)),
      ),
    );
  }

  /// The subject of the demo: the silhouette, drawn by the example. Only its
  /// counter is a DazzUI control; the art and the drag handle are the shape.
  Widget _previewContent(BuildContext context) {
    final vars = context.vars;
    return DefaultTextStyle.merge(
      style: const TextStyle(color: ShapeLook.ink),
      child: Align(
        alignment: Alignment.topLeft,
        child: SizedBox(
          width: _size,
          height: _size,
          child: ClipPath(
            clipper: _usesInputShape && !_rectangleRestored
                ? PolygonClipper(_points)
                : null,
            child: ShapeArt(
              look: _look,
              child: Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    MouseRegion(
                      cursor: SystemMouseCursors.move,
                      child: GestureDetector(
                        behavior: HitTestBehavior.opaque,
                        onPanStart: (_) => _nativePreview?.startDragging(),
                        child: Padding(
                          padding: EdgeInsets.all(vars.spacing25),
                          child: Text(
                            '⠿  DRAG ME',
                            style: vars.captionSmall.copyWith(
                              letterSpacing: 2,
                              color: ShapeLook.ink.withValues(alpha: .87),
                            ),
                          ),
                        ),
                      ),
                    ),
                    Text(
                      _rectangleRestored ? 'rectangle' : _shape.name,
                      style: vars.headlineLarge.copyWith(
                        color: ShapeLook.ink,
                        fontWeight: FontWeight.w700,
                        letterSpacing: -1,
                      ),
                    ),
                    SizedBox(height: vars.spacing25),
                    Button(
                      variant: ButtonVariant.normal,
                      tint: ButtonTint.neutral,
                      onPressed: () => setState(() => _count++),
                      child: Text('Tap · $_count'),
                    ),
                    SizedBox(height: vars.spacing25),
                    Text(
                      _look.name.toUpperCase(),
                      style: vars.captionSmall.copyWith(
                        letterSpacing: 3,
                        color: ShapeLook.ink.withValues(alpha: .8),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }

  // -- the control window --------------------------------------------------

  /// What the control window shows. The window itself is built once, so the
  /// preview's animation frames do not rebuild it; its page depends on
  /// [_ControlsScope] and rebuilds only when this changes.
  Object _controlsConfiguration() => (
    _shape,
    _editingShadow,
    _count,
    _toSize,
    _status,
    _rectangleRestored,
    _shadowEnabled,
    _shadowColor,
    _shadowOpacity,
    _shadowBlur,
    _shadowX,
    _shadowY,
    _shadowError,
  );

  late final Widget _controlsWindow = fw.RegularWindow(
    controller: _main,
    child: Host(
      title: 'Window shapes',
      home: Builder(
        builder: (context) {
          _ControlsScope.watch(context);
          return _controlsPage(context);
        },
      ),
    ),
  );

  Widget _controlsPage(BuildContext context) {
    final vars = context.vars;
    // The page is paper rather than the host's canvas: the gallery's cards
    // are the muted surface on it, and the Linux demo script finds this
    // window by the plain band under the header.
    return ColoredBox(
      color: vars.colorSurface,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          _banner(vars),
          _collectionHeading(vars),
          Expanded(child: _editingShadow ? _shadowControls() : _gallery(vars)),
          if (!_editingShadow) ...[
            const Divider(),
            OptionRow(
              label: 'Window',
              children: [
                ActionChip(
                  label: 'Apply shape',
                  onTap: () => selectShape(_shape),
                ),
                ActionChip(label: 'Restore rectangle', onTap: restoreRectangle),
              ],
            ),
            OptionRow(
              label: 'Size',
              children: [
                ActionChip(label: 'Toggle size', onTap: resizePreview),
                Text(
                  '${_toSize.toInt()} × ${_toSize.toInt()} · 450 ms',
                  style: vars.mono,
                ),
              ],
            ),
          ],
          _shadowHeading(vars),
          _presetBar(vars),
          _statusBar(vars),
        ],
      ),
    );
  }

  /// The playground's banner, in the selected shape's first colour. It is
  /// part of the art rather than chrome, and its height and dark start are
  /// what `tools/gui/flutter_window_shape_demo_linux.py` finds the window by.
  Widget _banner(ThemeVariables vars) => Container(
    height: 48,
    padding: EdgeInsets.symmetric(horizontal: vars.spacing4),
    decoration: BoxDecoration(
      gradient: LinearGradient(colors: [ShapeLook.night, _look.colors.first]),
    ),
    child: Row(
      children: [
        Text(
          'Outside the box.',
          style: vars.headlineSmall.copyWith(
            color: ShapeLook.ink,
            fontWeight: FontWeight.w700,
          ),
        ),
        const Spacer(),
        Text(
          'SHAPE PLAYGROUND',
          style: vars.captionSmall.copyWith(
            letterSpacing: 1.3,
            color: ShapeLook.ink.withValues(alpha: .85),
          ),
        ),
      ],
    ),
  );

  Widget _collectionHeading(ThemeVariables vars) => Padding(
    padding: EdgeInsets.fromLTRB(
      vars.spacing35,
      vars.spacing3,
      vars.spacing35,
      vars.spacing2,
    ),
    child: SizedBox(
      height: vars.controlTinySize,
      child: Row(
        children: [
          SectionLabel(_editingShadow ? 'CUSTOM SHADOW' : 'SHAPE COLLECTION'),
          const Spacer(),
          if (_editingShadow)
            ActionChip(
              label: 'Done',
              primary: true,
              onTap: () => setState(() => _editingShadow = false),
            )
          else
            Text('${DemoShape.values.length} silhouettes', style: vars.mono),
        ],
      ),
    ),
  );

  /// Four columns by three rows, never scrolling: the demo scripts aim at the
  /// cards by this grid.
  Widget _gallery(ThemeVariables vars) => Padding(
    padding: EdgeInsets.symmetric(horizontal: vars.spacing3),
    child: Column(
      children: [
        for (var row = 0; row < 3; row++)
          Expanded(
            child: Padding(
              padding: EdgeInsets.only(bottom: vars.spacing2),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  for (var col = 0; col < 4; col++) ...[
                    if (col > 0) SizedBox(width: vars.spacing2),
                    Expanded(
                      child: _shapeCard(DemoShape.values[row * 4 + col], vars),
                    ),
                  ],
                ],
              ),
            ),
          ),
      ],
    ),
  );

  Widget _shapeCard(DemoShape shape, ThemeVariables vars) {
    final selected = !_rectangleRestored && shape == _shape;
    return LayoutBuilder(
      builder: (context, constraints) {
        // The card draws its picture edge to edge; its border sits inside the
        // box, so leave room for the chosen card's thicker one.
        final inset = 2 * vars.strokeControl;
        return OptionCard(
          title: shape.name,
          selected: selected,
          onPressed: () => selectShape(shape),
          padding: EdgeInsets.zero,
          titleContent: SizedBox(
            width: (constraints.maxWidth - inset).clamp(0, double.infinity),
            height: (constraints.maxHeight - inset).clamp(0, double.infinity),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                ClipPath(
                  clipper: ShapeClipper(shape, 46),
                  child: SizedBox.square(
                    dimension: 46,
                    child: ShapeArt(look: ShapeLook.forShape(shape)),
                  ),
                ),
                SizedBox(height: vars.spacing15),
                Text(shape.name),
              ],
            ),
          ),
        );
      },
    );
  }

  Widget _shadowHeading(ThemeVariables vars) => Padding(
    padding: EdgeInsets.fromLTRB(
      vars.spacing35,
      vars.spacing25,
      vars.spacing35,
      vars.spacing15,
    ),
    child: Row(
      children: [
        const SectionLabel('SHADOW'),
        SizedBox(width: vars.spacing2),
        // The preset's name: the demo scripts read it back above the presets.
        Text(_selectedShadowPreset, style: vars.mono),
        const Spacer(),
        OptionChip(
          label: _editingShadow ? 'Back to shapes' : 'Adjust…',
          selected: _editingShadow,
          onTap: () => setState(() => _editingShadow = !_editingShadow),
        ),
      ],
    ),
  );

  Widget _presetBar(ThemeVariables vars) {
    final preset = _selectedShadowPreset;
    return Padding(
      padding: EdgeInsets.fromLTRB(
        vars.spacing3,
        0,
        vars.spacing3,
        vars.spacing3,
      ),
      child: SegmentedControl<String>(
        stretch: true,
        items: [
          for (final name in ['None', ..._shadowPresets.keys])
            SegmentedItem(value: name, label: name),
        ],
        // A slider-made shadow is none of them.
        value: preset == 'Custom' ? null : preset,
        onChanged: _selectShadowPreset,
      ),
    );
  }

  Widget _statusBar(ThemeVariables vars) => Column(
    crossAxisAlignment: CrossAxisAlignment.stretch,
    children: [
      const Divider(),
      Container(
        padding: EdgeInsets.symmetric(
          horizontal: vars.spacing3,
          vertical: vars.spacing2,
        ),
        color: vars.colorSurfaceSunken,
        child: Text(
          _shadowError ?? _status,
          maxLines: 2,
          overflow: TextOverflow.ellipsis,
          style: _shadowError == null
              ? vars.mono
              : vars.mono.copyWith(color: vars.colorDanger.shade600),
        ),
      ),
    ],
  );

  Widget _shadowSlider(
    String label,
    double value,
    double min,
    double max,
    ValueChanged<double> onChanged, {
    bool percent = false,
  }) {
    final display = percent
        ? '${(value * 100).round()}%'
        : '${value.round()} px';
    return Builder(
      builder: (context) {
        final vars = context.vars;
        // The same row as an [OptionRow], with a slider for its chips.
        return Column(
          children: [
            Padding(
              padding: EdgeInsets.symmetric(
                horizontal: vars.spacing25,
                vertical: vars.spacing05,
              ),
              child: Row(
                children: [
                  SizedBox(width: 66, child: Text(label, style: vars.muted)),
                  Expanded(
                    child: Padding(
                      padding: EdgeInsets.only(left: vars.spacing2),
                      child: Slider(
                        values: [value],
                        min: min,
                        max: max,
                        step: percent ? 0.01 : 1,
                        largeStep: percent ? 0.1 : 8,
                        size: WidgetSize.small,
                        valueLabel: display,
                        semanticsLabel: label,
                        onChanged: (values) =>
                            _changeShadow(() => onChanged(values.first)),
                      ),
                    ),
                  ),
                ],
              ),
            ),
            const Divider(),
          ],
        );
      },
    );
  }

  Widget _shadowControls() {
    const colors = <String, Color>{
      'Black': Color(0xFF000000),
      'Purple': Color(0xFF6558F5),
      'Blue': Color(0xFF1976D2),
      'Rose': Color(0xFFE74779),
      'Green': Color(0xFF00897B),
    };
    return Builder(
      builder: (context) {
        final vars = context.vars;
        return SingleChildScrollView(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              const Divider(),
              OptionRow(
                label: 'Shadow',
                children: [
                  Switch(
                    value: _shadowEnabled,
                    size: WidgetSize.small,
                    onChanged: (_) => toggleShadow(),
                  ),
                  const Hint('Contour shadow'),
                ],
              ),
              OptionRow(
                label: 'Color',
                children: [
                  for (final entry in colors.entries)
                    OptionChip(
                      label: entry.key,
                      selected: _shadowColor == entry.value,
                      onTap: () =>
                          _changeShadow(() => _shadowColor = entry.value),
                    ),
                ],
              ),
              _shadowSlider(
                'Opacity',
                _shadowOpacity,
                0,
                1,
                (v) => _shadowOpacity = v,
                percent: true,
              ),
              _shadowSlider(
                'Blur radius',
                _shadowBlur,
                0,
                64,
                (v) => _shadowBlur = v,
              ),
              _shadowSlider(
                'Horizontal',
                _shadowX,
                -64,
                64,
                (v) => _shadowX = v,
              ),
              _shadowSlider('Vertical', _shadowY, -64, 64, (v) => _shadowY = v),
              OptionRow(
                label: 'Defaults',
                children: [
                  ActionChip(
                    label: 'Reset shadow parameters',
                    onTap: _resetShadowParameters,
                  ),
                ],
              ),
              if (!_shadowEnabled)
                Padding(
                  padding: EdgeInsets.all(vars.spacing25),
                  child: const Callout(
                    tint: CalloutTint.info,
                    size: WidgetSize.small,
                    message: Text(
                      'Shadow hidden. Changes appear when enabled.',
                    ),
                  ),
                ),
              if (_shadowError != null)
                Padding(
                  padding: EdgeInsets.all(vars.spacing25),
                  child: Callout(
                    tint: CalloutTint.danger,
                    size: WidgetSize.small,
                    message: Text(_shadowError!),
                  ),
                ),
            ],
          ),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    if (_closing) return const ViewCollection(views: []);
    return _ControlsScope(
      configuration: _controlsConfiguration(),
      child: ViewCollection(views: [_controlsWindow, _buildPreviewWindow()]),
    );
  }
}

/// Carries what the control window shows, so its page rebuilds when that
/// changes and not on every frame of the preview's animation.
class _ControlsScope extends InheritedWidget {
  const _ControlsScope({required this.configuration, required super.child});

  final Object configuration;

  static void watch(BuildContext context) =>
      context.dependOnInheritedWidgetOfExactType<_ControlsScope>();

  @override
  bool updateShouldNotify(_ControlsScope oldWidget) =>
      oldWidget.configuration != configuration;
}
