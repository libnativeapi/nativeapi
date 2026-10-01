import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show SizeToNative, WindowManager;

import 'tabs/animate_tab.dart';
import 'tabs/checklist_tab.dart';
import 'tabs/properties_tab.dart';
import 'tray_controller.dart';
import 'widgets/live_preview.dart';

// The example is drawn with DazzUI over the shared host (dazzui_host: the
// WidgetsApp, the theme, the chips), so everything on screen is a few dozen
// lines you can read here.
//
//   tray_controller.dart   every TrayIcon / TrayManager call, scenes, events
//   icon_animator.dart     canvas or widget → PNG → TrayIcon.icon, per frame
//   icon_animations.dart   what the frames look like
//   context_menu.dart      the tray menu
//   checklist.dart         the acceptance checklist

void main() {
  runApp(const TrayIconExampleApp());
}

class TrayIconExampleApp extends StatelessWidget {
  const TrayIconExampleApp({super.key});

  @override
  Widget build(BuildContext context) =>
      const Host(title: 'Tray icon example', home: Shell());
}

enum _Tab { animate, properties, checklist }

/// Icons strip, live preview, three tabs, event footer.
class Shell extends StatefulWidget {
  const Shell({super.key});

  @override
  State<Shell> createState() => _ShellState();
}

class _ShellState extends State<Shell> {
  final TrayController _controller = TrayController();
  _Tab _tab = _Tab.animate;

  @override
  void initState() {
    super.initState();
    // The layout is made for exactly this content size, and the runners' default
    // size does not always give it: on Windows it includes the title bar.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      WindowManager.instance.getCurrent()?.contentSize = const Size(
        400,
        640,
      ).toNative();
    });
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return ListenableBuilder(
      listenable: Listenable.merge([_controller, _controller.checklist]),
      builder: (context, _) => ColoredBox(
        color: vars.colorCanvas,
        child: Column(
          children: [
            _iconsStrip(vars),
            const Divider(),
            LivePreview(controller: _controller),
            const Divider(),
            _tabBar(vars),
            const Divider(),
            Expanded(
              child: switch (_tab) {
                _Tab.animate => AnimateTab(controller: _controller),
                _Tab.properties => PropertiesTab(
                  controller: _controller,
                  onEdit: (title, initial, onSubmit) =>
                      _editText(context, title, initial, onSubmit),
                ),
                _Tab.checklist => ChecklistTab(
                  checklist: _controller.checklist,
                ),
              },
            ),
            EventFooter(
              headline: _controller.lastEvent,
              lines: _controller.log,
              onClear: _controller.clearLog,
            ),
          ],
        ),
      ),
    );
  }

  Widget _iconsStrip(ThemeVariables vars) {
    final selected = _controller.selected;
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing25,
        vertical: vars.spacing15,
      ),
      child: Row(
        children: [
          Expanded(
            child: Wrap(
              spacing: vars.spacing1,
              runSpacing: vars.spacing1,
              crossAxisAlignment: WrapCrossAlignment.center,
              children: [
                const SectionLabel('Tray icons'),
                SizedBox(width: vars.spacing05),
                for (final entry in _controller.entries)
                  OptionChip(
                    label: '#${entry.number}',
                    selected: entry == selected,
                    onTap: () => _controller.select(entry),
                  ),
                ActionChip(label: 'Add icon', onTap: _controller.addIcon),
              ],
            ),
          ),
          ActionChip(
            label: selected == null ? 'Remove' : 'Remove #${selected.number}',
            onTap: selected == null
                ? null
                : () => _controller.removeIcon(selected),
          ),
        ],
      ),
    );
  }

  Widget _tabBar(ThemeVariables vars) {
    final checklist = _controller.checklist;
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing25,
        vertical: vars.spacing15,
      ),
      child: SegmentedControl<_Tab>(
        stretch: true,
        // A failed item turns the whole bar: the tab is where to look next.
        tint: checklist.failed > 0
            ? SegmentedTint.danger
            : SegmentedTint.primary,
        items: [
          const SegmentedItem(value: _Tab.animate, label: 'Animate'),
          const SegmentedItem(value: _Tab.properties, label: 'Properties'),
          SegmentedItem(
            value: _Tab.checklist,
            label: 'Checklist ${checklist.passed}/${checklist.items.length}',
          ),
        ],
        value: _tab,
        onChanged: (tab) => setState(() => _tab = tab),
      ),
    );
  }

  /// A one-field dialog, for values the preset chips don't cover. Scripts
  /// never need it: every preset is a click.
  Future<void> _editText(
    BuildContext context,
    String title,
    String initial,
    ValueChanged<String> onSubmit,
  ) async {
    final value = await showDialog<String>(
      context: context,
      builder: (_) => _TextEditDialog(title: title, initial: initial),
    );
    if (value != null) onSubmit(value);
  }
}

class _TextEditDialog extends StatefulWidget {
  const _TextEditDialog({required this.title, required this.initial});

  final String title;
  final String initial;

  @override
  State<_TextEditDialog> createState() => _TextEditDialogState();
}

class _TextEditDialogState extends State<_TextEditDialog> {
  late final TextEditingController _text = TextEditingController(
    text: widget.initial,
  );

  @override
  void initState() {
    super.initState();
    _text.selection = TextSelection(
      baseOffset: 0,
      extentOffset: _text.text.length,
    );
  }

  @override
  void dispose() {
    _text.dispose();
    super.dispose();
  }

  void _apply() => Navigator.of(context).pop(_text.text);

  @override
  Widget build(BuildContext context) {
    return Dialog(
      children: [
        DialogHeader(title: widget.title),
        DialogBody(
          children: [
            TextField(
              controller: _text,
              autofocus: true,
              onSubmitted: (_) => _apply(),
            ),
          ],
        ),
        DialogFooter(
          children: [
            Button(
              variant: ButtonVariant.normal,
              tint: ButtonTint.neutral,
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('Cancel'),
            ),
            Button(
              variant: ButtonVariant.filled,
              onPressed: _apply,
              child: const Text('Apply'),
            ),
          ],
        ),
      ],
    );
  }
}
