import 'package:dazzui/dazzui.dart';

/// The two text roles the example adds to the design system's own: a muted
/// caption for row labels and hints, and a monospace line for read-back
/// values and the log.
extension ExampleStyles on ThemeVariables {
  TextStyle get muted => labelSmall.copyWith(color: colorContentMuted);

  TextStyle get mono =>
      captionSmall.inFace(fontCode).copyWith(color: colorContentMuted);
}
