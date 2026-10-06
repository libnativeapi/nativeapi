// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

// ignore_for_file: unused_import, unnecessary_import

class Point {
  const Point({required this.x, required this.y});

  final double x;
  final double y;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Point && other.x == x && other.y == y);

  @override
  int get hashCode => Object.hash(x, y);

  @override
  String toString() => 'Point(x: $x, y: $y)';
}

class Size {
  const Size({required this.width, required this.height});

  final double width;
  final double height;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Size && other.width == width && other.height == height);

  @override
  int get hashCode => Object.hash(width, height);

  @override
  String toString() => 'Size(width: $width, height: $height)';
}

class Rectangle {
  const Rectangle({
    required this.x,
    required this.y,
    required this.width,
    required this.height,
  });

  final double x;
  final double y;
  final double width;
  final double height;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Rectangle &&
          other.x == x &&
          other.y == y &&
          other.width == width &&
          other.height == height);

  @override
  int get hashCode => Object.hash(x, y, width, height);

  @override
  String toString() =>
      'Rectangle(x: $x, y: $y, width: $width, height: $height)';
}

class EdgeInsets {
  const EdgeInsets({
    required this.top,
    required this.right,
    required this.bottom,
    required this.left,
  });

  final double top;
  final double right;
  final double bottom;
  final double left;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is EdgeInsets &&
          other.top == top &&
          other.right == right &&
          other.bottom == bottom &&
          other.left == left);

  @override
  int get hashCode => Object.hash(top, right, bottom, left);

  @override
  String toString() =>
      'EdgeInsets(top: $top, right: $right, bottom: $bottom, left: $left)';
}
