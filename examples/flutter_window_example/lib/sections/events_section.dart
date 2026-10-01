import 'package:dazzui_host/dazzui_host.dart';

import '../event_log.dart';

/// The whole event log, newest first, with the time of each line.
///
/// A GUI test reads it: each message is its own text, starting with
/// `Window #<id>` for the WindowManager events.
class EventsSection extends StatelessWidget {
  const EventsSection({super.key, required this.log, required this.onClear});

  final EventLog log;
  final VoidCallback onClear;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    if (log.isEmpty) {
      return const Center(
        child: EmptyState(
          title: 'No events yet.\nInteract with windows to see events appear.',
        ),
      );
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        Padding(
          padding: EdgeInsets.fromLTRB(
            vars.spacing5,
            vars.spacing3,
            vars.spacing5,
            vars.spacing2,
          ),
          child: Row(
            children: [
              Expanded(child: SectionLabel('Event Log (${log.length})')),
              ActionChip(label: 'Clear', onTap: onClear),
            ],
          ),
        ),
        Expanded(
          child: SingleChildScrollView(
            padding: EdgeInsets.fromLTRB(
              vars.spacing5,
              0,
              vars.spacing5,
              vars.spacing4,
            ),
            child: Table(
              children: [
                const TableHead(
                  children: [
                    TableCell(head: true, width: 96, child: Text('Time')),
                    TableCell(head: true, child: Text('Event')),
                  ],
                ),
                for (final entry in log.entries)
                  TableRow(
                    children: [
                      TableCell(
                        width: 96,
                        child: Text(entry.formattedTime, style: vars.mono),
                      ),
                      TableCell(
                        child: Row(
                          spacing: vars.spacing2,
                          children: [
                            Container(
                              width: vars.spacing2,
                              height: vars.spacing2,
                              decoration: BoxDecoration(
                                color: entry.tone.color(vars),
                                shape: BoxShape.circle,
                              ),
                            ),
                            Expanded(
                              child: Text(
                                entry.message,
                                maxLines: 1,
                                overflow: TextOverflow.ellipsis,
                                style: vars.mono.copyWith(
                                  color: vars.colorContent,
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
              ],
            ),
          ),
        ),
      ],
    );
  }
}
