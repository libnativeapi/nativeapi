// Compile this entrypoint on supported SDKs without opting into VM experiments.
import 'package:nativeapi/nativeapi.dart';

void main() {
  const size = Size(width: 800, height: 600);
  print('${WindowManager.instance}: ${size.width}x${size.height}');
}
