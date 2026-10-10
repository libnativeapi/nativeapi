import 'package:dazzui_host/dazzui_host.dart';
import 'package:nativeapi_flutter/nativeapi_flutter.dart'
    show SizeToNative, WindowManager;

import 'icon_animations.dart';
import 'icon_animator.dart';
import 'signs/embedded_sign_preview.dart';
import 'tabs/animate_tab.dart';
import 'tabs/checklist_tab.dart';
import 'tabs/properties_tab.dart';
import 'tabs/sign_tab.dart';
import 'tray_controller.dart';
import 'widgets/live_preview.dart';
import 'widgets/add_tray_menu.dart';

void main() => runApp(const TrayIconExampleApp());

class TrayIconExampleApp extends StatelessWidget {
  const TrayIconExampleApp({super.key});
  @override
  Widget build(BuildContext context) =>
      const Host(title: 'Tray Icon', home: Shell());
}

enum _Tab { content, properties, checklist }

class Shell extends StatefulWidget {
  const Shell({super.key});
  @override
  State<Shell> createState() => _ShellState();
}

class _ShellState extends State<Shell> {
  final TrayController _controller = TrayController();
  _Tab _tab = _Tab.content;

  @override
  void initState() {
    super.initState();
    _controller.onContentRequested = () {
      if (mounted) setState(() => _tab = _Tab.content);
    };
    WidgetsBinding.instance.addPostFrameCallback((_) {
      final window = WindowManager.instance.getCurrent();
      window
        ?..title = 'Tray Icon'
        ..contentSize = const Size(800, 600).toNative()
        ..minimumSize = const Size(720, 560).toNative();
    });
  }

  @override
  void dispose() {
    _controller.onContentRequested = null;
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => ListenableBuilder(
    listenable: Listenable.merge([_controller, _controller.checklist]),
    builder: (context, _) {
      final vars = context.vars, selected = _controller.selected;
      return Stack(
        children: [
          // Widget animations keep their capture source painted even while a
          // different item or the native sign editor is selected.
          for (final entry in _controller.entries)
            if (entry.animator.animation == IconAnimation.widget)
              Positioned(
                left: 0,
                top: 0,
                child: RepaintBoundary(
                  key: entry.animator.captureKey,
                  child: CounterBadge(
                    time: entry.animator.time,
                    color: entry.animator.color,
                    size: kIconPoints,
                  ),
                ),
              ),
          ColoredBox(
            color: vars.colorCanvas,
            child: Row(
              children: [
                SizedBox(
                  width: 176,
                  child: ColoredBox(
                    color: vars.colorSurfaceColumn,
                    child: Column(
                      children: [
                        Expanded(
                          child: Sidebar(
                            width: 176,
                            header: Text('Tray Icon', style: vars.titleSmall),
                            children: [
                              for (final isSign in [false, true])
                                if (_controller.entries.any(
                                  (entry) => entry.isSign == isSign,
                                ))
                                  SidebarGroup(
                                    label: isSign ? 'Signs' : 'Icons',
                                    children: [
                                      for (final entry
                                          in _controller.entries.where(
                                            (entry) => entry.isSign == isSign,
                                          ))
                                        OptionCard(
                                          title: entry.label,
                                          description: entry.description,
                                          selected: entry == selected,
                                          semanticsLabel: entry.label,
                                          padding: EdgeInsets.all(
                                            vars.spacing2,
                                          ),
                                          onPressed: () =>
                                              _controller.select(entry),
                                        ),
                                    ],
                                  ),
                            ],
                          ),
                        ),
                        Padding(
                          padding: EdgeInsets.all(vars.spacing25),
                          child: AddTrayMenu(
                            controller: _controller,
                            onAdded: () => setState(() => _tab = _Tab.content),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.stretch,
                    children: [
                      SizedBox(
                        height: vars.frameTitlebarSize,
                        child: Padding(
                          padding: EdgeInsets.symmetric(
                            horizontal: vars.spacing4,
                          ),
                          child: Row(
                            children: [
                              Expanded(
                                child: Text(
                                  selected?.label ?? 'Tray items',
                                  style: vars.titleSmall,
                                ),
                              ),
                              if (selected != null) ...[
                                SegmentedControl<_Tab>(
                                  size: WidgetSize.small,
                                  items: [
                                    const SegmentedItem(
                                      value: _Tab.content,
                                      label: 'Content',
                                    ),
                                    const SegmentedItem(
                                      value: _Tab.properties,
                                      label: 'Properties',
                                    ),
                                    SegmentedItem(
                                      value: _Tab.checklist,
                                      label:
                                          'Checklist ${_controller.checklist.passed}/${_controller.checklist.items.length}',
                                    ),
                                  ],
                                  value: _tab,
                                  onChanged: (tab) =>
                                      setState(() => _tab = tab),
                                ),
                                SizedBox(width: vars.spacing2),
                                IconButton(
                                  size: WidgetSize.small,
                                  semanticsLabel:
                                      'Remove ${selected.label.toLowerCase()}',
                                  icon: const Icon(
                                    FluentIcons.delete_20_regular,
                                  ),
                                  onPressed: () =>
                                      _controller.removeIcon(selected),
                                ),
                              ],
                            ],
                          ),
                        ),
                      ),
                      const Divider(),
                      if (selected == null)
                        const Expanded(
                          child: Center(
                            child: Text(
                              'Use Add to create your first tray item.',
                            ),
                          ),
                        )
                      else ...[
                        if (selected.sign case final sign?)
                          ColoredBox(
                            color: vars.colorSurfaceSunken,
                            child: EmbeddedSignPreview(
                              key: ValueKey(sign.id),
                              controller: sign,
                            ),
                          )
                        else
                          LivePreview(controller: _controller),
                        const Divider(),
                        Expanded(
                          child: switch (_tab) {
                            _Tab.content =>
                              selected.sign != null
                                  ? SignTab(
                                      key: ValueKey(
                                        'sign-${selected.number}-${selected.sign!.style.name}',
                                      ),
                                      sign: selected.sign!,
                                    )
                                  : AnimateTab(controller: _controller),
                            _Tab.properties => PropertiesTab(
                              controller: _controller,
                              onEdit: _editText,
                            ),
                            _Tab.checklist => ChecklistTab(
                              checklist: _controller.checklist,
                            ),
                          },
                        ),
                      ],
                      const Divider(),
                      _footer(vars),
                    ],
                  ),
                ),
              ],
            ),
          ),
        ],
      );
    },
  );

  Widget _footer(ThemeVariables vars) => Padding(
    padding: EdgeInsets.symmetric(
      horizontal: vars.spacing4,
      vertical: vars.spacing2,
    ),
    child: Row(
      children: [
        Expanded(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                _controller.lastEvent,
                style: vars.labelSmall,
                maxLines: 1,
                overflow: TextOverflow.ellipsis,
              ),
              Text(
                _controller.log.firstOrNull ?? 'No calls yet',
                style: vars.mono,
                maxLines: 1,
                overflow: TextOverflow.ellipsis,
              ),
            ],
          ),
        ),
        Popover(
          side: PopoverSide.top,
          align: PopoverAlign.end,
          width: 360,
          title: const Text('Calls and events'),
          trigger: (context, state) => ActionChip(
            label: 'Log (${_controller.log.length})',
            onTap: state.toggle,
          ),
          child: SizedBox(
            height: 240,
            child: ListView(
              children: [
                for (final line in _controller.log)
                  Text(line, style: vars.mono),
              ],
            ),
          ),
        ),
        SizedBox(width: vars.spacing2),
        ActionChip(label: 'Clear', onTap: _controller.clearLog),
      ],
    ),
  );

  Future<void> _editText(
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
