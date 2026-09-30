import 'package:dazzui/dazzui.dart';

import 'styles.dart';

/// A small mouse-only choice: every setting in the example is one of these,
/// so a GUI test or a demo script never needs the keyboard. Labels are plain
/// text (no icon font), which also lets a UI probe find them by name.
///
/// It is a DazzUI [Toggle]: paper at rest, the tint's wash when it is the
/// current value.
class OptionChip extends StatelessWidget {
  const OptionChip({
    super.key,
    required this.label,
    this.selected = false,
    this.tint = ToggleTint.primary,
    this.onTap,
  });

  final String label;
  final bool selected;
  final ToggleTint tint;

  /// Null disables the chip.
  final VoidCallback? onTap;

  @override
  Widget build(BuildContext context) {
    // A Toggle is an aligned box with no width of its own, so in a Wrap it
    // would take the whole line; this keeps it the width of its label.
    return IntrinsicWidth(
      child: Toggle(
        size: WidgetSize.tiny,
        variant: ToggleVariant.normal,
        tint: tint,
        pressed: selected,
        enabled: onTap != null,
        // A chip is picked, never unpicked: pressing the current value again
        // just sets it again.
        onPressedChanged: onTap == null ? null : (_) => onTap!(),
        child: Text(label),
      ),
    );
  }
}

/// A chip that does something rather than picks something — the same box as
/// [OptionChip], as a DazzUI [Button], so a row of both reads as one row.
class ActionChip extends StatelessWidget {
  const ActionChip({
    super.key,
    required this.label,
    this.primary = false,
    this.onTap,
  });

  final String label;

  /// The one action a dialog or a row leads with.
  final bool primary;

  /// Null disables the chip.
  final VoidCallback? onTap;

  @override
  Widget build(BuildContext context) {
    return Button(
      size: WidgetSize.tiny,
      variant: primary ? ButtonVariant.filled : ButtonVariant.normal,
      tint: primary ? ButtonTint.primary : ButtonTint.neutral,
      onPressed: onTap,
      child: Text(label),
    );
  }
}

/// One labelled row of chips, closed by a hairline.
class OptionRow extends StatelessWidget {
  const OptionRow({super.key, required this.label, required this.children});

  final String label;
  final List<Widget> children;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
      children: [
        Padding(
          padding: EdgeInsets.symmetric(
            horizontal: vars.spacing25,
            vertical: vars.spacing15,
          ),
          child: Row(
            children: [
              SizedBox(width: 66, child: Text(label, style: vars.muted)),
              Expanded(
                child: Wrap(
                  spacing: vars.spacing1,
                  runSpacing: vars.spacing1,
                  crossAxisAlignment: WrapCrossAlignment.center,
                  children: children,
                ),
              ),
            ],
          ),
        ),
        const Divider(),
      ],
    );
  }
}

/// Muted remark placed after the chips of a row.
class Hint extends StatelessWidget {
  const Hint(this.text, {super.key});

  final String text;

  @override
  Widget build(BuildContext context) => Text(text, style: context.vars.muted);
}
