import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart' show Window;

import '../event_log.dart';

/// Shows the outcome of an action as a toast; [ok] false for "not available
/// here" and other refusals.
typedef Notify = void Function(String message, {bool ok});

/// Adds a line to the event log.
typedef LogLine = void Function(String message, {LogTone tone});

/// What a section about the selected window gets from the shell: the window,
/// and the two ways to report back. Every setter in a section is followed by a
/// [feedback], which also rebuilds the shell, so the controls always show what
/// the native getters return afterwards.
class Pane {
  const Pane({required this.window, required this.feedback, required this.log});

  final Window window;
  final Notify feedback;
  final LogLine log;
}

/// A section's page: a scrolling settings column.
class PaneBody extends StatelessWidget {
  const PaneBody({super.key, required this.children});

  final List<Widget> children;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return SingleChildScrollView(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing5,
        vertical: vars.spacing4,
      ),
      child: Align(
        alignment: Alignment.topCenter,
        child: Preferences(children: children),
      ),
    );
  }
}

/// A row's trailing run of small buttons.
class ChipRun extends StatelessWidget {
  const ChipRun({super.key, required this.children});

  final List<Widget> children;

  @override
  Widget build(BuildContext context) => Row(
    mainAxisSize: MainAxisSize.min,
    spacing: context.vars.spacing1,
    children: children,
  );
}

/// A settings row with a switch on the right.
class SwitchRow extends StatelessWidget {
  const SwitchRow({
    super.key,
    required this.title,
    this.subtitle,
    required this.value,
    required this.onChanged,
  });

  final String title;
  final String? subtitle;
  final bool value;
  final ValueChanged<bool> onChanged;

  @override
  Widget build(BuildContext context) => PreferenceRow(
    title: title,
    subtitle: subtitle,
    trailing: Switch(value: value, onChanged: onChanged),
  );
}

/// "w × h" for a native size.
String sizeText(double width, double height) =>
    '${width.round()} × ${height.round()}';
