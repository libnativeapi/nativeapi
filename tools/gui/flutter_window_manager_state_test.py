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
from contextlib import ExitStack
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path)
    parser.add_argument('--flutter', default=shutil.which('flutter'))
    parser.add_argument('--local-nativeapi', action='store_true',
                        help='Temporarily use the nativeapi packages in this workspace')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--release', action='store_true', help='Build and verify a release fixture')
    args = parser.parse_args()
    executable = args.executable
    if executable is None:
        workspace = Path(__file__).resolve().parents[2]
        package = Path(os.environ.get('WINDOW_MANAGER_DIR',
                                     workspace / 'bindings/dart/window_manager'))
        project = package / 'example'
        target = project / 'build/native_state_test.dart'
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            parser.error(f'fixture target already exists: {target}')
        system = {'Darwin': 'macos', 'Windows': 'windows', 'Linux': 'linux'}[platform.system()]
        if not args.flutter:
            parser.error('Flutter was not found; specify --flutter')
        configuration = 'Release' if args.release else 'Debug'
        with ExitStack() as cleanup:
            if args.local_nativeapi:
                content = 'dependency_overrides:\n' + ''.join(
                    f'  {name}:\n    path: {workspace / "bindings/dart" / name}\n'
                    for name in ('nativeapi', 'nativeapi_flutter', 'cnativeapi'))
                for directory in (package, project):
                    override = directory / 'pubspec_overrides.yaml'
                    original = override.read_bytes() if override.exists() else None
                    def restore(path=override, data=original):
                        if data is None:
                            path.unlink(missing_ok=True)
                        else:
                            path.write_bytes(data)
                    cleanup.callback(restore)
                    override.write_text(content)
            shutil.copy2(Path(__file__).with_name('window_manager_state_flutter.dart'), target)
            cleanup.callback(target.unlink, missing_ok=True)
            subprocess.run([args.flutter, 'build', system, '--release' if args.release else '--debug', '-t',
                            'build/native_state_test.dart'], cwd=project, check=True, timeout=900)
        name = 'window_manager_example'
        executable = project / {
            'macos': f'build/macos/Build/Products/{configuration}/{name}.app/Contents/MacOS/{name}',
            'windows': f'build/windows/x64/runner/{configuration}/{name}.exe',
            'linux': f'build/linux/x64/{configuration.lower()}/bundle/{name}',
        }[system]
    executable = executable.resolve()
    if args.build_only:
        print(executable)
        return
    result = subprocess.run([str(executable)], cwd=executable.parent,
                            text=True, capture_output=True, timeout=90)
    print(result.stdout, end='')
    print(result.stderr, end='')
    if result.returncode != 0 or 'PASS window_manager native state regression' not in result.stdout:
        raise SystemExit(result.returncode or 1)


if __name__ == '__main__':
    main()
