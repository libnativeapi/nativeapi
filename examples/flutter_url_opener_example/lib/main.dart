import 'dart:io' show Platform;

import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show SizeToNative, UrlOpenResult, UrlOpener, WindowManager;

void main() {
  runApp(const UrlOpenerExampleApp());
}

class UrlOpenerExampleApp extends StatelessWidget {
  const UrlOpenerExampleApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const Host(
      title: 'URL Opener Example',
      home: UrlOpenerExamplePage(),
    );
  }
}

/// Header with the support badge, the URL field, preset chips, the last
/// result, and the call log.
class UrlOpenerExamplePage extends StatefulWidget {
  const UrlOpenerExamplePage({super.key});

  @override
  State<UrlOpenerExamplePage> createState() => _UrlOpenerExamplePageState();
}

class _UrlOpenerExamplePageState extends State<UrlOpenerExamplePage> {
  /// Picking one only fills the field; Open URL is what opens it.
  static const _presets = [
    'https://flutter.dev',
    'https://github.com/libnativeapi',
    'mailto:hello@example.com',
    'not a url',
  ];

  final TextEditingController _urlController = TextEditingController(
    text: 'https://flutter.dev',
  );

  bool _isSupported = false;
  bool _isOpening = false;
  String _status = 'Checking platform support...';
  UrlOpenResult? _lastResult;

  // What the calls answered, newest first.
  String _lastEvent = 'No URL opened yet';
  final List<String> _log = [];

  @override
  void initState() {
    super.initState();
    _urlController.addListener(() => setState(() {}));
    _refreshSupport();
    if (Platform.isMacOS || Platform.isWindows || Platform.isLinux) {
      // A compact panel rather than the runners' default window.
      WidgetsBinding.instance.addPostFrameCallback((_) {
        WindowManager.instance.getCurrent()?.contentSize = const Size(
          520,
          480,
        ).toNative();
      });
    }
  }

  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }

  void _refreshSupport() {
    final supported = UrlOpener.instance.isSupported();
    setState(() {
      _isSupported = supported;
      _status = supported
          ? 'URL opening is supported on this platform.'
          : 'URL opening is not supported on this platform.';
    });
  }

  void _checkSupportPressed() {
    _refreshSupport();
    _report(_status, 'isSupported() → $_isSupported');
  }

  void _openUrl() {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      setState(() {
        _status = 'Enter a URL first.';
        _lastResult = null;
      });
      _report(_status);
      return;
    }

    setState(() {
      _isOpening = true;
      _status = 'Opening $url ...';
      _lastResult = null;
    });

    final result = UrlOpener.instance.open(url);

    setState(() {
      _isOpening = false;
      _lastResult = result;
      _status = result.success
          ? 'Successfully handed URL to the system.'
          : 'Open failed.';
    });
    _report(
      _status,
      'open("$url") → ${result.success} · ${result.errorCode.name}',
    );
    Toaster.of(context).add(
      ToastOptions(
        title: result.success ? 'Opened $url' : 'Could not open $url',
        tint: result.success ? ToastTint.success : ToastTint.danger,
        timeout: const Duration(seconds: 4),
      ),
    );
  }

  void _report(String message, [String? call]) {
    setState(() {
      _lastEvent = message;
      _log.insert(0, call ?? message);
      if (_log.length > 50) _log.removeLast();
    });
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
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
                _urlField(vars),
                _presetChips(vars),
                _resultCard(vars),
              ],
            ),
          ),
        ),
        EventFooter(
          headline: _lastEvent,
          lines: _log,
          onClear: _log.isEmpty
              ? null
              : () => setState(() {
                  _log.clear();
                  _lastEvent = 'No URL opened yet';
                }),
        ),
      ],
    );
  }

  Widget _header(ThemeVariables vars) {
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing4,
        vertical: vars.spacing25,
      ),
      child: Row(
        spacing: vars.spacing2,
        children: [
          Expanded(child: Text('URL Opener Example', style: vars.titleMedium)),
          Badge(
            size: WidgetSize.small,
            variant: BadgeVariant.tinted,
            tint: _isSupported ? BadgeTint.success : BadgeTint.danger,
            child: Text(_isSupported ? 'Supported' : 'Not supported'),
          ),
          Button(
            variant: ButtonVariant.normal,
            tint: ButtonTint.neutral,
            onPressed: _checkSupportPressed,
            child: const Text('Check support'),
          ),
        ],
      ),
    );
  }

  Widget _urlField(ThemeVariables vars) {
    return FormField(
      label: 'URL',
      hint: 'Handed to the system handler as is.',
      child: Row(
        spacing: vars.spacing2,
        children: [
          Expanded(
            child: TextField(
              controller: _urlController,
              placeholder: 'https://example.com',
              mono: true,
              onSubmitted: (_) {
                if (!_isOpening && _isSupported) _openUrl();
              },
            ),
          ),
          Button(
            variant: ButtonVariant.filled,
            onPressed: _isOpening || !_isSupported ? null : _openUrl,
            child: const Text('Open URL'),
          ),
        ],
      ),
    );
  }

  Widget _presetChips(ThemeVariables vars) {
    final current = _urlController.text.trim();
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      spacing: vars.spacing15,
      children: [
        const SectionLabel('Presets'),
        Wrap(
          spacing: vars.spacing1,
          runSpacing: vars.spacing1,
          children: [
            for (final preset in _presets)
              OptionChip(
                label: preset,
                selected: current == preset,
                onTap: () => _urlController.text = preset,
              ),
          ],
        ),
      ],
    );
  }

  Widget _resultCard(ThemeVariables vars) {
    final result = _lastResult;
    final value = vars.mono.copyWith(color: vars.colorContent);
    Widget line(String label, String text) => Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        SizedBox(width: 96, child: Text(label, style: vars.muted)),
        Expanded(child: Text(text, style: value)),
      ],
    );

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      spacing: vars.spacing15,
      children: [
        Row(
          children: [
            const Expanded(child: SectionLabel('Last result')),
            if (result != null)
              Badge(
                size: WidgetSize.small,
                variant: BadgeVariant.tinted,
                tint: result.success ? BadgeTint.success : BadgeTint.danger,
                child: Text(result.success ? 'Opened' : 'Failed'),
              ),
          ],
        ),
        Card(
          variant: CardVariant.sunken,
          size: WidgetSize.small,
          child: Column(
            spacing: vars.spacing1,
            children: [
              line('Supported', _isSupported.toString()),
              line('Status', _status),
              if (result != null) ...[
                line('Success', result.success.toString()),
                line('Error code', result.errorCode.name),
                if (result.errorMessage?.isNotEmpty ?? false)
                  line('Error message', result.errorMessage ?? ''),
              ],
            ],
          ),
        ),
      ],
    );
  }
}
