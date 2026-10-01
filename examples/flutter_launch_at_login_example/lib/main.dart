import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show LaunchAtLogin, SizeToNative, WindowManager;

void main() {
  runApp(const LaunchAtLoginExampleApp());
}

class LaunchAtLoginExampleApp extends StatelessWidget {
  const LaunchAtLoginExampleApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const Host(
      title: 'LaunchAtLogin Example',
      home: LaunchAtLoginExamplePage(),
    );
  }
}

/// Header with the state badges, one settings column, the call log.
class LaunchAtLoginExamplePage extends StatefulWidget {
  const LaunchAtLoginExamplePage({super.key});

  @override
  State<LaunchAtLoginExamplePage> createState() =>
      _LaunchAtLoginExamplePageState();
}

class _LaunchAtLoginExamplePageState extends State<LaunchAtLoginExamplePage> {
  late final LaunchAtLogin _launchAtLogin;

  bool _isSupported = false;
  bool _isEnabled = false;
  bool _isBusy = false;
  String _status = 'Initializing...';

  // Config fields
  final _displayNameController = TextEditingController();
  final _executablePathController = TextEditingController();
  final _argumentsController = TextEditingController();

  // Current values (read from API)
  String _currentId = '';
  String _currentDisplayName = '';
  String _currentExecutablePath = '';
  String _currentArguments = '';

  // What the calls answered, newest first.
  String _lastEvent = 'No calls yet';
  final List<String> _log = [];

  @override
  void initState() {
    super.initState();
    _launchAtLogin = LaunchAtLogin.create()!;
    _refreshState();
    // A compact settings panel rather than the runner's default 800×600.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      WindowManager.instance.getCurrent()?.contentSize = const Size(
        460,
        640,
      ).toNative();
    });
  }

  @override
  void dispose() {
    _displayNameController.dispose();
    _executablePathController.dispose();
    _argumentsController.dispose();
    _launchAtLogin.dispose();
    super.dispose();
  }

  void _refreshState() {
    setState(() {
      _isSupported = LaunchAtLogin.isSupported();
      if (_isSupported) {
        _isEnabled = _launchAtLogin.isEnabled;
        _currentId = _launchAtLogin.id ?? '';
        _currentDisplayName = _launchAtLogin.displayName ?? '';
        _currentExecutablePath = _launchAtLogin.executablePath ?? '';
        _currentArguments = _launchAtLogin.arguments.join(' ');

        // Sync text fields with current values
        _displayNameController.text = _currentDisplayName;
        _executablePathController.text = _currentExecutablePath;
        _argumentsController.text = _currentArguments;

        final enabledStatus = _isEnabled ? 'enabled' : 'disabled';
        _status = 'Launch-at-login is $enabledStatus.';
      } else {
        _status = 'Launch-at-login is not supported on this platform.';
      }
    });
  }

  void _refreshPressed() {
    _refreshState();
    _report(_status, 'isEnabled → $_isEnabled');
  }

  Future<void> _setDisplayName() async {
    final name = _displayNameController.text.trim();
    if (name.isEmpty) {
      _report('Display name cannot be empty.');
      return;
    }
    setState(() => _isBusy = true);
    final success = _launchAtLogin.setDisplayName(name);
    setState(() => _isBusy = false);
    final call = 'setDisplayName("$name") → $success';
    if (success) {
      _report('Display name updated.', call);
      _refreshState();
    } else {
      _report('Failed to set display name.', call);
    }
  }

  Future<void> _setProgram() async {
    final path = _executablePathController.text.trim();
    if (path.isEmpty) {
      _report('Executable path cannot be empty.');
      return;
    }
    final args = _argumentsController.text.trim();
    final argsList = args.isNotEmpty ? args.split(RegExp(r'\s+')) : <String>[];
    setState(() => _isBusy = true);
    final success = _launchAtLogin.setProgram(path, argsList);
    setState(() => _isBusy = false);
    final call = 'setProgram("$path", $argsList) → $success';
    if (success) {
      _report('Program configured.', call);
      _refreshState();
    } else {
      _report('Failed to set program.', call);
    }
  }

  Future<void> _toggleEnabled(bool enable) async {
    setState(() => _isBusy = true);
    final success = enable ? _launchAtLogin.enable() : _launchAtLogin.disable();
    setState(() => _isBusy = false);
    final call = '${enable ? 'enable' : 'disable'}() → $success';
    if (success) {
      _report(
        enable ? 'Launch-at-login enabled.' : 'Launch-at-login disabled.',
        call,
      );
      _refreshState();
    } else {
      _report('Operation failed.', call);
    }
  }

  /// Puts [message] in the footer's headline and [call] (or the message) on
  /// top of its log.
  void _report(String message, [String? call]) {
    if (!mounted) return;
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
        Expanded(
          child: SingleChildScrollView(
            padding: EdgeInsets.all(vars.spacing4),
            child: Align(
              alignment: Alignment.topCenter,
              child: Preferences(
                children: [
                  PreferenceGroup(
                    title: 'LaunchAtLogin Example',
                    description:
                        'Register this app to start when the user logs in.',
                    action: _stateBadges(vars),
                    children: _isSupported ? _sections(vars) : [_unsupported()],
                  ),
                ],
              ),
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
                  _lastEvent = 'No calls yet';
                }),
        ),
      ],
    );
  }

  Widget _stateBadges(ThemeVariables vars) {
    Widget badge(String label, BadgeTint tint) => Badge(
      size: WidgetSize.small,
      variant: BadgeVariant.tinted,
      tint: tint,
      child: Text(label),
    );

    return Row(
      mainAxisSize: MainAxisSize.min,
      spacing: vars.spacing1,
      children: [
        badge(
          _isSupported ? 'Supported' : 'Not supported',
          _isSupported ? BadgeTint.success : BadgeTint.danger,
        ),
        if (_isSupported)
          badge(
            _isEnabled ? 'Enabled' : 'Disabled',
            _isEnabled ? BadgeTint.primary : BadgeTint.neutral,
          ),
        Tooltip(
          label: 'Refresh state',
          child: IconButton(
            icon: const Icon(FluentIcons.arrow_clockwise_20_regular),
            semanticsLabel: 'Refresh state',
            onPressed: _isBusy ? null : _refreshPressed,
          ),
        ),
      ],
    );
  }

  Widget _unsupported() {
    return Callout(
      tint: CalloutTint.warning,
      title: const Text('Not supported'),
      message: Text(_status),
    );
  }

  List<Widget> _sections(ThemeVariables vars) {
    final value = vars.mono.copyWith(color: vars.colorContent);
    Widget readBack(String label, String text) => Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        SizedBox(width: 96, child: Text(label, style: vars.muted)),
        Expanded(
          child: Text(
            text,
            maxLines: 3,
            overflow: TextOverflow.ellipsis,
            style: value,
          ),
        ),
      ],
    );

    return [
      PreferenceSection(
        label: 'Login item',
        children: [
          PreferenceRow(
            title: 'Launch at login',
            subtitle: _status,
            trailing: Switch(
              value: _isEnabled,
              onChanged: _isBusy ? null : _toggleEnabled,
            ),
          ),
        ],
      ),
      PreferenceSection(
        label: 'Registered as',
        children: [
          Card(
            variant: CardVariant.sunken,
            size: WidgetSize.small,
            child: Column(
              spacing: vars.spacing1,
              children: [
                readBack('ID', _currentId),
                readBack(
                  'Display name',
                  _currentDisplayName.isNotEmpty
                      ? _currentDisplayName
                      : '(not set)',
                ),
                readBack(
                  'Executable path',
                  _currentExecutablePath.isNotEmpty
                      ? _currentExecutablePath
                      : '(not set)',
                ),
                readBack(
                  'Arguments',
                  _currentArguments.isNotEmpty ? _currentArguments : '(none)',
                ),
              ],
            ),
          ),
        ],
      ),
      PreferenceSection(
        label: 'Display name',
        children: [
          Row(
            spacing: vars.spacing2,
            children: [
              Expanded(
                child: TextField(
                  controller: _displayNameController,
                  placeholder: 'My Application',
                  onSubmitted: (_) => _setDisplayName(),
                ),
              ),
              Button(
                variant: ButtonVariant.normal,
                tint: ButtonTint.neutral,
                onPressed: _isBusy ? null : _setDisplayName,
                child: const Text('Set display name'),
              ),
            ],
          ),
        ],
      ),
      PreferenceSection(
        label: 'Program',
        children: [
          FormField(
            label: 'Executable path',
            child: TextField(
              controller: _executablePathController,
              placeholder: '/usr/bin/myapp',
              mono: true,
            ),
          ),
          FormField(
            label: 'Arguments (space-separated)',
            child: TextField(
              controller: _argumentsController,
              placeholder: '--flag1 --flag2',
              mono: true,
            ),
          ),
          Align(
            alignment: Alignment.centerRight,
            child: Button(
              variant: ButtonVariant.filled,
              onPressed: _isBusy ? null : _setProgram,
              child: const Text('Set program'),
            ),
          ),
        ],
      ),
    ];
  }
}
