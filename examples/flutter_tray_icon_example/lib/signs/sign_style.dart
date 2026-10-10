enum SignStyle {
  missing('Missing You Sign'),
  welcome('City Welcome Sign'),
  travel('Travel Sign'),
  guide('Scenic Guide Sign');

  const SignStyle(this.label);
  final String label;
  String get shortLabel => switch (this) {
    missing => 'Missing You',
    welcome => 'Welcome',
    travel => 'Travel',
    guide => 'Guide',
  };

  bool get hasDirection => switch (this) {
    missing || travel || guide => true,
    _ => false,
  };

  bool get hasSubtitleToggle => this == welcome || this == guide;

  List<String> get fields => switch (this) {
    missing => ['Place', 'Pinyin · Separate syllables with spaces'],
    welcome => ['City', 'English name'],
    guide => ['Attraction', 'English name', 'Distance · e.g. 2 km / 300 m'],
    travel => ['Upper text', 'Lower text'],
  };

  SignContent get defaults => switch (this) {
    missing => const SignContent('上海', 'SHANG HAI'),
    welcome => const SignContent('大理', 'DALI'),
    travel => const SignContent('下一站，山海', '去有风的地方'),
    guide => const SignContent('观景台', 'VIEWPOINT', '300 m'),
  };
}

final class SignContent {
  const SignContent(
    this.primary, [
    this.secondary = '',
    this.value = '',
    this.detail = '',
  ]);
  final String primary, secondary, value, detail;
  List<String> get values => [primary, secondary, value, detail];

  String heading(SignStyle style) => switch (style) {
    SignStyle.missing => '我在$primary很想你',
    SignStyle.welcome => '$primary欢迎您',
    _ => primary,
  };

  String? validate(SignStyle style) {
    final labels = style.fields;
    final entries = values;
    for (var i = 0; i < labels.length; i++) {
      final limit = i == 1 ? 36 : 16;
      if (entries[i].isEmpty ||
          entries[i].runes.length > limit ||
          entries[i].contains(RegExp(r'[\r\n\t]'))) {
        return '${labels[i].split(' · ').first} must contain 1–$limit characters and no line breaks or tabs.';
      }
    }
    switch (style) {
      case SignStyle.missing:
        if (primary.runes.length > 10) {
          return 'Place must contain at most 10 characters.';
        }
        if (secondary.length > 24 ||
            !RegExp(r'^[a-zA-Z0-9 -]+$').hasMatch(secondary)) {
          return 'Pinyin must contain 1–24 letters, digits, spaces or hyphens.';
        }
      default:
        break;
    }
    return null;
  }
}
