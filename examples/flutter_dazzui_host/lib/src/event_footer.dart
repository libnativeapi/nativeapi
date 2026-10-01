import 'package:dazzui/dazzui.dart';

import 'chips.dart';
import 'styles.dart';

/// The log band at the bottom of an example: the latest event in large type
/// (readable in a video) over the last few lines, and a Clear button.
class EventFooter extends StatelessWidget {
  const EventFooter({
    super.key,
    required this.headline,
    required this.lines,
    this.onClear,
    this.visibleLines = 3,
    this.height = 92,
  });

  /// The last event, or a resting message such as "No events yet".
  final String headline;

  /// Newest first; only the first [visibleLines] are shown.
  final Iterable<String> lines;

  final VoidCallback? onClear;
  final int visibleLines;
  final double height;

  @override
  Widget build(BuildContext context) {
    final vars = context.vars;
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        const Divider(),
        Container(
          height: height,
          padding: EdgeInsets.symmetric(
            horizontal: vars.spacing25,
            vertical: vars.spacing15,
          ),
          color: vars.colorSurfaceSunken,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  Expanded(
                    child: Text(
                      headline,
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: vars.titleMedium,
                    ),
                  ),
                  if (onClear != null)
                    ActionChip(label: 'Clear log', onTap: onClear),
                ],
              ),
              SizedBox(height: vars.spacing05),
              for (final line in lines.take(visibleLines))
                Text(
                  line,
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: vars.mono,
                ),
            ],
          ),
        ),
      ],
    );
  }
}
