// Runs on the UI isolate. Wrap with core/build/tests/clipboard_macos_test on macOS.
import 'dart:async';

import 'package:nativeapi/nativeapi.dart';

void check(bool value) {
  if (!value) throw StateError('clipboard assertion failed');
}

void main() => runNativeApp(appMain);
void appMain() {
  unawaited(run());
}

Future<void> run() async {
  final board = Clipboard.instance;
  check(!board.writeText('a\u0000b'));
  check(!board.writeHtml('\uD800'));
  final image = Image.fromBase64(
    'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAC0lEQVR42mNgAAIAAAUAAen63NgAAAAASUVORK5CYII=',
  )!;
  check(
    board.write(
      ClipboardData(
        text: '剪贴板 😀',
        html: '<b>中文</b>',
        image: image,
        filePaths: ['/tmp/file', '/tmp/directory'],
      ),
    ),
  );
  final data = await board.readAsync();
  check(
    data.text == '剪贴板 😀' &&
        data.html == '<b>中文</b>' &&
        data.filePaths.length == 2 &&
        data.image != null,
  );
  final returnedImage = await board.readImageAsync();
  check(board.clear());
  check(data.image!.size.width == 1 && returnedImage!.size.width == 1);
  check(await board.readTextAsync() == null);
  check(board.writeText(''));
  check(await board.readTextAsync() == '');
  check(await board.readHtmlAsync() == null);
  print('Dart clipboard roundtrip passed');
  Application.instance.quit(0);
}
