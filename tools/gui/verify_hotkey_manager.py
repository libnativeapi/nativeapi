#!/usr/bin/env python3
"""Verify the hotkey_manager submodule against this workspace's nativeapi.

Run through fastforge host exec from the workspace root. The default runs
analysis, package tests and a debug example build. --smoke runs the built app
on the current desktop without sending mouse or keyboard input.
"""

import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import select
import shutil
import subprocess
import sys
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'bindings/dart/hotkey_manager/packages/hotkey_manager'


def run(command, cwd=PACKAGE, env=None):
    print('RUN', subprocess.list2cmdline([str(item) for item in command]), flush=True)
    subprocess.run(command, cwd=cwd, env=env, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--smoke', action='store_true')
    parser.add_argument('--desktop', action='store_true', help='Run package tests and the built smoke app on a desktop')
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--isolated-x11', action='store_true', help='Use a private rootful XWayland display on Linux')
    args = parser.parse_args()
    os.environ.setdefault('CI', 'true')
    if platform.system() == 'Linux':
        # SSH lacks the logged-on desktop's X11 authentication and Wayland bus.
        desktop = subprocess.run(['systemctl', '--user', 'show-environment'],
                                 capture_output=True, text=True, check=True).stdout
        keys = {'DISPLAY', 'XAUTHORITY', 'WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR', 'DBUS_SESSION_BUS_ADDRESS'}
        for line in desktop.splitlines():
            key, separator, value = line.partition('=')
            if separator and key in keys:
                os.environ.setdefault(key, value)
    if args.isolated_x11:
        if platform.system() != 'Linux':
            parser.error('--isolated-x11 is Linux only')
        # A separate server avoids changing the user's XWayland sockets or
        # intercepting any shortcut registered by another desktop application.
        output = PACKAGE / 'build/hotkey-probe'
        output.mkdir(parents=True, exist_ok=True)
        read_fd, write_fd = os.pipe()
        with (output / 'xwayland.log').open('wb') as log:
            server = subprocess.Popen(['Xwayland', ':99', '-displayfd', str(write_fd),
                                       '-geometry', '1280x800', '-ac', '-nolisten', 'tcp', '-noreset'],
                                      pass_fds=(write_fd,), stdout=log, stderr=log)
        os.close(write_fd)
        try:
            if not select.select([read_fd], [], [], 20)[0]:
                raise RuntimeError(f'Private XWayland did not start: {output / "xwayland.log"}')
            number = os.read(read_fd, 32).decode().strip()
            assert number.isdigit(), (output / 'xwayland.log').read_text()
            env = os.environ.copy()
            env.update(DISPLAY=':' + number, GDK_BACKEND='x11')
            run([sys.executable, __file__, *[arg for arg in sys.argv[1:] if arg != '--isolated-x11']], cwd=ROOT, env=env)
        finally:
            os.close(read_fd)
            server.terminate()
            try:
                server.wait(timeout=5)
            except subprocess.TimeoutExpired:
                server.kill()
                server.wait(timeout=5)
        return
    flutter = shutil.which('flutter')
    if args.desktop:
        if not flutter:
            raise SystemExit('Flutter is not on PATH')
        os.environ['HOTKEY_MANAGER_NATIVE_TESTS'] = '1'
        run([flutter, 'test', '--no-pub', '--reporter', 'expanded'])
    if args.smoke or args.desktop:
        # Keep the probe's pure-Python dependency local to the ignored build tree.
        env = os.environ.copy()
        if importlib.util.find_spec('websocket') is None:
            with urllib.request.urlopen('https://pypi.org/pypi/websocket-client/1.9.0/json', timeout=30) as response:
                metadata = json.load(response)
            wheel = next(item for item in metadata['urls'] if item['filename'].endswith('py3-none-any.whl'))
            path = PACKAGE / 'build/hotkey-probe' / wheel['filename']
            path.parent.mkdir(parents=True, exist_ok=True)
            if not path.exists():
                with urllib.request.urlopen(wheel['url'], timeout=30) as response:
                    path.write_bytes(response.read())
            assert hashlib.sha256(path.read_bytes()).hexdigest() == wheel['digests']['sha256']
            env['PYTHONPATH'] = str(path) + os.pathsep + env.get('PYTHONPATH', '')
        run([sys.executable, str(ROOT / 'tools/gui/flutter_hotkey_manager_smoke.py')], cwd=ROOT, env=env)
        return

    overrides = 'dependency_overrides:\n' + ''.join(
        f'  {name}:\n    path: {json.dumps(str(ROOT / "bindings/dart" / name))}\n'
        for name in ('nativeapi', 'cnativeapi')
    )
    for directory in (PACKAGE, PACKAGE / 'example'):
        (directory / 'pubspec_overrides.yaml').write_text(overrides)
    if not flutter:
        raise SystemExit('Flutter is not on PATH')
    print(f'Platform: {platform.platform()}; DISPLAY={os.environ.get("DISPLAY", "")}', flush=True)
    run([flutter, 'pub', 'get'])
    if not args.build_only:
        run([flutter, 'analyze', '--no-pub'])
        run([flutter, 'test', '--no-pub', '--reporter', 'expanded'])
    system = {'Darwin': 'macos', 'Windows': 'windows', 'Linux': 'linux'}[platform.system()]
    run([flutter, 'build', system, '--debug', '--no-pub'], cwd=PACKAGE / 'example')
    print('PASS hotkey_manager example build' if args.build_only else
          'PASS hotkey_manager analysis, tests and example build', flush=True)


if __name__ == '__main__':
    main()
