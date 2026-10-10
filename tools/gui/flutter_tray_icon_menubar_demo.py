#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = ["websocket-client>=1.8,<2"]
# ///
"""Record only this example's menu-bar items on the primary macOS display.

Uses the debug VM service to run the same controllers as the UI; sends no input.
Dry-run first, then --record. --pace scales the scene holds. Native capture is
restricted to the menu-bar rectangle, encoded once, without cuts or audio.
"""
import argparse
import json
import math
import os
from pathlib import Path
import subprocess
import time
import websocket

from common import EXAMPLES, OUTPUT, output_path
from guiapp import assert_idle, flutter_executable, inp
from recorder import Recorder, PERMISSION_HINT
from uiprobe import App


class MenuBarRecorder(Recorder):
    def __init__(self, output, rect):
        super().__init__(output, display=1)
        self.rect = rect

    def start(self):
        Path(self.movie).unlink(missing_ok=True)
        self.proc = subprocess.Popen(
            ['screencapture', '-x', '-v', '-D1',
             '-R' + ','.join(map(str, self.rect)), self.movie],
            stdin=subprocess.PIPE, stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL)
        time.sleep(1.5)
        if self.proc.poll() is not None:
            raise RuntimeError(PERMISSION_HINT)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--record', action='store_true')
    parser.add_argument('--pace', type=float, default=1)
    parser.add_argument('--countdown', type=int, default=3)
    args = parser.parse_args()
    os.makedirs(OUTPUT, exist_ok=True)
    screen = json.loads(subprocess.check_output(['swift', '-e', r'''import Cocoa
import CoreGraphics
let id = CGMainDisplayID()
let b = CGDisplayBounds(id)
let screen = NSScreen.screens.first { ($0.deviceDescription[NSDeviceDescriptionKey("NSScreenNumber")] as? NSNumber)?.uint32Value == id }!
print(String(data: try! JSONSerialization.data(withJSONObject: ["id": id, "width": b.width, "height": b.height, "bar": screen.frame.maxY - screen.visibleFrame.maxY, "scale": screen.backingScaleFactor]), encoding: .utf8)!)
'''], text=True))
    print('PRIMARY', screen, flush=True)
    log_path = os.path.join(OUTPUT, 'flutter_tray_icon_menubar_demo-macos.app.log')
    log = open(log_path, 'w')
    proc = subprocess.Popen([flutter_executable(os.path.join(EXAMPLES, 'flutter_tray_icon_example')),
                             '-ApplePersistenceIgnoreState', 'YES'],
                            stdout=log, stderr=subprocess.STDOUT)
    app = App(log_path)
    recorder = None
    warmup = None
    connection = None
    try:
        deadline = time.monotonic() + 40
        while not app.vm_url():
            if proc.poll() is not None or time.monotonic() > deadline:
                raise RuntimeError('App did not start: ' + log_path)
            time.sleep(.15)
        connection = websocket.create_connection(app.vm_url().replace('http:', 'ws:') + 'ws', timeout=15)
        request_id = 0
        id_zone = None

        def rpc(method, **params):
            nonlocal request_id
            request_id += 1
            if id_zone and method in ('getObject', 'getInstances', 'invoke', 'evaluate'):
                params['idZoneId'] = id_zone
            connection.send(json.dumps({'jsonrpc': '2.0', 'id': request_id, 'method': method, 'params': params}))
            while True:
                message = connection.recv()
                if not message:
                    raise websocket.WebSocketConnectionClosedException('App exited')
                result = json.loads(message)
                if result.get('id') == request_id:
                    break
            if 'error' in result:
                raise RuntimeError(result['error'])
            return result['result']

        iso = next(i['id'] for i in rpc('getVM')['isolates'] if i['name'] == 'main')
        # Keep controller and label IDs alive across all template checks.
        # The default service ring overwrites IDs during this longer scenario.
        id_zone = rpc('createIdZone', isolateId=iso, backingBufferKind='Ring',
                      idAssignmentPolicy='ReuseExisting', capacity=100000)['id']
        libraries = rpc('getIsolate', isolateId=iso)['libraries']
        def invoke(target, selector, *args):
            result = rpc('invoke', isolateId=iso, targetId=target['id'], selector=selector,
                         argumentIds=[arg['id'] for arg in args])
            if result['type'] in ('@Error', 'Error', 'Sentinel'):
                raise RuntimeError(result)
            return result

        def field(target, name):
            obj = rpc('getObject', isolateId=iso, objectId=target['id'])
            return next(item['value'] for item in obj['fields'] if item['decl']['name'] == name)

        def get(target, name):
            return invoke(target, 'get:' + name)

        def value(target):
            return target.get('valueAsString')

        def number(target):
            return float(value(target))

        def integer(n):
            return {'id': f'objects/int-{n}'}

        def instances(uri, name, limit=30):
            library = next(lib for lib in libraries if lib['uri'] == uri)
            cls = next(item for item in rpc('getObject', isolateId=iso, objectId=library['id'])['classes'] if item['name'] == name)
            return rpc('getInstances', isolateId=iso, objectId=cls['id'], limit=limit)['instances']

        deadline = time.monotonic() + 15
        while not (controllers := instances('package:tray_icon_example/tray_controller.dart', 'TrayController')):
            if time.monotonic() > deadline:
                raise RuntimeError(f'Tray controller did not initialize; see {log.name}')
            time.sleep(.1)
        controller = controllers[0]
        time.sleep(.5)
        yes, no = {'id': 'objects/bool-true'}, {'id': 'objects/bool-false'}
        window = field(controller, '_settingsWindow')
        position = get(window, 'position')
        print('SETTINGS POSITION', {key: number(field(position, key)) for key in ('x', 'y')}, flush=True)
        assert_idle()
        # Move only our own window through Accessibility. Keeping it on the
        # primary screen anchors AppKit's active menu bar there for the take.
        inp('setframe', proc.pid, 464, 127, 800, 628, 'Tray Icon')
        invoke(window, 'show')
        inp('activate', proc.pid)
        invoke(window, 'focus')
        time.sleep(.4)

        def items(uri, name):
            return instances(uri, name)

        styles = items('package:tray_icon_example/signs/sign_style.dart', 'SignStyle')
        styles = {int(number(get(item, 'index'))): item for item in styles}
        original = get(controller, 'selected')
        if args.record:
            # Stabilize macOS's own recording indicator before measuring the
            # demo. Include the system controls so the menu-bar context is clear.
            warmup = MenuBarRecorder(os.path.join(OUTPUT, '.menubar-warmup.mp4'), (0, 0, int(screen['width']), int(screen['bar'])))
            warmup.start()
            invoke(controller, 'removeIcon', original)
            original = invoke(controller, 'addIcon')
        active = [original]
        def bounds(entry):
            rect = invoke(field(entry, 'trayIcon'), 'getBounds')
            return {key: number(field(rect, key)) for key in ('x', 'y', 'width', 'height')}

        def wait_native(entry):
            end = time.monotonic() + 5
            sign = field(entry, 'sign')
            while value(get(field(sign, '_traySign'), 'isReady')) != 'true':
                if time.monotonic() > end:
                    raise RuntimeError('Native typography did not finish')
                time.sleep(.05)

        # Prepare native status items and measure the widest scene. Reveal the
        # three animated icons together after the sign demonstration.
        extras = [invoke(controller, 'addIcon') for _ in range(2)]
        sign_entries = [invoke(controller, 'addSign', styles[i]) for i in range(2)]
        models = [field(entry, 'sign') for entry in sign_entries]
        for entry in sign_entries:
            wait_native(entry)
        city_content = get(models[1], 'content')
        city_name, city_latin = field(city_content, 'primary'), field(city_content, 'secondary')
        invoke(models[0], 'selectStyle', integer(3))
        invoke(models[1], 'selectStyle', integer(0))
        active = [original, *extras, *sign_entries]
        time.sleep(.6)
        measured = [bounds(entry) for entry in active]
        print('PREFLIGHT', measured, flush=True)
        right_edge = screen['width']
        if any(b['x'] <= 0 or b['y'] < -1 or b['y'] > screen['bar'] or
               b['x'] + b['width'] > screen['width'] for b in measured):
            raise RuntimeError('Tray items are not visible on the primary display')
        left = math.floor(min(b['x'] for b in measured)) - 14
        rect = (left, 0, math.ceil(right_edge - left), math.ceil(screen['bar']))
        print('CAPTURE', rect, flush=True)
        still = os.path.join(OUTPUT, 'flutter_tray_icon_menubar_demo-preflight.png')
        subprocess.run(['screencapture', '-x', '-D1', '-R' + ','.join(map(str, rect)), still], check=True)
        for entry in [original, *extras]:
            invoke(field(entry, 'trayIcon'), 'setVisible', no)
        invoke(models[0], 'selectStyle', integer(0))
        invoke(models[1], 'selectStyle', integer(1))
        active = list(sign_entries)
        time.sleep(.4)

        for remaining in range(args.countdown, 0, -1):
            print(f'Starting in {remaining}… no mouse or keyboard input', flush=True)
            time.sleep(1)
        assert_idle()
        if args.record:
            recorder = MenuBarRecorder(output_path(__file__, 'macos'), rect)
            recorder.start()
            warmup.stop()
            Path(warmup.movie).unlink(missing_ok=True)
            warmup = None

        def hold(label, seconds):
            print('SCENE', label, flush=True)
            time.sleep(max(.15, seconds * args.pace))
            boxes = [bounds(entry) for entry in active]
            if any(b['x'] < rect[0] or b['x'] + b['width'] > right_edge + 1 or
                   abs(b['y']) > screen['bar'] for b in boxes):
                raise RuntimeError(f'Scene escaped the primary menu-bar crop: {boxes}')

        missing_model, welcome_model = models
        hold('Signs first · Shanghai and Dali', 3)
        invoke(missing_model, 'setPlace', city_name, city_latin)
        invoke(missing_model, 'setGreen', yes)
        invoke(missing_model, 'setRight', no)
        hold('Live place, color and direction', 3)
        invoke(missing_model, 'selectStyle', integer(2))
        hold('Travel and welcome signs', 3)
        invoke(missing_model, 'selectStyle', integer(3))
        invoke(welcome_model, 'selectStyle', integer(0))
        invoke(welcome_model, 'setPlace', city_name, city_latin)
        invoke(welcome_model, 'setGreen', yes)
        hold('Guide and Missing You signs', 3)
        invoke(controller, 'playThreeAtOnce')
        for entry in [original, *extras]:
            invoke(field(entry, 'trayIcon'), 'setVisible', yes)
        entries = get(controller, 'entries')
        count = int(number(get(entries, 'length')))
        active = [invoke(entries, '[]', integer(i)) for i in range(count)]
        if count != 5:
            raise RuntimeError(f'Expected two signs and three icons; found {count}')
        hold('Add three animated icons together · coexist with signs', 6)
        print('PROOF native tray bounds:', [bounds(entry) for entry in active], flush=True)
        if recorder:
            recorder.stop()
            recorder.convert()
            recorder = None
    finally:
        if warmup:
            warmup.stop()
            Path(warmup.movie).unlink(missing_ok=True)
        if recorder:
            recorder.stop()
        if connection:
            connection.close()
        if proc.poll() is None:
            proc.terminate()
            proc.wait(timeout=10)
        log.close()
        print('App log:', log_path, flush=True)


if __name__ == '__main__':
    main()
