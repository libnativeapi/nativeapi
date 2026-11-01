#!/usr/bin/env python3
"""Real Linux desktop regression for tray_manager's legacy and README menus.

Run from the workspace through fastforge host exec. --build-only works without
a desktop; --run-only tests its saved bundle on the logged-on desktop. --baseline
builds against published tray_manager 0.8.1 and should fail the menu-export check.
No mouse or keyboard input: query the real panel's StatusNotifierWatcher and drive
the app's exported dbusmenu, then assert callbacks in Flutter's render tree.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import socket
import subprocess
import sys
import time
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
PACKAGE = ROOT / 'bindings/dart/tray_manager'
PROJECT = PACKAGE / 'example'
sys.path.insert(0, str(ROOT / '.agents/skills/flutter-ui-probe/scripts'))
from uiprobe import App


def run(command, **kwargs):
    print('RUN', subprocess.list2cmdline([str(value) for value in command]), flush=True)
    return subprocess.run(command, check=True, **kwargs)


def build(output, baseline):
    def example(name):
        return re.search(r'```dart\n(.*?)\n```', (PACKAGE / name).read_text(), re.S)[1]

    snippet = example('README.md')
    assert snippet == example('README-ZH.md'), 'README native examples differ'
    snippet = snippet.removeprefix("import 'package:tray_manager/tray_manager.dart';\n")
    template = (ROOT / 'tools/gui/tray_manager_menu_flutter.dart.in').read_text()
    target = PROJECT / 'lib/gui_test_main.dart'
    override = PROJECT / 'pubspec_overrides.yaml'
    assert not target.exists() and not override.exists(), 'Refusing to overwrite local test inputs'
    try:
        target.write_text(template.replace('{{README_NATIVE_EXAMPLE}}', snippet))
        if baseline:
            override.write_text('dependency_overrides:\n  tray_manager: 0.8.1\n')
        run(['flutter', 'pub', 'get'], cwd=PROJECT)
        run(['flutter', 'build', 'linux', '--debug', '-t', 'lib/gui_test_main.dart'], cwd=PROJECT)
        shutil.copytree(PROJECT / 'build/linux/x64/debug/bundle', output / 'bundle', dirs_exist_ok=True)
    finally:
        target.unlink(missing_ok=True)
        override.unlink(missing_ok=True)
        if baseline:
            run(['flutter', 'pub', 'get'], cwd=PROJECT)


def desktop_environment():
    # A virtual desktop supplies a complete private session; do not import the
    # console's user-manager environment or try to activate systemd on its bus.
    has_display = os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY')
    if has_display and os.environ.get('DBUS_SESSION_BUS_ADDRESS'):
        return
    result = subprocess.run(['systemctl', '--user', 'show-environment'],
                            capture_output=True, text=True, check=True)
    keys = {'DISPLAY', 'XAUTHORITY', 'WAYLAND_DISPLAY', 'XDG_RUNTIME_DIR',
            'DBUS_SESSION_BUS_ADDRESS', 'XDG_CURRENT_DESKTOP', 'XDG_SESSION_TYPE',
            'HYPRLAND_INSTANCE_SIGNATURE'}
    for line in result.stdout.splitlines():
        key, separator, value = line.partition('=')
        if separator and key in keys:
            os.environ.setdefault(key, value)
    assert os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY'), 'No logged-on graphical session'


def wait_for(predicate, message, timeout=40):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        value = predicate()
        if value:
            return value
        time.sleep(.2)
    raise AssertionError(message)


def verify(output):
    desktop_environment()
    from gi.repository import Gio, GLib

    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)

    def call(destination, path, interface, method, arguments=None):
        return bus.call_sync(destination, path, interface, method, arguments, None,
                             Gio.DBusCallFlags.NONE, 10000, None).unpack()

    def prop(destination, name):
        return call(destination, '/StatusNotifierItem', 'org.freedesktop.DBus.Properties',
                    'Get', GLib.Variant('(ss)', ('org.kde.StatusNotifierItem', name)))[0]

    def registered():
        return call('org.kde.StatusNotifierWatcher', '/StatusNotifierWatcher',
                    'org.freedesktop.DBus.Properties', 'Get',
                    GLib.Variant('(ss)', ('org.kde.StatusNotifierWatcher', 'RegisteredStatusNotifierItems')))[0]

    registered()  # A real panel's watcher must exist before launching the fixture.
    if 'hyprland' in os.environ.get('XDG_CURRENT_DESKTOP', '').lower():
        def cursor():
            return subprocess.check_output(['hyprctl', '-j', 'cursorpos'], text=True)
        before = cursor()
        time.sleep(1.5)
        assert cursor() == before, 'The cursor is moving; stop while the desktop is in use'
    else:
        idle = call('org.gnome.Mutter.IdleMonitor', '/org/gnome/Mutter/IdleMonitor/Core',
                    'org.gnome.Mutter.IdleMonitor', 'GetIdletime')[0]
        assert idle >= 1500, f'Desktop input {idle} ms ago; stop while the desktop is in use'
    executable = output / 'bundle/tray_manager_example'
    with (output / 'app.log').open('w') as log:
        process = subprocess.Popen([str(executable)], cwd=executable.parent,
                                   stdout=log, stderr=subprocess.STDOUT)
    app = App(output / 'app.log')
    passed = []
    report = {'host': socket.gethostname(), 'desktop': os.environ.get('XDG_CURRENT_DESKTOP'),
              'virtual_display': os.environ.get('GUI_TEST_HEADLESS_GNOME') == '1',
              'mode': output.name,
              'readme_sha256': hashlib.sha256((PACKAGE / 'README.md').read_bytes()).hexdigest()}
    if output.name == 'fixed':
        report['source_sha256'] = hashlib.sha256((PACKAGE / 'lib/src/tray_manager.dart').read_bytes()).hexdigest()

    def check(condition, message):
        assert condition, message
        passed.append(message)
        print('PASS', message, flush=True)

    def ours():
        # Watchers may append the object path to the service name (Quickshell).
        return [item.partition('/')[0] for item in registered()
                if f'StatusNotifierItem-{process.pid}-' in item]

    def items():
        return {prop(item, 'ToolTip')[2].removeprefix('tray-menu-regression-'): item for item in ours()}

    def layout(item):
        return call(item, '/StatusNotifierItem/Menu', 'com.canonical.dbusmenu', 'GetLayout',
                    GLib.Variant('(iias)', (0, -1, [])))[1]

    def entry(tree, label):
        if tree[1].get('label') == label:
            return tree
        for child in tree[2]:
            found = entry(child, label)
            if found:
                return found
        return None

    def click(item, label):
        node = entry(layout(item), label)
        assert node, f'Menu item missing: {label}'
        call(item, '/StatusNotifierItem/Menu', 'com.canonical.dbusmenu', 'Event',
             GLib.Variant('(isvu)', (node[0], 'clicked', GLib.Variant('i', 0), 0)))

    try:
        wait_for(lambda: app.vm_url() if process.poll() is None else None,
                 'Fixture did not expose a VM service')
        isolate = wait_for(
            lambda: next((item['id'] for item in app._call('getVM')['result']['isolates']
                          if item['name'] == 'main'), None),
            'Main Dart isolate did not start')
        wait_for(lambda: 'ext.tray_manager.menuRegression' in
                 app._call('getIsolate', isolateId=isolate)['result']['extensionRPCs'],
                 'Fixture extension did not register')

        def state(action='status'):
            query = urllib.parse.urlencode({'isolateId': isolate, 'action': action})
            url = app.vm_url() + 'ext.tray_manager.menuRegression?' + query
            with urllib.request.urlopen(url, timeout=10) as response:
                result = json.load(response)
            assert 'error' not in result, result
            return result['result']

        wait_for(lambda: len(ours()) == 2, 'The desktop did not register both tray icons')
        icons = items()
        check(set(icons) == {'legacy', 'native'}, 'desktop registers legacy and README icons')
        native, legacy = icons['native'], icons['legacy']
        check(prop(native, 'Menu') == '/StatusNotifierItem/Menu', 'README example exports its menu')
        click(native, 'Show Window')
        wait_for(lambda: state()['nativeClicks'] == 1, 'README callback missing')
        check(state()['nativeClicks'] == 1, 'README menu callback arrives exactly once')
        check(prop(legacy, 'Menu') == '/StatusNotifierItem/Menu',
              f'legacy menu is exported (actual: {prop(legacy, "Menu")})')
        check(not prop(legacy, 'ItemIsMenu'), 'right-click trigger keeps ItemIsMenu false')
        click(legacy, 'Legacy action 1')
        wait_for(lambda: state()['legacyClicks'] == 1, 'Legacy callback missing')
        check(state()['listenerCalls'] == 1, 'legacy callback and TrayListener arrive once')
        click(legacy, 'Legacy checkbox')
        wait_for(lambda: state()['checked'], 'Checkbox callback missing')
        check(entry(layout(legacy), 'Legacy checkbox')[1]['toggle-state'] == 1,
              'checkbox callback updates the exported menu')
        state('replace')
        check(entry(layout(legacy), 'Legacy action 2') is not None, 'replacement menu is exported')
        check(entry(layout(legacy), 'Legacy action 1') is None, 'replacement removes the old menu item')
        click(legacy, 'Legacy action 2')
        wait_for(lambda: state()['legacyClicks'] == 2, 'Replacement callback missing')
        check(state()['listenerCalls'] == 3, 'replacement callback arrives once')
        state('recreate')
        wait_for(lambda: len(ours()) == 2 and legacy not in ours(), 'Old icon remained registered')
        legacy = items()['legacy']
        check(prop(legacy, 'Menu') == '/StatusNotifierItem/Menu', 'recreated legacy icon exports its menu')
        click(legacy, 'Legacy action 3')
        wait_for(lambda: state()['legacyClicks'] == 3, 'Recreated callback missing')
        check(state()['listenerCalls'] == 4, 'recreated callback has no stale listeners')
        wait_for(lambda: any(view.has('Legacy callbacks: 3') and view.has('Native callbacks: 1')
                             for view in app.views()), 'Flutter did not show the actual callback state')
        check(True, 'Flutter render tree shows legacy and native callbacks')
        state('dispose')
        wait_for(lambda: not ours(), 'Disposed icons remained registered')
        check(state()['disposed'], 'disposing both icons removes them from the desktop')
        report['status'] = 'passed'
    except Exception as error:
        report.update(status='failed', error=str(error))
        raise
    finally:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)
        report['checks'] = passed
        (output / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
        print(json.dumps(report, indent=2), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-only', action='store_true')
    parser.add_argument('--run-only', action='store_true')
    parser.add_argument('--baseline', action='store_true')
    args = parser.parse_args()
    output = PROJECT / 'build/tray-menu-regression' / ('baseline' if args.baseline else 'fixed')
    output.mkdir(parents=True, exist_ok=True)
    if not args.run_only:
        build(output, args.baseline)
    if not args.build_only:
        verify(output)


if __name__ == '__main__':
    main()
