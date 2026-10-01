// dazzui's Preferences is the settings column; this file means nativeapi's.
import 'package:dazzui_host/dazzui_host.dart' hide Preferences;
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show
        AccessibilityManager,
        DisplayManager,
        Preferences,
        TrayManager,
        UrlOpener,
        WindowManager;

void main() {
  runApp(const CocoapodsExampleApp());
}

class CocoapodsExampleApp extends StatelessWidget {
  const CocoapodsExampleApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const Host(
      title: 'CocoaPods nativeapi smoke test',
      home: NativeApiSmokeTestPage(),
    );
  }
}

/// A header with the tally and the Run button, a note on why the example
/// exists, and one card per check.
class NativeApiSmokeTestPage extends StatefulWidget {
  const NativeApiSmokeTestPage({super.key});

  @override
  State<NativeApiSmokeTestPage> createState() => _NativeApiSmokeTestPageState();
}

class _NativeApiSmokeTestPageState extends State<NativeApiSmokeTestPage> {
  final List<_CheckResult> _results = [];
  bool _isRunning = false;

  @override
  void initState() {
    super.initState();
    _runChecks();
  }

  Future<void> _runChecks() async {
    setState(() {
      _isRunning = true;
      _results.clear();
    });

    final results = <_CheckResult>[
      _check('UrlOpener support', () {
        final supported = UrlOpener.instance.isSupported();
        return 'supported: $supported';
      }),
      _check('TrayManager support', () {
        final supported = TrayManager.instance.isSupported();
        return 'supported: $supported';
      }),
      _check('Accessibility state', () {
        final enabled = AccessibilityManager.instance.isEnabled();
        return 'enabled: $enabled';
      }),
      _check('DisplayManager primary display', () {
        final primary = DisplayManager.instance.getPrimary();
        if (primary == null) {
          return 'no primary display';
        }
        return '${primary.name} ${primary.size.width.toInt()}x${primary.size.height.toInt()}';
      }),
      _check('WindowManager current window', () {
        final current = WindowManager.instance.getCurrent();
        return current == null
            ? 'no active native window'
            : (current.title ?? '');
      }),
      _check('Preferences read/write', () {
        final prefs = Preferences.createWithScope('cocoapods_example')!;
        const key = 'smoke_test';
        final value = DateTime.now().toIso8601String();
        final wrote = prefs.set(key, value);
        final readValue = prefs.get(key, '');
        final removed = prefs.remove(key);
        prefs.dispose();
        return 'wrote: $wrote, matched: ${readValue == value}, removed: $removed';
      }),
    ];

    if (!mounted) {
      return;
    }

    setState(() {
      _results.addAll(results);
      _isRunning = false;
    });
  }

  _CheckResult _check(String name, String Function() body) {
    try {
      return _CheckResult.success(name, body());
    } catch (error, stackTrace) {
      debugPrintStack(label: '$name failed: $error', stackTrace: stackTrace);
      return _CheckResult.failure(name, error.toString());
    }
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        _header(vars),
        const Divider(),
        Expanded(
          child: SingleChildScrollView(
            padding: EdgeInsets.all(vars.spacing4),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              spacing: vars.spacing4,
              children: [
                const Callout(
                  tint: CalloutTint.info,
                  title: Text('Built through CocoaPods'),
                  message: Text(
                    'Swift Package Manager is off for this project, so it '
                    'checks that nativeapi still builds the CocoaPods way. '
                    'Each card calls one nativeapi module.',
                  ),
                ),
                const SectionLabel('Checks'),
                _checkGrid(vars),
              ],
            ),
          ),
        ),
      ],
    );
  }

  Widget _header(ThemeVariables vars) {
    final failed = _results.where((result) => !result.ok).length;
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing4,
        vertical: vars.spacing25,
      ),
      child: Row(
        spacing: vars.spacing2,
        children: [
          Expanded(
            child: Text(
              'CocoaPods nativeapi smoke test',
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.titleMedium,
            ),
          ),
          if (_isRunning)
            const Spinner(size: WidgetSize.small)
          else if (_results.isNotEmpty)
            Badge(
              size: WidgetSize.small,
              variant: BadgeVariant.tinted,
              tint: failed == 0 ? BadgeTint.success : BadgeTint.danger,
              child: Text(
                failed == 0
                    ? '${_results.length}/${_results.length} passed'
                    : '$failed failed',
              ),
            ),
          Button(
            variant: ButtonVariant.filled,
            onPressed: _isRunning ? null : _runChecks,
            child: Text(_isRunning ? 'Running...' : 'Run checks'),
          ),
        ],
      ),
    );
  }

  /// Two columns when the window is wide enough, one on a phone.
  Widget _checkGrid(ThemeVariables vars) {
    return LayoutBuilder(
      builder: (context, constraints) {
        final columns = constraints.maxWidth >= 560 ? 2 : 1;
        final gap = vars.spacing2;
        final width = (constraints.maxWidth - gap * (columns - 1)) / columns;
        return Wrap(
          spacing: gap,
          runSpacing: gap,
          children: [
            for (final result in _results)
              SizedBox(
                width: width,
                child: _CheckCard(result: result),
              ),
          ],
        );
      },
    );
  }
}

class _CheckCard extends StatelessWidget {
  const _CheckCard({required this.result});

  final _CheckResult result;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final ramp = result.ok ? vars.colorSuccess : vars.colorDanger;
    return Card(
      variant: CardVariant.sunken,
      size: WidgetSize.small,
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        spacing: vars.spacing2,
        children: [
          Icon(
            result.ok
                ? FluentIcons.checkmark_circle_20_filled
                : FluentIcons.error_circle_20_filled,
            color: ramp.shade600,
          ),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              spacing: vars.spacing05,
              children: [
                Text(result.name, style: vars.titleSmall),
                Text(
                  result.detail,
                  maxLines: 2,
                  overflow: TextOverflow.ellipsis,
                  style: vars.mono,
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

class _CheckResult {
  const _CheckResult({
    required this.name,
    required this.detail,
    required this.ok,
  });

  factory _CheckResult.success(String name, String detail) {
    return _CheckResult(name: name, detail: detail, ok: true);
  }

  factory _CheckResult.failure(String name, String detail) {
    return _CheckResult(name: name, detail: detail, ok: false);
  }

  final String name;
  final String detail;
  final bool ok;
}
