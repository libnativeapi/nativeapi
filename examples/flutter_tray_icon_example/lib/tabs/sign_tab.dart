import 'package:dazzui_host/dazzui_host.dart';

import '../signs/sign_art.dart';
import '../signs/sign_controller.dart';
import '../signs/sign_style.dart';

class SignTab extends StatefulWidget {
  const SignTab({super.key, required this.sign});
  final SignController sign;
  @override
  State<SignTab> createState() => _SignTabState();
}

class _SignTabState extends State<SignTab> {
  late final _editors = [
    for (final value in widget.sign.content.values)
      TextEditingController(text: value),
  ];
  String? _error;
  void _load() {
    for (var i = 0; i < _editors.length; i++) {
      _editors[i].text = widget.sign.content.values[i];
    }
    setState(() => _error = null);
  }

  void _apply() => setState(
    () => _error = widget.sign.setContent(
      _editors[0].text,
      _editors[1].text,
      _editors[2].text,
      _editors[3].text,
    ),
  );
  @override
  void dispose() {
    for (final editor in _editors) {
      editor.dispose();
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final sign = widget.sign, vars = context.vars;
    return ListView(
      padding: EdgeInsets.all(vars.spacing4),
      children: [
        const SectionLabel('Sign style'),
        SizedBox(height: vars.spacing2),
        Row(
          children: [
            for (final style in SignStyle.values) ...[
              if (style != SignStyle.values.first)
                SizedBox(width: vars.spacing2),
              Expanded(
                child: Tooltip(
                  label: style.label,
                  child: OptionCard(
                    title: style.shortLabel,
                    selected: sign.style == style,
                    padding: EdgeInsets.all(vars.spacing2),
                    onPressed: () {
                      sign.selectStyle(style.index);
                      _load();
                    },
                    titleContent: Column(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        SignArt(style: style),
                        SizedBox(height: vars.spacing1),
                        Text(
                          style.shortLabel,
                          style: vars.captionSmall,
                          maxLines: 1,
                        ),
                      ],
                    ),
                  ),
                ),
              ),
            ],
          ],
        ),
        SizedBox(height: vars.spacing3),
        const SectionLabel('Sign content'),
        SizedBox(height: vars.spacing2),
        for (var i = 0; i < sign.fields.length; i++) ...[
          Text(sign.fields[i], style: vars.labelSmall),
          SizedBox(height: vars.spacing1),
          TextField(controller: _editors[i], onSubmitted: (_) => _apply()),
          SizedBox(height: vars.spacing3),
        ],
        if (_error != null) ...[
          Text(_error!, style: vars.muted),
          SizedBox(height: vars.spacing2),
        ],
        Row(
          children: [
            ActionChip(label: 'Apply sign', primary: true, onTap: _apply),
            SizedBox(width: vars.spacing2),
            ActionChip(
              label: 'Reset example',
              onTap: () {
                sign.restoreDefaults();
                _load();
              },
            ),
          ],
        ),
        SizedBox(height: vars.spacing4),
        const SectionLabel('Display options'),
        if (sign.style == SignStyle.missing)
          OptionRow(
            label: 'Sign color',
            children: [
              OptionChip(
                label: 'Blue',
                selected: !sign.green,
                onTap: () => sign.setGreen(false),
              ),
              OptionChip(
                label: 'Green',
                selected: sign.green,
                onTap: () => sign.setGreen(true),
              ),
            ],
          ),
        if (sign.style.hasDirection)
          OptionRow(
            label: sign.style == SignStyle.missing
                ? 'Direction'
                : 'Upper arrow',
            children: [
              OptionChip(
                label: sign.style == SignStyle.missing ? 'W → E' : 'Left',
                selected: !sign.right,
                onTap: () => sign.setRight(false),
              ),
              OptionChip(
                label: sign.style == SignStyle.missing ? 'E → W' : 'Right',
                selected: sign.right,
                onTap: () => sign.setRight(true),
              ),
            ],
          ),
        if (sign.style.hasSubtitleToggle)
          OptionRow(
            label: 'English',
            children: [
              Switch(value: sign.english, onChanged: sign.setEnglish),
              const Hint('Show English subtitle'),
            ],
          ),
        SizedBox(height: vars.spacing3),
        Align(
          alignment: Alignment.centerLeft,
          child: ActionChip(label: 'Enlarge preview', onTap: sign.showPreview),
        ),
        if (sign.error != null) Text(sign.error!, style: vars.muted),
      ],
    );
  }
}
