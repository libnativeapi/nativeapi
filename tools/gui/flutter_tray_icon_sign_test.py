#!/usr/bin/env python3
# /// script
# requires-python = ">=3.10"
# dependencies = ["websocket-client>=1.8,<2"]
# ///
"""macOS integrated tray icon / sign smoke test, without synthetic input.

Build the Flutter example in debug first. The VM service invokes the same
controller methods as the settings page and reads the real native getters.
Screenshots in tools/gui/output/ are for visual inspection, not assertions.
"""

import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

import websocket

from common import EXAMPLES, OUTPUT
from guiapp import Checks, flutter_executable
from uiprobe import App


def main():
    checks = Checks()
    # Capture this process's windows directly, even while the user works in a
    # different app. A screen rectangle can contain an unrelated front window.
    capture_tools = tempfile.TemporaryDirectory(prefix='tray-sign-capture-')
    window_source = Path(capture_tools.name) / 'windows.swift'
    window_source.write_text('''
import Cocoa
let pid = Int(CommandLine.arguments[1])!
let windows = CGWindowListCopyWindowInfo(.optionOnScreenOnly, kCGNullWindowID)
  as? [[String: Any]] ?? []
let owned = windows.filter {
  ($0[kCGWindowOwnerPID as String] as? Int) == pid &&
  ($0[kCGWindowLayer as String] as? Int) == 0
}.map { window in
  ["id": window[kCGWindowNumber as String] ?? 0,
   "title": window[kCGWindowName as String] ?? "",
   "bounds": window[kCGWindowBounds as String] ?? [:]]
}
let data = try JSONSerialization.data(withJSONObject: owned)
print(String(data: data, encoding: .utf8)!)
''')
    window_tool = str(Path(capture_tools.name) / 'windows')
    subprocess.run(['swiftc', str(window_source), '-o', window_tool], check=True)
    log = tempfile.NamedTemporaryFile(suffix='.tray-icon-sign.log', delete=False)
    project = os.path.join(EXAMPLES, 'flutter_tray_icon_example')
    proc = subprocess.Popen(
        [flutter_executable(project), '-ApplePersistenceIgnoreState', 'YES'],
        stdout=log, stderr=subprocess.STDOUT,
    )
    app = App(log.name)
    os.makedirs(OUTPUT, exist_ok=True)
    try:
        deadline = time.monotonic() + 40
        while not app.vm_url():
            if proc.poll() is not None or time.monotonic() > deadline:
                raise RuntimeError(f'App did not start; see {log.name}')
            time.sleep(0.25)

        connection = websocket.create_connection(app.vm_url().replace('http:', 'ws:') + 'ws', timeout=15)
        request_id = 0
        id_zone = None

        def rpc(method, **params):
            nonlocal request_id
            request_id += 1
            if id_zone and method in ('getObject', 'getInstances', 'invoke'):
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
        binding = instances('package:flutter/src/widgets/binding.dart', 'WidgetsFlutterBinding')[0]
        yes, no = {'id': 'objects/bool-true'}, {'id': 'objects/bool-false'}
        def count():
            return number(get(field(controller, 'entries'), 'length'))

        def wait_preview(sign):
            deadline = time.monotonic() + 15
            diagnostics = []
            while time.monotonic() < deadline:
                invoke(binding, 'scheduleWarmUpFrame')
                preview_states = instances('package:tray_icon_example/signs/embedded_sign_preview.dart', '_EmbeddedSignPreviewState')
                seen = []
                for state in preview_states:
                    seen.append((value(field(state, '_disposed')), value(field(get(get(state, 'widget'), 'controller'), 'id'))))
                    if value(field(state, '_disposed')) == 'true':
                        continue
                    if value(field(get(get(state, 'widget'), 'controller'), 'id')) != value(field(sign, 'id')):
                        continue
                    native = field(state, '_sign')
                    tray_sign = field(sign, '_traySign')
                    diagnostics = [value(field(state, '_registered')), value(get(state, 'isAttached')),
                                   value(get(native, 'isReady')), value(get(tray_sign, 'isReady')),
                                   value(get(field(native, '_title'), 'text')),
                                   value(get(field(tray_sign, '_title'), 'text'))]
                    if (native.get('kind') != 'Null' and value(get(state, 'isAttached')) == 'true' and
                        value(get(native, 'isReady')) == 'true' and value(get(tray_sign, 'isReady')) == 'true' and
                        value(get(field(native, '_title'), 'text')) == value(get(field(tray_sign, '_title'), 'text'))):
                        return state, native
                time.sleep(.1)
            snapshot('flutter_tray_icon-attachment-failure')
            Path('/tmp/tray-icon-render-tree.json').write_text(json.dumps(rpc('ext.flutter.debugDumpRenderTree', isolateId=iso)))
            raise RuntimeError(f'PlatformView did not attach to sign {value(field(sign, "id"))}: {diagnostics}; states {seen}; selected {value(field(get(controller, "selected"), "number"))}; listener count {value(field(controller, "_count"))}; phase {value(invoke(get(binding,"schedulerPhase"), "toString"))}')

        def snapshot(name):
            windows = json.loads(subprocess.check_output([window_tool, str(proc.pid)], text=True))
            owned = [item for item in windows if item['title'] == 'Tray Icon']
            if not owned:
                owned = [item for item in windows if abs(item['bounds']['Width'] - 800) < 3]
            if len(owned) == 1:
                subprocess.run(['screencapture', '-x', '-o', '-l', str(owned[0]['id']),
                                os.path.join(OUTPUT, name + '.png')], check=True)

        time.sleep(.5)
        checks.check('settings window subscribes after Flutter creates it',
                     field(controller, '_settingsWindow')['kind'] != 'Null' and
                     field(controller, '_settingsWindowListener')['kind'] != 'Null')
        checks.check('starts with one ordinary tray icon', count() == 1 and value(get(get(controller, 'selected'), 'isSign')) == 'false')
        first = invoke(controller, 'addSign')
        sign = field(first, 'sign')
        state, embedded = wait_preview(sign)
        snapshot('flutter_tray_icon-merged-signs-macos')
        checks.check('Add sign preserves the original icon', count() == 2)
        checks.check('inline native face is 40 pt high', abs(number(get(embedded, 'height')) - 40) < .01)
        tray_sign = field(sign, '_traySign')
        tray = field(first, 'trayIcon')
        root = field(sign, '_trayRoot')
        checks.check('sign is native content on its managed TrayIcon',
                     value(get(get(tray, 'contentView'), 'id')) == value(get(root, 'id')))
        checks.check('PlatformView owns a separate native tree',
                     value(get(field(embedded, 'view'), 'id')) != value(get(field(tray_sign, 'view'), 'id')))
        checks.check('native label contains the Chinese sample', value(get(field(tray_sign, '_title'), 'text')) == '我在上海很想你')
        invoke(sign, 'setEnglish', no)
        checks.check('Missing You always keeps its pinyin strip',
                     value(get(field(tray_sign, '_caption'), 'isVisible')) == 'true' and value(field(sign, 'english')) == 'true')
        content = get(sign, 'content')
        primary, secondary = field(content, 'primary'), field(content, 'secondary')
        empty = invoke(primary, 'substring', integer(0), integer(0))
        checks.check('invalid edit does not replace current content',
                     invoke(sign, 'setPlace', empty, secondary)['kind'] == 'String' and
                     value(get(field(tray_sign, '_title'), 'text')) == '我在上海很想你')
        edited = invoke(primary, '*', integer(2))
        invoke(sign, 'setPlace', edited, secondary)
        invoke(sign, 'setGreen', yes)
        wait_preview(sign)
        entries = [first]
        for i, expected in enumerate(['大理欢迎您', '下一站，山海', '观景台'], 1):
            entry = invoke(controller, 'addSign')
            owner = field(entry, 'sign')
            invoke(owner, 'selectStyle', integer(i))
            _, native = wait_preview(owner)
            checks.check(f'{i}: selected style reaches its PlatformView', value(get(field(native, '_title'), 'text')) == expected)
            entries.append(entry)
        checks.check('four signs coexist with the image icon', count() == 5)
        checks.check('all signs have independent native tray IDs', len({value(invoke(field(entry, 'trayIcon'), 'getId')) for entry in entries}) == 4)
        for entry in entries:
            owner = field(entry, 'sign')
            native = field(owner, '_traySign')
            container = field(owner, '_trayRoot')
            frame = get(field(native, 'view'), 'frame')
            preferred = get(container, 'preferredSize')
            left = number(field(frame, 'x'))
            right = number(field(preferred, 'width')) - left - number(field(frame, 'width'))
            checks.check(f'{value(field(entry, "number"))}: equal transparent side padding', abs(left - 4) < .01 and abs(right - 4) < .01)
            for key in ('_title', '_caption', '_west', '_east', '_w', '_e', '_badgeNumber', '_detail'):
                label = field(native, key)
                if value(get(label, 'isVisible')) == 'true':
                    checks.check(f'{value(field(entry, "number"))}: {key} fits native frame',
                      number(field(get(label, 'frame'), 'width')) + .01 >= number(field(get(label, 'intrinsicSize'), 'width')))
        guide = field(entries[-1], 'sign')
        invoke(guide, 'setEnglish', no)
        checks.check('English subtitle remains optional for guide', value(get(field(field(guide, '_traySign'), '_caption'), 'isVisible')) == 'false')
        invoke(guide, 'setEnglish', yes)
        invoke(guide, 'showPreview')
        time.sleep(.5)
        preview = field(guide, '_previewWindow')
        checks.check('enlarged native preview opens', value(get(preview, 'isVisible')) == 'true')
        invoke(preview, 'close')
        time.sleep(.5)
        checks.check('closing preview releases its native tree', field(guide, '_previewSign')['kind'] == 'Null')
        invoke(guide, 'showPreview')
        time.sleep(.5)
        invoke(controller, 'select', first)
        wait_preview(sign)
        checks.check('switching selection preserves edited text and color',
                     value(field(get(sign, 'content'), 'primary')) == value(edited) and value(field(sign, 'green')) == 'true')
        invoke(controller, 'addThreeIcons')
        checks.check('Three icons adds images without replacing signs', count() == 8 and all(value(get(entry, 'isSign')) == 'true' for entry in entries))
        invoke(controller, 'playThreeAtOnce')
        checks.check('Three icons preset preserves all native sign trees', all(field(field(entry, 'sign'), '_trayRoot')['kind'] != 'Null' for entry in entries))
        image_entry = get(field(controller, 'entries'), 'last')
        animator = field(image_entry, 'animator')
        animations = instances('package:tray_icon_example/icon_animations.dart', 'IconAnimation')
        widget_animation = next(item for item in animations if value(get(item, 'label')) == 'Any widget')
        invoke(controller, 'play', widget_animation, image_entry)
        invoke(controller, 'select', first)
        wait_preview(sign)
        time.sleep(1)
        checks.check('widget capture keeps rendering while a sign is selected',
                     number(field(animator, 'frames')) > 5 and get(animator, 'lastFrame')['kind'] != 'Null')
        invoke(controller, 'select', entries[-1])
        wait_preview(guide)
        invoke(controller, 'removeIcon', entries[-1])
        time.sleep(.5)
        checks.check('removing a sign closes its enlarged preview', field(guide, '_previewWindow')['kind'] == 'Null')
        checks.check('removal preserves other signs and icons', count() == 7)
        invoke(controller, 'select', first)
        wait_preview(sign)
        settings = field(sign, 'settingsWindow')
        invoke(settings, 'close')
        time.sleep(.4)
        checks.check('closing settings keeps tray icons alive', value(get(settings, 'isVisible')) == 'false' and value(invoke(tray, 'isVisible')) == 'true')
        invoke(sign, 'showSettings')
        wait_preview(sign)
        snapshot('flutter_tray_icon-merged-signs-macos')
        for entry in list(entries[:-1]):
            invoke(controller, 'removeIcon', entry)
        time.sleep(.6)
        checks.check('all sign removals preserve the image entries', count() == 4)
        invoke(binding, 'scheduleWarmUpFrame')
        time.sleep(.3)
        output = Path(log.name).read_text()
        checks.check('no Flutter, native or disposal exceptions', not any(message in output for message in
            ('EXCEPTION CAUGHT', 'used after being disposed', 'Native sign typography:', 'Embedded sign release:', 'Preview unavailable:')))
    finally:
        print(f'App log: {log.name}')
        if proc.poll() is None:
            proc.terminate()
            proc.wait(timeout=10)
        log.close()
        capture_tools.cleanup()
    return checks.failures


if __name__ == '__main__':
    raise SystemExit(main())
