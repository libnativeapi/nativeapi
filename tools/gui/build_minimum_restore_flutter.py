#!/usr/bin/env python3
"""Build #35 in a freshly generated Flutter app, without runner modifications."""
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
APP = ROOT / 'core/build/minimum-restore-flutter'

def main():
    flutter = shutil.which('flutter')
    assert flutter, 'Flutter must be on PATH'
    if not (APP / 'pubspec.yaml').exists():
        subprocess.run([flutter, 'create', '--empty', '--platforms=linux',
                        '--project-name=minimum_restore', str(APP)], check=True)
    pubspec = '''name: minimum_restore
environment:
  sdk: ^3.14.0
dependencies:
  flutter:
    sdk: flutter
  window_manager: 0.7.0
dependency_overrides:
'''
    for name in ('window_manager', 'nativeapi', 'nativeapi_flutter', 'cnativeapi'):
        pubspec += f'  {name}:\n    path: {json.dumps(str(ROOT / "bindings/dart" / name))}\n'
    (APP / 'pubspec.yaml').write_text(pubspec)
    shutil.copyfile(ROOT / 'tools/gui/window_minimum_restore_flutter.dart', APP / 'lib/main.dart')
    subprocess.run([flutter, 'pub', 'get'], cwd=APP, check=True)
    subprocess.run([flutter, 'build', 'linux', '--debug'], cwd=APP, check=True)
    print('BUNDLE', APP / 'build/linux/x64/debug/bundle', flush=True)

if __name__ == '__main__':
    main()
