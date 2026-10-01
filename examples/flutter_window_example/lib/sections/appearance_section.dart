import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    hide Button, Menu, MenuItem, Preferences;

import 'pane.dart';

/// Title and title bar, shadow, opacity, visual effect and background.
class AppearanceSection extends StatefulWidget {
  const AppearanceSection({super.key, required this.pane});

  final Pane pane;

  // The values these presets send to the window, not colours of this UI.
  static const Color _indigo = Color(0xFF3F51B5);
  static const Color _white = Color(0xFFFFFFFF);

  static const _backgrounds = <String, Color>{
    'White': _white,
    'Light Grey': Color(0xFFEEEEEE),
    'Dark': Color(0xFF212121),
    'Blue': Color(0xFF90CAF9),
    'Transparent': Color(0x00000000),
  };

  static const _titles = ['Hello Window', 'My App', 'nativeapi'];

  @override
  State<AppearanceSection> createState() => _AppearanceSectionState();
}

class _AppearanceSectionState extends State<AppearanceSection> {
  /// The slider's value while it is dragged, so the toast comes once, on
  /// release, rather than once per step.
  double? _opacity;

  @override
  Widget build(BuildContext context) {
    final window = widget.pane.window;
    final feedback = widget.pane.feedback;
    final vars = context.vars;
    final title = window.title ?? '';
    final opacity = _opacity ?? window.opacity;
    final background = window.backgroundColor.toColor().toARGB32();
    final backgroundName = AppearanceSection._backgrounds.entries
        .where((e) => e.value.toARGB32() == background)
        .map((e) => e.key)
        .firstOrNull;
    String effectLabel(VisualEffect e) =>
        e.name[0].toUpperCase() + e.name.substring(1);

    return PaneBody(
      children: [
        PreferenceSection(
          label: 'Title bar',
          children: [
            PreferenceRow(
              title: 'Title',
              subtitle: title.isEmpty ? 'Untitled' : '"$title"',
              trailing: ChipRun(
                children: [
                  for (final t in AppearanceSection._titles)
                    OptionChip(
                      label: t,
                      selected: title == t,
                      onTap: () {
                        window.title = t;
                        feedback('Title set to "$t"');
                      },
                    ),
                ],
              ),
            ),
            SwitchRow(
              title: 'Title bar hidden',
              value: window.titleBarStyle == TitleBarStyle.hidden,
              onChanged: (hidden) {
                window.titleBarStyle = hidden
                    ? TitleBarStyle.hidden
                    : TitleBarStyle.normal;
                feedback(
                  'Title bar ${window.titleBarStyle == TitleBarStyle.hidden ? 'hidden' : 'shown'}',
                );
              },
            ),
            SwitchRow(
              title: 'Content under the title bar',
              value: window.isContentUnderTitleBar,
              onChanged: (v) {
                final applied = window.setContentUnderTitleBar(v);
                feedback(
                  applied
                      ? 'Content ${window.isContentUnderTitleBar ? 'extends into' : 'stops at'} the title bar'
                      : 'Extending into the title bar is not available here',
                  ok: applied,
                );
              },
            ),
            PreferenceRow(
              title: 'Title bar colors',
              subtitle: 'Windows, WinUI 3 only',
              trailing: ChipRun(
                children: [
                  ActionChip(
                    label: 'Blue',
                    onTap: () {
                      final ok = window.setTitleBarColors(
                        AppearanceSection._indigo.toNative(),
                        AppearanceSection._white.toNative(),
                      );
                      feedback(
                        ok ? 'Title bar colors applied' : 'Unsupported: requires Windows WinUI 3 and a live window',
                        ok: ok,
                      );
                    },
                  ),
                  ActionChip(
                    label: 'Reset',
                    onTap: () {
                      final ok = window.resetTitleBarColors();
                      feedback(
                        ok
                            ? 'Title bar colors reset'
                            : 'Title bar reset unsupported or failed',
                        ok: ok,
                      );
                    },
                  ),
                ],
              ),
            ),
          ],
        ),
        PreferenceSection(
          label: 'Surface',
          children: [
            SwitchRow(
              title: 'Shadow',
              value: window.hasShadow,
              onChanged: (v) {
                window.hasShadow = v;
                feedback('Shadow ${window.hasShadow ? 'enabled' : 'disabled'}');
              },
            ),
            PreferenceRow(
              title: 'Opacity',
              trailing: SizedBox(
                width: 200,
                child: Slider(
                  values: [opacity],
                  min: 0.1,
                  max: 1.0,
                  step: 0.05,
                  valueLabel: opacity.toStringAsFixed(2),
                  semanticsLabel: 'Opacity',
                  onChanged: (vs) {
                    window.opacity = vs.first;
                    setState(() => _opacity = vs.first);
                  },
                  onChangeEnd: (vs) {
                    setState(() => _opacity = null);
                    feedback('Opacity: ${vs.first.toStringAsFixed(2)}');
                  },
                ),
              ),
            ),
            PreferenceRow(
              title: 'Visual effect',
              trailing: SizedBox(
                width: 140,
                child: Select<VisualEffect>(
                  size: WidgetSize.small,
                  options: [
                    for (final e in VisualEffect.values)
                      SelectOption(value: e, label: effectLabel(e)),
                  ],
                  value: window.visualEffect,
                  onChanged: (e) {
                    if (e == null) return;
                    final label = effectLabel(e);
                    final applied = window.setVisualEffect(e);
                    feedback(
                      applied
                          ? 'Visual effect: $label'
                          : 'Visual effect $label is not available here',
                      ok: applied,
                    );
                  },
                ),
              ),
            ),
            PreferenceRow(
              title: 'Background color',
              subtitle: backgroundName ?? 'Custom',
              trailing: ChipRun(
                children: [
                  for (final MapEntry(key: name, value: color)
                      in AppearanceSection._backgrounds.entries)
                    Tooltip(
                      label: name,
                      child: IntrinsicWidth(
                        child: Toggle(
                          variant: ToggleVariant.normal,
                          tint: ToggleTint.primary,
                          pressed: background == color.toARGB32(),
                          semanticsLabel: name,
                          onPressedChanged: (_) {
                            window.backgroundColor = color.toNative();
                            feedback('Background: $name');
                          },
                          child: Container(
                            width: vars.iconSmall,
                            height: vars.iconSmall,
                            decoration: BoxDecoration(
                              color: color,
                              borderRadius: BorderRadius.circular(
                                vars.radiusTiny,
                              ),
                              border: Border.all(
                                color: vars.colorBorderStrong,
                                width: context.hairlineWidth,
                              ),
                            ),
                          ),
                        ),
                      ),
                    ),
                ],
              ),
            ),
          ],
        ),
      ],
    );
  }
}
