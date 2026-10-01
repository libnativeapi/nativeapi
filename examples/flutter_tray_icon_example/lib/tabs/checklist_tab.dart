import 'package:dazzui_host/dazzui_host.dart';
import 'package:flutter/services.dart';

import '../checklist.dart';

/// Acceptance in one screen: what ticked itself, what still needs a look.
class ChecklistTab extends StatelessWidget {
  const ChecklistTab({super.key, required this.checklist});

  final Checklist checklist;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final auto = checklist.items.where((i) => !i.manual);
    final manual = checklist.items.where((i) => i.manual);

    Widget header(String text) => Padding(
      padding: EdgeInsets.fromLTRB(
        vars.spacing25,
        vars.spacing2,
        vars.spacing25,
        vars.spacing1,
      ),
      child: SectionLabel(text),
    );

    return Column(
      children: [
        Expanded(
          child: ListView(
            children: [
              header('Auto · ticks itself from events and return values'),
              for (final item in auto) _Row(item: item, checklist: checklist),
              Padding(
                padding: EdgeInsets.only(top: vars.spacing15),
                child: const Divider(),
              ),
              header('Manual · look at the tray, then mark it'),
              for (final item in manual) _Row(item: item, checklist: checklist),
              SizedBox(height: vars.spacing15),
            ],
          ),
        ),
        const Divider(),
        Padding(
          padding: EdgeInsets.symmetric(
            horizontal: vars.spacing25,
            vertical: vars.spacing15,
          ),
          child: Row(
            children: [
              Expanded(child: Text(checklist.summary, style: vars.mono)),
              ActionChip(
                label: 'Copy report',
                onTap: () =>
                    Clipboard.setData(ClipboardData(text: checklist.report())),
              ),
              SizedBox(width: vars.spacing1),
              ActionChip(label: 'Reset checklist', onTap: checklist.reset),
            ],
          ),
        ),
      ],
    );
  }
}

class _Row extends StatelessWidget {
  const _Row({required this.item, required this.checklist});

  final CheckItem item;
  final Checklist checklist;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    final detail = [
      if (item.note != null) item.note!,
      if (item.detail.isNotEmpty) item.detail,
    ].join(' · ');
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: vars.spacing25,
        vertical: vars.spacing05 + 1,
      ),
      child: Row(
        children: [
          SizedBox(
            width: 44,
            child: Badge(
              size: WidgetSize.small,
              tint: switch (item.status) {
                CheckStatus.pass => BadgeTint.success,
                CheckStatus.fail => BadgeTint.danger,
                CheckStatus.open => BadgeTint.neutral,
              },
              variant: item.status == CheckStatus.open
                  ? BadgeVariant.outlined
                  : BadgeVariant.tinted,
              child: Text(item.status.name),
            ),
          ),
          SizedBox(width: vars.spacing15),
          Expanded(
            child: Text(
              item.label,
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.bodySmall,
            ),
          ),
          if (detail.isNotEmpty)
            Padding(
              padding: EdgeInsets.only(left: vars.spacing15),
              child: Text(
                detail,
                style: vars.mono.copyWith(
                  color: item.status == CheckStatus.fail
                      ? vars.colorDanger.shade600
                      : vars.colorContentMuted,
                ),
              ),
            ),
          if (item.manual) ...[
            SizedBox(width: vars.spacing15),
            OptionChip(
              label: 'Pass',
              tint: ToggleTint.success,
              selected: item.status == CheckStatus.pass,
              onTap: () => checklist.mark(item.id, CheckStatus.pass),
            ),
            SizedBox(width: vars.spacing1),
            OptionChip(
              label: 'Fail',
              tint: ToggleTint.danger,
              selected: item.status == CheckStatus.fail,
              onTap: () => checklist.mark(item.id, CheckStatus.fail),
            ),
          ],
        ],
      ),
    );
  }
}
