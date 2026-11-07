#!/usr/bin/env python3
"""Run the Linux report fixtures in a private KWin Wayland desktop, without input.

Requires kwin_wayland and its dependencies. An optional environment JSON supports
rootless extracted packages; the caller supplies their library/plugin/data paths.
The logged-on desktop and user settings are never used.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kwin', default=shutil.which('kwin_wayland'))
    parser.add_argument('--environment-json', type=Path)
    parser.add_argument('--flutter', action='store_true', help='Run the built FlView fixture')
    args = parser.parse_args()
    assert args.kwin, 'Install KWin or provide --kwin'
    env = os.environ.copy()
    if args.environment_json:
        env.update(json.loads(args.environment_json.read_text()))
    for key in ('DISPLAY', 'WAYLAND_DISPLAY', 'XAUTHORITY', 'DBUS_SESSION_BUS_ADDRESS'):
        env.pop(key, None)
    output = ROOT / 'core/build/window-manager-kwin' / str(time.time_ns())
    output.mkdir(parents=True)
    # D-Bus-activated GVfs can briefly retain a mount in the private runtime
    # after KWin exits. Do not let that mask the actual suite result.
    with tempfile.TemporaryDirectory(prefix='nativeapi-kwin-', ignore_cleanup_errors=True) as scratch:
        for key in ('XDG_RUNTIME_DIR', 'XDG_CONFIG_HOME', 'XDG_CACHE_HOME', 'XDG_DATA_HOME'):
            folder = Path(scratch) / key
            folder.mkdir(mode=0o700)
            env[key] = str(folder)
        env.update(WAYLAND_DISPLAY='nativeapi-test-kwin', GUI_TEST_PRIVATE_WAYLAND='1',
                   XDG_CURRENT_DESKTOP='KDE', XDG_SESSION_TYPE='wayland', GDK_BACKEND='wayland',
                   KWIN_COMPOSE='O2', LIBGL_ALWAYS_SOFTWARE='1', GSETTINGS_BACKEND='keyfile')
        test = 'window_manager_linux_flutter_test.py' if args.flutter else 'window_manager_linux_reports.py'
        # KWin receives one executable for --exit-with-session. This generated
        # entry point uses Python repr for paths, avoiding shell interpolation.
        session = Path(scratch) / 'run-test'
        session.write_text('#!/usr/bin/python3\nimport os\nos.execv(' + repr('/usr/bin/python3') +
                           ', ["python3", ' + repr(str(ROOT / 'tools/gui' / test)) + '])\n')
        session.chmod(0o700)
        with (output / 'kwin.log').open('w') as log:
            proc = subprocess.Popen(['dbus-run-session', '--', args.kwin, '--virtual',
                '--width', '1536', '--height', '1000', '--socket', env['WAYLAND_DISPLAY'],
                '--no-lockscreen', '--no-global-shortcuts', '--no-kactivities',
                '--exit-with-session', str(session)], env=env, stdout=log,
                stderr=subprocess.STDOUT, start_new_session=True)
            try:
                code = proc.wait(timeout=90)
            finally:
                try:
                    os.killpg(proc.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
        text = (output / 'kwin.log').read_text()
        for line in text.splitlines():
            if line.startswith(('PASS ', 'FAIL ', '{"passed"', 'RESULT')):
                print(line, flush=True)
        print('KWIN_LOG', output / 'kwin.log', flush=True)
        # A compositor exit before launching its client is not a passing suite.
        assert ('PASS ' in text or 'FAIL ' in text) and ('"failed"' in text or 'RESULT ' in text), text[-4000:]
        raise SystemExit(code or int('FAIL ' in text))


if __name__ == '__main__':
    main()
