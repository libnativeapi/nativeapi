/// The plumbing every Flutter example in this repository shares, so an
/// example is only the part that is about nativeapi.
///
/// [Host] is the app: a `WidgetsApp` (no material) under a DazzUI [Theme]
/// that follows the platform's light or dark appearance, or on Omarchy the
/// desktop's own palette and rounding ([OmarchyTheme]), with toasts and the
/// localizations the kit's text field needs. The rest are the example-only
/// widgets: [OptionChip] and [ActionChip] for the mouse-only settings the
/// GUI tests click, [OptionRow] to lay them out, [EventFooter] for the log
/// every example keeps, and two text roles ([ExampleStyles]). Icons are
/// Fluent's ([FluentIcons]), the set dazzui draws its own glyphs from.
library;

export 'package:dazzui/dazzui.dart';
export 'package:fluentui_system_icons/fluentui_system_icons.dart'
    show FluentIcons;

export 'src/chips.dart';
export 'src/event_footer.dart';
export 'src/host.dart';
export 'src/omarchy_theme.dart';
export 'src/styles.dart';
