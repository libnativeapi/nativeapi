#!/usr/bin/env python3
"""Build and run the window_manager migration regression; sends no input.

The app changes its own size, maximize/full-screen state and close requests.
Use --executable to run a previously built fixture, including on remote hosts.
"""

import argparse
import os
import platform
import shutil
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path)
    parser.add_argument('--flutter', default=shutil.which('flutter'))
    args = parser.parse_args()
    executable = args.executable
    if executable is None:
        workspace = Path(__file__).resolve().parents[2]
        package = Path(os.environ.get('WINDOW_MANAGER_DIR',
                                     workspace / 'bindings/dart/window_manager'))
        project = package / 'example'
        target = project / 'lib/native_state_test.dart'
        if target.exists():
            parser.error(f'fixture target already exists: {target}')
        system = {'Darwin': 'macos', 'Windows': 'windows', 'Linux': 'linux'}[platform.system()]
        if not args.flutter:
            parser.error('Flutter was not found; specify --flutter')
        shutil.copy2(Path(__file__).with_name('window_manager_state_flutter.dart'), target)
        try:
            subprocess.run([args.flutter, 'build', system, '--debug', '-t',
                            'lib/native_state_test.dart'], cwd=project, check=True, timeout=900)
        finally:
            target.unlink()
        name = 'window_manager_example'
        executable = project / {
            'macos': f'build/macos/Build/Products/Debug/{name}.app/Contents/MacOS/{name}',
            'windows': f'build/windows/x64/runner/Debug/{name}.exe',
            'linux': f'build/linux/x64/debug/bundle/{name}',
        }[system]
    executable = executable.resolve()
    result = subprocess.run([str(executable)], cwd=executable.parent,
                            text=True, capture_output=True, timeout=90)
    print(result.stdout, end='')
    print(result.stderr, end='')
    if result.returncode != 0 or 'PASS window_manager native state regression' not in result.stdout:
        raise SystemExit(result.returncode or 1)


if __name__ == '__main__':
    main()
