#!/usr/bin/env python3
"""Exercise the real FlView in an isolated Wayland compositor, without input."""
import json
import os
from pathlib import Path
import re
import subprocess
import time
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[2]


def main():
    assert (os.environ.get('GUI_TEST_HEADLESS_GNOME') == '1' or
            os.environ.get('GUI_TEST_PRIVATE_WAYLAND') == '1'), 'Use an isolated desktop'
    bundle = ROOT / 'bindings/dart/window_manager/example/build/linux-reports/local'
    output = ROOT / 'core/build/window-manager-linux-flutter' / str(time.time_ns())
    output.mkdir(parents=True)
    checks = []
    with (output / 'app.log').open('w') as log:
        proc = subprocess.Popen([str(bundle / 'window_manager_example')], cwd=bundle,
                                stdout=log, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 40
            while time.monotonic() < deadline:
                text = (output / 'app.log').read_text()
                match = re.search(r'http://127\.0\.0\.1:\d+/[^/\s]+=/', text)
                if match and 'READY linux-shadow' in text:
                    break
                if proc.poll() is not None:
                    raise RuntimeError(text)
                time.sleep(.1)
            else:
                raise TimeoutError(text)
            base = match[0]

            def request(method, **params):
                with urllib.request.urlopen(base + method + '?' + urllib.parse.urlencode(params),
                                            timeout=10) as reply:
                    data = json.load(reply)
                if 'error' in data:
                    raise RuntimeError(data)
                return data['result']

            isolate = next(i['id'] for i in request('getVM')['isolates'] if i['name'] == 'main')
            for action in ('state', 'resize', 'shadow-off', 'shadow-on', 'maximize', 'restore', 'normal'):
                request('ext.nativeapi.linuxReport', isolateId=isolate, action=action)
                time.sleep(1)
                state = request('ext.nativeapi.linuxReport', isolateId=isolate, action='state')
                ok = state['nativeSize'] == state['flutterSize'] and min(state['flutterSize']) > 100
                if action == 'resize':
                    ok &= state['nativeSize'] == [560, 360]
                checks.append({'action': action, 'passed': ok, **state})
                print(('PASS ' if ok else 'FAIL ') + action + ' ' + json.dumps(state), flush=True)
        finally:
            if proc.poll() is None:
                proc.terminate()
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait()
    (output / 'result.json').write_text(json.dumps(checks, indent=2) + '\n')
    print('RESULT', output, flush=True)
    raise SystemExit(any(not c['passed'] for c in checks))


if __name__ == '__main__':
    main()
