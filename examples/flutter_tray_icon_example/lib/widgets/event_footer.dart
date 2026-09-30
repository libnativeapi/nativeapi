import 'package:dazzui/dazzui.dart';

import '../tray_controller.dart';
import 'option_chip.dart';
import 'styles.dart';

/// The last event in large type (readable in a video) over a short log.
class EventFooter extends StatelessWidget {
  const EventFooter({super.key, required this.controller});

  final TrayController controller;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Container(
      height: 92,
      padding: EdgeInsets.fromLTRB(
        vars.spacing25,
        vars.spacing15,
        vars.spacing25,
        vars.spacing15,
      ),
      color: vars.colorSurfaceSunken,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Expanded(
                child: Text(
                  controller.lastEvent,
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: vars.titleMedium,
                ),
              ),
              ActionChip(label: 'Clear log', onTap: controller.clearLog),
            ],
          ),
          SizedBox(height: vars.spacing05),
          for (final line in controller.log.take(3))
            Text(
              line,
              maxLines: 1,
              overflow: TextOverflow.ellipsis,
              style: vars.mono,
            ),
        ],
      ),
    );
  }
}
