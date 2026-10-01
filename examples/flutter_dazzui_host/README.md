# dazzui_host

What every Flutter example in this repository shares, so each example is only the part
that is about nativeapi. It is a workspace package, not an example of its own, and it is
not published.

```dart
import 'package:dazzui_host/dazzui_host.dart'; // DazzUI, FluentIcons, flutter/widgets

void main() => runApp(const Host(title: 'Window example', home: MyPage()));
```

| | |
| --- | --- |
| `Host` | the app: `WidgetsApp` (no material) under a [DazzUI](https://pub.dev/packages/dazzui) theme, toasts, and the localizations the kit's text field needs. A multi-window example puts one inside each window |
| `OmarchyTheme` | the theme on [Omarchy](https://omarchy.org): the desktop's palette (`~/.local/state/omarchy/current/theme/colors.toml`, resolved through `omarchy-theme-color`) and Hyprland's `decoration:rounding`, followed live across `omarchy-theme-set`. Elsewhere the theme is Studio Light or Studio Dark by platform brightness |
| `OptionChip`, `ActionChip`, `OptionRow`, `Hint` | the mouse-only settings the GUI tests click by their labels: a picked value (a `Toggle`), an action (a `Button`), a labelled row of them |
| `EventFooter` | the log band at the bottom of an example |
| `ExampleStyles` | `vars.muted` and `vars.mono`, the two text roles the examples add |

Everything else an example draws is a DazzUI component; the only custom drawing left in
the examples is the subject of each demo (a window's shape, a title bar, a drop target).
To try the Omarchy mapping without switching the desktop, point `OMARCHY_STATE_DIR` at a
copy of `~/.local/state/omarchy/current`.
