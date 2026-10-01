import 'package:dazzui_host/dazzui_host.dart';

/// What kind of line a log entry is, drawn as the dot in front of it. The
/// colour comes from the theme's ramp of the same name.
enum LogTone { neutral, primary, info, success, warning }

extension LogToneColor on LogTone {
  Color color(ThemeVariables vars) => switch (this) {
    LogTone.neutral => vars.colorNeutral[500]!,
    LogTone.primary => vars.colorPrimary[500]!,
    LogTone.info => vars.colorInfo[500]!,
    LogTone.success => vars.colorSuccess[500]!,
    LogTone.warning => vars.colorWarning[500]!,
  };
}

class LogEntry {
  LogEntry(this.message, {this.tone = LogTone.neutral, this.replaceTag});

  final DateTime timestamp = DateTime.now();
  final String message;
  final LogTone tone;

  /// See [EventLog.add].
  final String? replaceTag;

  String get formattedTime {
    final m = timestamp.minute.toString().padLeft(2, '0');
    final s = timestamp.second.toString().padLeft(2, '0');
    final ms = timestamp.millisecond.toString().padLeft(3, '0');
    return '$m:$s.$ms';
  }
}

/// The event log, newest first, capped at [maxEntries].
class EventLog {
  static const int maxEntries = 200;

  final List<LogEntry> entries = [];

  bool get isEmpty => entries.isEmpty;
  int get length => entries.length;

  /// [replaceTag] keeps a stream of events (a window being dragged or resized)
  /// on one line: an entry replaces the newest one when it carries the same tag.
  void add(
    String message, {
    LogTone tone = LogTone.neutral,
    String? replaceTag,
  }) {
    if (replaceTag != null &&
        entries.isNotEmpty &&
        entries.first.replaceTag == replaceTag) {
      entries.removeAt(0);
    }
    entries.insert(0, LogEntry(message, tone: tone, replaceTag: replaceTag));
    if (entries.length > maxEntries) entries.removeLast();
  }

  void clear() => entries.clear();
}
