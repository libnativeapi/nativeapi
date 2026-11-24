import 'package:code_assets/code_assets.dart';
import 'package:hooks/hooks.dart';
import 'package:test/test.dart';

import '../hook/build.dart' as hook;

/// Runs the build hook for [targetOS], with [skipPlatforms] as the app's
/// `skip_platforms`, and returns the native libraries it built.
Future<List<CodeAsset>> build(OS targetOS, {Object? skipPlatforms}) async {
  late List<CodeAsset> libraries;
  await testCodeBuildHook(
    mainMethod: hook.main,
    targetOS: targetOS,
    userDefines: skipPlatforms == null
        ? null
        : PackageUserDefines(
            workspacePubspec: PackageUserDefinesSource(
              defines: {'skip_platforms': skipPlatforms},
              basePath: Uri.file('/'),
            ),
          ),
    check: (input, output) => libraries = output.assets.code.toList(),
  );
  return libraries;
}

void main() {
  group('skip_platforms', () {
    for (final os in [OS.android, OS.iOS, OS.linux, OS.macOS, OS.windows]) {
      test('builds nothing for ${os.name} when it lists it', () async {
        expect(await build(os, skipPlatforms: [os.name]), isEmpty);
      });
    }

    test('skips each of the platforms it lists', () async {
      final skip = ['android', 'ios'];
      expect(await build(OS.android, skipPlatforms: skip), isEmpty);
      expect(await build(OS.iOS, skipPlatforms: skip), isEmpty);
    });

    // Fuchsia has no sources, so the hook fails once it gets to building: a
    // way to see that it wasn't skipped, without compiling core.
    for (final skip in <Object?>[
      null,
      <String>[],
      ['android', 'ios'],
    ]) {
      test('goes on to build a platform $skip does not list', () async {
        await expectLater(
          build(OS.fuchsia, skipPlatforms: skip),
          throwsUnsupportedError,
        );
      });
    }

    for (final invalid in <Object>[
      'android',
      true,
      ['beos'],
      ['android', 'fuchsia'],
      ['iOS'],
      [1],
    ]) {
      test('rejects $invalid, naming the setting', () async {
        await expectLater(
          build(OS.android, skipPlatforms: invalid),
          throwsA(
            isFormatException.having(
              (e) => e.message,
              'message',
              contains('hooks.user_defines.cnativeapi.skip_platforms'),
            ),
          ),
        );
      });
    }
  });
}
