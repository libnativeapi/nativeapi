#!/usr/bin/env python3
"""Compile public entrypoints independently from the opt-in standalone runner.

Run after `flutter pub get` at the workspace root. Pass --standalone only on
Dart 3.13, whose experimental VM APIs the native standalone runner currently uses.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dart', default=shutil.which('dart'))
    parser.add_argument('--packages', type=Path)
    parser.add_argument('--web', action='store_true')
    parser.add_argument('--standalone', action='store_true')
    args = parser.parse_args()
    if not args.dart:
        parser.error('Provide --dart or put the Dart SDK on PATH')
    package = Path(__file__).resolve().parents[1]
    workspace = package.parents[2]
    config = (args.packages or workspace / '.dart_tool/package_config.json').resolve()
    if not config.is_file():
        parser.error('Run flutter pub get at the workspace root first')
    fixtures = package / 'tool/entrypoints'
    cases = [('kernel', 'nativeapi_import.dart')]
    if args.standalone:
        cases.append(('kernel', 'standalone_import.dart'))
    if args.web:
        cases += [('js', 'nativeapi_import.dart'), ('js', 'standalone_import.dart')]
    subprocess.run([args.dart, '--version'], check=True)
    with tempfile.TemporaryDirectory(prefix='nativeapi-entrypoints-') as temporary:
        for kind, source in cases:
            target = Path(temporary) / (Path(source).stem + ('.js' if kind == 'js' else '.dill'))
            subprocess.run([
                args.dart, 'compile', kind, '--packages=' + str(config),
                '-o', str(target), str(fixtures / source),
            ], check=True)
            print('PASS', kind, source, flush=True)


if __name__ == '__main__':
    main()
