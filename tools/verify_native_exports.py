#!/usr/bin/env python3
"""Check that a stripped cnativeapi library still exports the complete C API.

Build with `dart run bindings/dart/nativeapi/example/nativeapi_example.dart`,
then run this script (or pass --library to inspect a particular build).
"""
import argparse
import ctypes
import gzip
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path)
    parser.add_argument('--allow-internal', action='store_true', help='Measure an unhidden baseline build')
    args = parser.parse_args()
    path = args.library
    if path is None:
        candidates = [p for p in (ROOT / '.dart_tool/hooks_runner/shared/cnativeapi/build').glob('*/*')
                      if p.name in ('libcnativeapi.so', 'libcnativeapi.dylib', 'cnativeapi.dll')]
        if not candidates:
            parser.error('Build cnativeapi first, or supply --library')
        path = max(candidates, key=lambda p: p.stat().st_mtime)
    path = path.resolve()
    expected = set()
    pattern = r'\bFFI_PLUGIN_EXPORT\s+[^;{}#]*?\b(native_\w+)\s*\('
    for header in (ROOT / 'core/src/capi').glob('*.h'):
        expected.update(re.findall(pattern, header.read_text()))
    shim = (ROOT / 'bindings/dart/cnativeapi/src/event_loop.h').read_text()
    expected.update(re.findall(r'\bCNATIVEAPI_EXPORT\s+[^;{}#]*?\b(cnativeapi_\w+)\s*\(', shim))
    expected.update(('nativeapi_protocol_handler_host_attach', 'nativeapi_protocol_handler_host_deliver'))
    assert {'native_window_create', 'native_handle_finalize'} <= expected, 'C API headers were not parsed'
    library = ctypes.CDLL(str(path))
    missing = sorted(name for name in expected if not hasattr(library, name))
    assert not missing, f'Missing public exports: {missing}'
    exported = None
    internal = None
    if sys.platform == 'darwin' or sys.platform.startswith('linux'):
        command = (['nm', '-gU', str(path)] if sys.platform == 'darwin' else
                   ['nm', '-D', '--defined-only', str(path)])
        symbols = subprocess.check_output(command, text=True).splitlines()
        names = [line.split()[-1] for line in symbols if line.split()]
        internal = sum(name.startswith(('_Z', '__Z')) for name in names)
        assert args.allow_internal or internal == 0, 'Internal C++ symbols are exported'
        exported = len(names)
    data = path.read_bytes()
    print(json.dumps({'library': str(path), 'bytes': len(data),
                      'gzipBytes': len(gzip.compress(data, mtime=0)),
                      'requiredExports': len(expected), 'exportedSymbols': exported,
                      'internalCppSymbols': internal,
                      'missingExports': missing}, indent=2))


if __name__ == '__main__':
    main()
