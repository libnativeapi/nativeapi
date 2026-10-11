#!/usr/bin/env python3
"""Run the real Flutter clipboard scenario, preserving the macOS pasteboard.

No synthetic input. The core test wrapper restores every original native format
when the child exits, including after an assertion failure or child timeout.
"""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
EXAMPLE = ROOT / 'bindings/dart/clipboard_watcher/example'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', action='store_true')
    args = parser.parse_args()
    if sys.platform != 'darwin':
        parser.error('This pasteboard-preserving wrapper requires macOS.')
    if args.build:
        subprocess.run(
            ['flutter', 'build', 'macos', '--debug', '-t', 'lib/clipboard_smoke.dart'],
            cwd=EXAMPLE, check=True,
        )
    wrapper = ROOT / 'core/build/tests/clipboard_macos_test'
    if not wrapper.is_file():
        parser.error('Build core with its clipboard_macos_test target first.')
    candidates = list((EXAMPLE / 'build/macos/Build/Products/Debug').glob('*.app/Contents/MacOS/*'))
    binaries = [p for p in candidates if p.is_file() and p.suffix not in ('.dylib', '.so')]
    if len(binaries) != 1:
        parser.error(f'Expected one example executable, got {binaries}. Use --build.')
    # Timeout the child, never the restoration wrapper. Python forwards the app
    # output and kills only the tested child if it stalls.
    child = '''import subprocess, sys
try:
    result = subprocess.run([sys.argv[1]], timeout=45, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True)
    print(result.stdout, flush=True)
    sys.exit(0 if result.returncode == 0 and
             'CLIPBOARD_WATCHER_SMOKE_PASSED' in result.stdout else 1)
except subprocess.TimeoutExpired as error:
    print(error.stdout or b'', flush=True)
    print('FAIL clipboard scenario timed out', flush=True)
    sys.exit(1)
'''
    result = subprocess.run([str(wrapper), sys.executable, '-c', child, str(binaries[0])])
    return result.returncode


if __name__ == '__main__':
    sys.exit(main())
