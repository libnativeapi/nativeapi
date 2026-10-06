// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

class Color {
  const Color({
    required this.r,
    required this.g,
    required this.b,
    required this.a,
  });

  final int r;
  final int g;
  final int b;
  final int a;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Color &&
          other.r == r &&
          other.g == g &&
          other.b == b &&
          other.a == a);

  @override
  int get hashCode => Object.hash(r, g, b, a);

  @override
  String toString() => 'Color(r: $r, g: $g, b: $b, a: $a)';
}
