#!/usr/bin/env python3
"""Run a command in an isolated GNOME Wayland desktop with a virtual monitor.

Uses the installed GNOME Shell and software rendering; no console login or sudo.
The display, bus and settings live for the command. Logs and display metadata are
kept in --output-dir; runtime files and child processes are cleaned up on exit.
"""

import argparse
from datetime import datetime
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--size', default='1280x720', help='Virtual monitor pixels, WIDTHxHEIGHT')
    parser.add_argument('--extension', action='append', default=[], help='Enable a system GNOME extension')
    parser.add_argument('--wait-name', action='append', default=[], help='Wait for a desktop D-Bus service')
    parser.add_argument('--output-dir', type=Path, help='Parent directory for retained logs')
    parser.add_argument('--startup-timeout', type=float, default=60)
    parser.add_argument('--session-child', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if args.command[:1] == ['--']:
        args.command.pop(0)
    if not args.command:
        parser.error('A command is required after --')
    if not re.fullmatch(r'[1-9]\d*x[1-9]\d*', args.size):
        parser.error('--size must be positive WIDTHxHEIGHT')
    return args


def stop(process):
    if process.poll() is None:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)


def session(args):
    from gi.repository import Gio, GLib

    output = Path(os.environ['GUI_TEST_GNOME_LOG_DIR'])
    version = subprocess.check_output(['gnome-shell', '--version'], text=True).strip()
    settings = [
        ('org.gnome.shell', 'enabled-extensions', repr(args.extension)),
        ('org.gnome.shell', 'disable-user-extensions', 'false'),
        ('org.gnome.shell', 'welcome-dialog-last-shown-version', repr(version.split()[-1])),
        ('org.gnome.desktop.interface', 'enable-animations', 'false'),
        ('org.gnome.desktop.session', 'idle-delay', '0'),
        ('org.gnome.desktop.screensaver', 'lock-enabled', 'false'),
    ]
    for schema, key, value in settings:
        subprocess.run(['gsettings', 'set', schema, key, value], check=True)

    with (output / 'gnome-shell.log').open('w') as log:
        shell = subprocess.Popen([
            'gnome-shell', '--wayland', '--headless', '--no-x11', '--sm-disable',
            '--wayland-display=' + os.environ['WAYLAND_DISPLAY'],
            '--virtual-monitor=' + args.size,
        ], stdout=log, stderr=subprocess.STDOUT)
    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)

    def call(destination, path, interface, method, arguments=None):
        return bus.call_sync(destination, path, interface, method, arguments, None,
                             Gio.DBusCallFlags.NONE, 1000, None).unpack()

    def has_name(name):
        return call('org.freedesktop.DBus', '/org/freedesktop/DBus',
                    'org.freedesktop.DBus', 'NameHasOwner', GLib.Variant('(s)', (name,)))[0]

    try:
        deadline = time.monotonic() + args.startup_timeout
        names = ['org.gnome.Shell', 'org.gnome.Mutter.DisplayConfig', *args.wait_name]
        while not all(has_name(name) for name in names):
            if shell.poll() is not None:
                raise RuntimeError(f'GNOME Shell exited with {shell.returncode}; see {output}')
            if time.monotonic() >= deadline:
                missing = [name for name in names if not has_name(name)]
                raise TimeoutError(f'Desktop services did not start: {missing}; see {output}')
            time.sleep(.2)
        display = call('org.gnome.Mutter.DisplayConfig', '/org/gnome/Mutter/DisplayConfig',
                       'org.gnome.Mutter.DisplayConfig', 'GetCurrentState')
        monitors = []
        for spec, modes, _ in display[1]:
            for mode in modes:
                if mode[-1].get('is-current'):
                    monitors.append({'name': spec[0], 'width': mode[1], 'height': mode[2],
                                     'refresh_hz': mode[3]})
        width, height = map(int, args.size.split('x'))
        assert any((monitor['width'], monitor['height']) == (width, height)
                   for monitor in monitors), f'Virtual monitor did not activate: {monitors}'
        metadata = {'gnome_version': version, 'session_type': 'wayland',
                    'software_rendering': True, 'monitors': monitors,
                    'extensions': args.extension, 'command': args.command}
        (output / 'session.json').write_text(json.dumps(metadata, indent=2) + '\n')
        print('GNOME virtual display ready:', json.dumps(monitors), flush=True)
        print('GNOME session logs:', output, flush=True)
        result = subprocess.run(args.command)
        metadata['command_exit_code'] = result.returncode
        (output / 'session.json').write_text(json.dumps(metadata, indent=2) + '\n')
        return result.returncode
    finally:
        stop(shell)


def main():
    args = parse_args()
    if args.session_child:
        return session(args)
    if sys.platform != 'linux':
        raise SystemExit('This runner requires Linux and an installed GNOME Shell')
    parent = args.output_dir or Path(tempfile.mkdtemp(prefix='gnome-headless-logs-'))
    output = (parent / (datetime.now().strftime('%Y%m%d-%H%M%S') + f'-{os.getpid()}')).resolve()
    output.mkdir(parents=True, exist_ok=False)
    print('Headless GNOME output:', output, flush=True)
    # Wayland Unix socket paths must stay short, independently of the log path.
    with tempfile.TemporaryDirectory(prefix='gui-gnome-') as temporary:
        root = Path(temporary)
        env = os.environ.copy()
        for key in ['DISPLAY', 'WAYLAND_DISPLAY', 'XAUTHORITY', 'DBUS_SESSION_BUS_ADDRESS',
                    'HYPRLAND_INSTANCE_SIGNATURE', 'GNOME_SETUP_DISPLAY']:
            env.pop(key, None)
        for key, directory in [('XDG_CONFIG_HOME', 'config'), ('XDG_CACHE_HOME', 'cache'),
                               ('XDG_DATA_HOME', 'data'), ('XDG_RUNTIME_DIR', 'run')]:
            path = root / directory
            path.mkdir(mode=0o700)
            env[key] = str(path)
        env.update(WAYLAND_DISPLAY='gui-test-gnome', XDG_CURRENT_DESKTOP='GNOME',
                   XDG_SESSION_TYPE='wayland', XDG_SESSION_DESKTOP='gnome',
                   GNOME_SHELL_SESSION_MODE='user', GDK_BACKEND='wayland',
                   LIBGL_ALWAYS_SOFTWARE='1', MESA_LOADER_DRIVER_OVERRIDE='swrast',
                   GSETTINGS_BACKEND='keyfile', GUI_TEST_HEADLESS_GNOME='1',
                   GUI_TEST_GNOME_LOG_DIR=str(output))
        with (output / 'session.log').open('w') as log:
            process = subprocess.Popen([
                'dbus-run-session', '--', sys.executable, str(Path(__file__).resolve()),
                '--session-child', *sys.argv[1:],
            ], env=env, stderr=log, start_new_session=True)
            try:
                code = process.wait()
            finally:
                # Only this runner's new process group, including bus-activated helpers.
                try:
                    os.killpg(process.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
                stop(process)
                time.sleep(.2)
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
        if code:
            for name in ['session.log', 'gnome-shell.log']:
                lines = (output / name).read_text(errors='replace').splitlines()
                print(f'{name} (last 20 lines):', file=sys.stderr)
                print('\n'.join(lines[-20:]), file=sys.stderr)
        return code


if __name__ == '__main__':
    raise SystemExit(main())
