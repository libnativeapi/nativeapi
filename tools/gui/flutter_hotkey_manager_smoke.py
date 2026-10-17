#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = ["websocket-client>=1.8,<2"]
# ///
"""No-input smoke test of hotkey_manager's real desktop Flutter example.

Build the submodule's example with `flutter build macos|windows|linux --debug` first, using
local nativeapi/cnativeapi overrides while 0.5.3 is unpublished. The debug VM
service invokes the example's registration actions. Activation is injected
through the native manager; this does not test physical keyboard delivery.
"""

import argparse
import ctypes
import json
import platform
from pathlib import Path
import subprocess
import sys
import tempfile
import time

import websocket

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.agents/skills/flutter-ui-probe/scripts'))
from uiprobe import App


def os_shortcut_taken():
    """Probe Ctrl+Alt+H through a second OS client, without sending any keys."""
    if platform.system() == 'Windows':
        user32 = ctypes.WinDLL('user32', use_last_error=True)
        if user32.RegisterHotKey(None, 0x7ffe, 0x4003, ord('H')):
            assert user32.UnregisterHotKey(None, 0x7ffe)
            return False
        error = ctypes.get_last_error()
        assert error == 1409, f'RegisterHotKey error: {error}'
        return True
    if platform.system() != 'Linux':
        return None

    class XErrorEvent(ctypes.Structure):
        _fields_ = [('type', ctypes.c_int), ('display', ctypes.c_void_p),
                    ('resourceid', ctypes.c_ulong), ('serial', ctypes.c_ulong),
                    ('error_code', ctypes.c_ubyte), ('request_code', ctypes.c_ubyte),
                    ('minor_code', ctypes.c_ubyte)]

    x11 = ctypes.CDLL('libX11.so.6')
    x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
    x11.XOpenDisplay.restype = ctypes.c_void_p
    x11.XDefaultRootWindow.argtypes = [ctypes.c_void_p]
    x11.XDefaultRootWindow.restype = ctypes.c_ulong
    x11.XKeysymToKeycode.argtypes = [ctypes.c_void_p, ctypes.c_ulong]
    x11.XKeysymToKeycode.restype = ctypes.c_ubyte
    x11.XGrabKey.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_uint, ctypes.c_ulong,
                            ctypes.c_int, ctypes.c_int, ctypes.c_int]
    x11.XUngrabKey.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_uint, ctypes.c_ulong]
    x11.XSync.argtypes = [ctypes.c_void_p, ctypes.c_int]
    x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
    x11.XSetErrorHandler.argtypes = [ctypes.c_void_p]
    x11.XSetErrorHandler.restype = ctypes.c_void_p
    display = x11.XOpenDisplay(None)
    assert display, 'No X11 display for independent shortcut ownership check'
    errors = []

    @ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.POINTER(XErrorEvent))
    def error_handler(_, event):
        errors.append(event.contents.error_code)
        return 0

    previous = x11.XSetErrorHandler(ctypes.cast(error_handler, ctypes.c_void_p))
    try:
        root = x11.XDefaultRootWindow(display)
        key = x11.XKeysymToKeycode(display, ord('H'))
        assert key != 0
        x11.XGrabKey(display, key, 4 | 8, root, 1, 1, 1)
        x11.XSync(display, 0)
        if not errors:
            x11.XUngrabKey(display, key, 4 | 8, root)
            x11.XSync(display, 0)
            return False
        assert errors == [10], f'XGrabKey errors: {errors}'  # BadAccess: another client owns it.
        return True
    finally:
        x11.XCloseDisplay(display)
        x11.XSetErrorHandler(previous)


def main():
    project = ROOT / 'bindings/dart/hotkey_manager/packages/hotkey_manager/example'
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path)
    args = parser.parse_args()
    executable = args.executable or project / {
        'Darwin': 'build/macos/Build/Products/Debug/hotkey_manager_example.app/Contents/MacOS/hotkey_manager_example',
        'Windows': 'build/windows/x64/runner/Debug/hotkey_manager_example.exe',
        'Linux': 'build/linux/x64/debug/bundle/hotkey_manager_example',
    }[platform.system()]
    executable = executable.resolve()
    with tempfile.NamedTemporaryFile(suffix='.hotkey-manager.log', delete=False) as log:
        process = subprocess.Popen([str(executable)], cwd=executable.parent,
                                   stdout=log, stderr=subprocess.STDOUT)
    app = App(log.name)
    connection = None
    request_id = 0

    def call(method, **params):
        nonlocal request_id
        request_id += 1
        connection.send(json.dumps({'jsonrpc': '2.0', 'id': request_id,
                                    'method': method, 'params': params}))
        while True:
            payload = json.loads(connection.recv())
            if payload.get('id') == request_id:
                break
        if 'error' in payload:
            raise AssertionError(payload['error'])
        result = payload['result']
        assert result.get('type') not in ('@Error', 'Error', 'Sentinel'), result
        return result

    try:
        deadline = time.monotonic() + 40
        while not app.vm_url():
            assert process.poll() is None, Path(log.name).read_text()
            assert time.monotonic() < deadline, f'No VM service: {log.name}'
            time.sleep(.2)
        connection = websocket.create_connection(app.vm_url().replace('http:', 'ws:') + 'ws', timeout=10)
        deadline = time.monotonic() + 15
        while True:
            isolates = [i['id'] for i in call('getVM')['isolates'] if i['name'] == 'main']
            if isolates:
                iso = isolates[0]
                break
            assert process.poll() is None, Path(log.name).read_text()
            assert time.monotonic() < deadline, 'Main Dart isolate did not start'
            time.sleep(.1)

        def invoke(target, selector, *arguments):
            return call('invoke', isolateId=iso, targetId=target['id'],
                        selector=selector, argumentIds=[a['id'] for a in arguments])

        def field(target, name):
            fields = call('getObject', isolateId=iso, objectId=target['id'])['fields']
            return next(f['value'] for f in fields if f['decl']['name'] == name)

        classes = call('getClassList', isolateId=iso)['classes']

        def instance(name):
            cls = next(c['id'] for c in classes if c['name'] == name)
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                objects = call('getInstances', isolateId=iso, objectId=cls, limit=1)['instances']
                if objects:
                    return objects[0]
                time.sleep(.1)
            raise AssertionError(f'No live {name}')

        state = instance('_NativeShortcutsPageState')
        manager = field(state, '_manager')
        binding = instance('WidgetsFlutterBinding')

        def rendered(text):
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline:
                invoke(binding, 'scheduleWarmUpFrame')
                if any(view.has(text) for view in app.views()):
                    return
                time.sleep(.1)
            raise AssertionError(f'Not rendered: {text}')

        def press(label):
            rendered(label)
            cls = next(c['id'] for c in classes if c['name'] == 'FilledButton')
            buttons = call('getInstances', isolateId=iso, objectId=cls, limit=100)['instances']
            for button in buttons:
                child = invoke(button, 'get:child')
                if invoke(child, 'get:data').get('valueAsString') == label:
                    invoke(invoke(button, 'get:onPressed'), 'call')
                    return
            raise AssertionError(f'No callback for {label}')

        rendered('Register shortcut')
        press('Register shortcut')
        rendered('Unregister shortcut')
        shortcut = field(state, '_shortcut')
        shortcut_id = invoke(shortcut, 'get:id')
        accelerator = invoke(shortcut, 'get:accelerator')
        assert accelerator['valueAsString'] == 'Ctrl+Alt+H', accelerator
        if platform.system() in ('Windows', 'Linux'):
            assert os_shortcut_taken(), 'Native registration did not reserve the OS shortcut'
        print('PASS example registers a real native shortcut and renders registered state')

        invoke(manager, 'emitShortcutActivated', shortcut_id, accelerator)
        rendered('Pressed: 1    Released: 0')
        print('PASS asynchronous native activation reaches the Flutter counter')

        press('Unregister shortcut')
        assert invoke(manager, 'getWithId', shortcut_id).get('kind') == 'Null'
        if platform.system() in ('Windows', 'Linux'):
            assert not os_shortcut_taken(), 'OS shortcut remained reserved after unregistration'
        invoke(manager, 'emitShortcutActivated', shortcut_id, accelerator)
        rendered('Pressed: 1    Released: 0')
        time.sleep(.2)
        assert field(state, '_presses')['valueAsString'] == '1'
        print('PASS unregistration removes the native shortcut and stale activation is ignored')

        press('Register shortcut')
        shortcut_id = invoke(field(state, '_shortcut'), 'get:id')
        invoke(instance('HotKeyManager'), 'unregisterAll')
        assert invoke(manager, 'getWithId', shortcut_id).get('kind') == 'Null'
        press('Unregister shortcut')
        press('Register shortcut')
        rendered('Unregister shortcut')
        shortcut_id = invoke(field(state, '_shortcut'), 'get:id')
        invoke(manager, 'emitShortcutActivated', shortcut_id, accelerator)
        rendered('Pressed: 2    Released: 0')
        press('Unregister shortcut')
        print('PASS startup cleanup reclaims the example registration and permits reuse')
        output = Path(log.name).read_text()
        assert 'EXCEPTION CAUGHT' not in output and 'Unhandled Exception' not in output, output
        print('4 checks passed; no mouse or keyboard input sent')
    finally:
        if connection:
            connection.close()
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)
        print(f'App log: {log.name}')


if __name__ == '__main__':
    main()
