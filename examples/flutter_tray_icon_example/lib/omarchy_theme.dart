import 'dart:async';
import 'dart:io';

import 'package:dazzui/dazzui.dart';

/// The desktop's theme on [Omarchy](https://omarchy.org), as a DazzUI
/// [ThemeData] that follows `omarchy-theme-set`.
///
/// Omarchy keeps the active theme staged in `~/.local/state/omarchy/current/`:
/// `theme.name` and `theme/colors.toml`, a semantic palette (`background`,
/// `foreground`, `accent`, `muted`, the ANSI hues, and a `mode`). Every
/// Omarchy consumer resolves that file through `omarchy-theme-color`, whose
/// alias and fallback cascade this reuses rather than restates. A theme switch
/// replaces the `theme` directory and rewrites `theme.name`, which the watcher
/// on the state directory sees; listeners then get the next [data].
///
/// Shape follows the desktop as well as colour: Omarchy's shell mirrors
/// Hyprland's `decoration:rounding` for every corner it draws (square when it
/// is 0), so that one number becomes every radius token here.
///
/// [start] returns null anywhere else, and the app falls back to the design
/// system's own light or dark theme.
class OmarchyTheme extends ChangeNotifier {
  OmarchyTheme._(this._stateDir);

  final Directory _stateDir;
  StreamSubscription<FileSystemEvent>? _watch;
  Timer? _debounce;
  Map<String, String> _palette = const {};
  int? _rounding;

  /// The theme as last read, or null before the first read succeeded.
  ThemeData? data;

  /// The theme's name (`theme.name`), for the log.
  String? name;

  /// Null when this is not an Omarchy desktop. `OMARCHY_STATE_DIR` overrides
  /// the state directory, which is how a second palette can be tried without
  /// switching the desktop's theme.
  static OmarchyTheme? start() {
    if (!Platform.isLinux) return null;
    final override = Platform.environment['OMARCHY_STATE_DIR'];
    final home = Platform.environment['HOME'] ?? '';
    final dir = Directory(override ?? '$home/.local/state/omarchy/current');
    if (!File('${dir.path}/theme/colors.toml').existsSync()) return null;
    final theme = OmarchyTheme._(dir);
    theme._reload();
    theme._watch = dir.watch().listen((_) {
      theme._debounce?.cancel();
      theme._debounce = Timer(const Duration(milliseconds: 300), theme._reload);
    });
    return theme;
  }

  @override
  void dispose() {
    _watch?.cancel();
    _debounce?.cancel();
    super.dispose();
  }

  Future<void> _reload() async {
    final palette = await _resolve();
    if (palette == null || palette.isEmpty) return;
    String? label;
    try {
      label = File('${_stateDir.path}/theme.name').readAsStringSync().trim();
    } on IOException {
      label = null;
    }
    final rounding = await _hyprlandRounding();
    if (_mapEquals(palette, _palette) &&
        label == name &&
        rounding == _rounding) {
      return;
    }
    _palette = palette;
    _rounding = rounding;
    name = label;
    data = _themeFrom(palette, rounding);
    notifyListeners();
  }

  /// Hyprland's `decoration:rounding`, or null when it cannot be asked (not
  /// under Hyprland, or its socket is not in the environment).
  static Future<int?> _hyprlandRounding() async {
    try {
      final result = await Process.run('hyprctl', [
        '-j',
        'getoption',
        'decoration:rounding',
      ]);
      if (result.exitCode != 0) return null;
      final match = RegExp(r'"int"\s*:\s*(\d+)')
          .firstMatch(result.stdout as String);
      return match == null ? null : int.parse(match.group(1)!);
    } on ProcessException {
      return null;
    }
  }

  /// The resolved palette: `key\tvalue` lines from `omarchy-theme-color --all`,
  /// or the raw file when the tool is not on the path.
  Future<Map<String, String>?> _resolve() async {
    final file = '${_stateDir.path}/theme/colors.toml';
    try {
      final result = await Process.run('omarchy-theme-color', [
        '--file',
        file,
        '--all',
      ]);
      if (result.exitCode == 0) {
        return {
          for (final line in (result.stdout as String).split('\n'))
            if (line.contains('\t'))
              line.substring(0, line.indexOf('\t')): line
                  .substring(line.indexOf('\t') + 1)
                  .trim(),
        };
      }
    } on ProcessException {
      // Not on the path: read the file itself, below.
    }
    try {
      final raw = <String, String>{};
      for (final line in File(file).readAsLinesSync()) {
        final eq = line.indexOf('=');
        if (eq < 0 || line.trimLeft().startsWith('#')) continue;
        final key = line.substring(0, eq).trim();
        var value = line.substring(eq + 1).trim();
        final quote = value.indexOf(RegExp('["\']'));
        if (quote >= 0) {
          value = value.substring(quote + 1);
          value = value.substring(0, value.indexOf(RegExp('["\']')));
        }
        raw[key] = value;
      }
      // The few fallbacks the mapping below needs.
      raw['background'] ??= raw['bg'] ?? raw['color0'] ?? '';
      raw['foreground'] ??= raw['fg'] ?? raw['color7'] ?? '';
      raw['accent'] ??= raw['blue'] ?? raw['color4'] ?? raw['foreground']!;
      raw['muted'] ??= raw['color8'] ?? raw['dark_foreground'] ?? '';
      return raw;
    } on IOException {
      return null;
    }
  }

  static bool _mapEquals(Map<String, String> a, Map<String, String> b) {
    if (a.length != b.length) return false;
    for (final MapEntry(:key, :value) in a.entries) {
      if (b[key] != value) return false;
    }
    return true;
  }
}

/// The palette mapped onto the design system's roles.
///
/// The system's own dark theme is the base for a dark palette and its light
/// theme for a light one, so the shade each control recipe picks — filled at
/// 600 in the light, 500 in the dark, and so on — stays what the system
/// decided; only the colours those shades are taken from change.
ThemeData _themeFrom(Map<String, String> p, int? rounding) {
  Color? hex(String key) {
    final value = p[key];
    if (value == null || !RegExp(r'^#[0-9a-fA-F]{6}$').hasMatch(value)) {
      return null;
    }
    return Color(0xFF000000 | int.parse(value.substring(1), radix: 16));
  }

  final background = hex('background') ?? const Color(0xFF1E1E2E);
  final foreground = hex('foreground') ?? const Color(0xFFCDD6F4);
  final dark = switch (p['mode']) {
    'light' => false,
    'dark' => true,
    _ => background.computeLuminance() < 0.4,
  };
  final accent = hex('accent') ?? hex('blue') ?? foreground;
  final muted = hex('muted') ?? _mix(foreground, background, 0.5);
  final lighterBackground =
      hex('lighter_background') ??
      _mix(background, dark ? Colors.white : Colors.black, 0.08);
  final darkBackground =
      hex('dark_background') ?? _mix(background, Colors.black, 0.25);
  final selection = hex('selection') ?? _mix(background, foreground, 0.2);

  // Two directions off the canvas: "lighter" (paper — white in the light,
  // the theme's lighter background in the dark) and "towards the ink" (what
  // a sunken band or an inset is in either mode).
  Color lighter(double t) =>
      _mix(background, dark ? lighterBackground : Colors.white, t);
  Color inked(double t) =>
      _mix(background, dark ? lighterBackground : darkBackground, t);
  Color ink(double t) => _mix(foreground, muted, t);

  final base = dark ? themeVariablesStudioDark : themeVariables;
  final baseShade = dark ? 500 : 600;
  final onAccent = accent.computeLuminance() > 0.4
      ? (dark ? background : foreground)
      : Colors.white;

  // The desktop's one corner, at every step: the shell draws its cards,
  // popups and controls all at Hyprland's rounding, and so does this.
  final r = rounding?.toDouble();
  final vars = r == null
      ? base
      : base.copyWith(
          radiusTiny: r,
          radiusSmall: r,
          radiusMedium: r,
          radiusLarge: r,
          radiusBig: r * 1.5,
          controlFieldRadius: r,
          controlContainerRadius: r,
          framePopoverRadius: r,
          frameWindowRadius: r,
          checkboxRadius: r < 5 ? r : 5,
        );

  return ThemeData(
    brightness: dark ? Brightness.dark : Brightness.light,
    vars: vars.copyWith(
      colorPrimary: _ramp(accent, baseShade),
      colorNeutral: _ramp(_mix(background, foreground, 0.55), baseShade),
      colorInfo: _ramp(hex('blue') ?? hex('cyan') ?? accent, baseShade),
      colorSuccess: _ramp(hex('green') ?? accent, baseShade),
      colorWarning: _ramp(hex('orange') ?? hex('yellow') ?? accent, baseShade),
      colorDanger: _ramp(hex('red') ?? accent, baseShade),
      colorCanvas: background,
      colorSurface: lighter(dark ? 0.3 : 0.6),
      colorSurfaceMuted: lighter(0.15),
      colorSurfaceSunken: inked(1),
      colorSurfaceSubtle: foreground.withValues(alpha: 0.07),
      colorSurfaceInset: inked(0.5),
      colorSurfaceRaised: dark ? selection : Colors.white,
      colorSurfaceOverlay: lighter(dark ? 0.15 : 0.7),
      colorSurfaceChrome: lighter(dark ? 0.15 : 0.7),
      colorSurfaceColumn: dark ? darkBackground : lighter(0.3),
      colorContent: foreground,
      colorContentSecondary: hex('light_foreground') ?? ink(0.2),
      colorContentNav: ink(0.35),
      colorContentMuted: ink(0.45),
      colorContentSubtle: ink(0.6),
      colorContentFaint: ink(0.8),
      colorBorder: foreground.withValues(alpha: 0.08),
      colorBorderStrong: foreground.withValues(alpha: 0.12),
      colorBorderMuted: _mix(background, foreground, 0.3),
      colorOnAccent: onAccent,
      progressGradientFrom: accent,
      progressGradientTo: _mix(accent, Colors.white, 0.35),
    ),
  );
}

Color _mix(Color a, Color b, double t) => Color.lerp(a, b, t)!;

/// An eleven-step ramp with [base] at [baseShade]: whiter above it, blacker
/// below, the way the system's own ramps run.
ColorSwatch<int> _ramp(Color base, int baseShade) {
  const shades = [50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 950];
  return ColorSwatch<int>(base.toARGB32(), {
    for (final shade in shades)
      shade: shade == baseShade
          ? base
          : shade < baseShade
          ? _mix(
              base,
              Colors.white,
              0.9 * (baseShade - shade) / (baseShade - 50),
            )
          : _mix(
              base,
              Colors.black,
              0.7 * (shade - baseShade) / (950 - baseShade),
            ),
  });
}
