import 'package:dazzui/dazzui.dart';
import 'package:flutter/cupertino.dart' show DefaultCupertinoLocalizations;

import 'omarchy_theme.dart';

/// The app an example is: `WidgetsApp` under a DazzUI [Theme].
///
/// The theme is [HostTheme]'s: Studio Light or Studio Dark by platform
/// brightness, or on Omarchy the desktop's own ([OmarchyTheme]). It sits above
/// the navigator so a dialog or a drawer is drawn in it too. A [ToastProvider]
/// sits there as well, and the viewport is under the navigator with [home],
/// so `Toaster.of(context).add(...)` works from any page.
///
/// The page is painted the theme's canvas colour unless [transparent] is set:
/// a window whose see-through parts are the demo (a visual effect, a shaped or
/// frameless window) leaves every pixel it does not paint to the window below.
///
/// The kit's text field is a cupertino one underneath and asks the cupertino
/// localizations for the word on its clear button, which is the one delegate
/// installed here.
///
/// [home] is the navigator's first route, so it is built once: a parent that
/// rebuilds with a different [home] does not replace the page. Pass state the
/// page must follow through a `Listenable` or an `InheritedWidget` above the
/// `Host` instead. A multi-window example puts one `Host` inside each window.
class Host extends StatelessWidget {
  const Host({
    super.key,
    required this.title,
    required this.home,
    this.transparent = false,
    this.debugShowCheckedModeBanner = false,
  });

  final String title;
  final Widget home;

  /// Paint no background under [home].
  final bool transparent;

  final bool debugShowCheckedModeBanner;

  @override
  Widget build(BuildContext context) {
    return WidgetsApp(
      title: title,
      color: transparent ? const Color(0x00000000) : themeVariables.colorCanvas,
      debugShowCheckedModeBanner: debugShowCheckedModeBanner,
      localizationsDelegates: const [DefaultCupertinoLocalizations.delegate],
      pageRouteBuilder: <T>(RouteSettings settings, WidgetBuilder builder) =>
          PageRouteBuilder<T>(
            settings: settings,
            // A transparent host's page must not be hidden by the route.
            opaque: !transparent,
            pageBuilder: (context, _, _) => builder(context),
          ),
      builder: (context, child) => HostTheme(
        child: ToastProvider(
          child: transparent
              ? child!
              : Builder(
                  builder: (context) =>
                      ColoredBox(color: context.vars.colorCanvas, child: child),
                ),
        ),
      ),
      home: Stack(children: [home, const ToastViewport()]),
    );
  }
}

/// The DazzUI theme an example is drawn in, over whatever app it is in.
///
/// Studio Light or Studio Dark by the platform's brightness; on Omarchy the
/// desktop's palette and rounding, followed live across `omarchy-theme-set`.
/// [Host] puts one over its navigator; an example that has to keep its own
/// `WidgetsApp` puts one in that app's `builder`.
class HostTheme extends StatefulWidget {
  const HostTheme({super.key, required this.child});

  final Widget child;

  @override
  State<HostTheme> createState() => _HostThemeState();
}

class _HostThemeState extends State<HostTheme> {
  static final ThemeData _light = ThemeData.studioLight();
  static final ThemeData _dark = ThemeData.studioDark();

  final OmarchyTheme? _omarchy = OmarchyTheme.start();

  @override
  void dispose() {
    _omarchy?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final omarchy = _omarchy;
    ThemeData system() =>
        MediaQuery.platformBrightnessOf(context) == Brightness.dark
        ? _dark
        : _light;
    if (omarchy == null) return Theme(data: system(), child: widget.child);
    return ListenableBuilder(
      listenable: omarchy,
      builder: (context, _) =>
          Theme(data: omarchy.data ?? system(), child: widget.child),
    );
  }
}
