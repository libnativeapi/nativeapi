#!/usr/bin/env python3
"""Real Flutter/AppKit quit (#65): app-menu AXPress and OS normal termination.

No mouse or keyboard input by default; policy is set through the debug VM service.
With --pointer, all pointer presses stay on the fixture's own window.
The menu and OS driver validates both PID and executable before targeting it.
Build the title-bar example with --target tools/gui/application_quit_flutter.dart
(an absolute path), then run this script. The runner is the ordinary Flutter one.
"""
from pathlib import Path
import argparse
import subprocess
import tempfile
import time

from common import example
from guiapp import Abort, Checks, assert_idle, pause


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', choices=('menu', 'system'))
    parser.add_argument('--pointer', action='store_true', help='click policy buttons instead of using the VM service')
    args = parser.parse_args()
    checks = Checks()
    with tempfile.TemporaryDirectory(prefix='nativeapi-quit-driver-') as temporary:
        driver = str(Path(temporary) / 'quit-driver')
        subprocess.run(['clang', '-fobjc-arc', '-framework', 'Cocoa', '-framework',
                        'ApplicationServices', str(Path(__file__).with_name(
                            'application_quit_macos_driver.m')), '-o', driver], check=True)
        for source in ((args.source,) if args.source else ('menu', 'system')):
            if args.pointer:
                assert_idle()
            app = example('window_title_bar_example', args=['-ApplePersistenceIgnoreState', 'YES'])
            try:
                app.launch()

                def counts():
                    view = next(v for v in app.views() if v.has('NativeAPI quit verification'))
                    values = {}
                    for label in ('Native requests', 'Host requests'):
                        line = next(t for t, _ in view.texts if t.startswith(label + ': '))
                        values[label] = int(line.split(': ')[1])
                    return values['Native requests'], values['Host requests']

                def select(title):
                    if not args.pointer:
                        vm = app.probe
                        isolate = next(i['id'] for i in vm._call('getVM')['result']['isolates']
                                       if i['name'] == 'main')
                        mode = {'Native cancels': 'native', 'Flutter cancels': 'flutter',
                                'Allow quit': 'allow'}[title]
                        result = vm._call('ext.nativeapi.quitMode', isolateId=isolate, mode=mode)
                        if 'error' in result:
                            raise Abort(f'cannot set fixture mode: {result}')
                        pause(0.3)
                        return
                    _, frame = app.windows()[0]
                    view = next(v for v in app.views() if v.has(title))
                    app.click(app.to_screen(frame, view, view.center(title)))
                    pause(0.3)

                def request():
                    subprocess.run([driver, str(app.proc.pid), app.executable, source],
                                   check=True, timeout=10)

                def expect(native, host):
                    deadline = time.monotonic() + 8
                    actual = None
                    while time.monotonic() < deadline and app.proc.poll() is None:
                        actual = counts()
                        if actual == (native, host):
                            break
                        pause(0.2)
                    # Give the asynchronous veto/host reply time to finish too.
                    pause(0.5)
                    alive = app.proc.poll() is None and len(app.windows()) == 1
                    checks.check(f'{source}: veto keeps the app and window alive', alive)
                    if not alive:
                        raise Abort('quit bypassed cancellation')
                    checks.check(f'{source}: exactly {native} native and {host} host requests',
                                 counts() == (native, host), counts())

                checks.check(f'{source}: counters start at zero', counts() == (0, 0))
                request()
                expect(1, 0)
                request()
                expect(2, 0)
                select('Flutter cancels')
                request()
                expect(3, 1)
                request()
                expect(4, 2)
                select('Allow quit')
                request()
                try:
                    app.proc.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    pass
                checks.check(f'{source}: approval exits normally', app.proc.poll() == 0,
                             app.proc.poll())
                output = app.output()
                checks.check(f'{source}: final native request is delivered once',
                             output.count('NATIVE_QUIT 5 allow=true') == 1)
                checks.check(f'{source}: final Flutter approval is delivered once',
                             output.count('HOST_QUIT 3 allow=true') == 1)
            except (Abort, subprocess.SubprocessError, StopIteration) as error:
                checks.check(f'{source}: scenario finishes', False, error)
            finally:
                app.quit()
                print(f'{source} log: {app.log_path}', flush=True)
    print(f'{checks.failures} failure(s)', flush=True)
    return int(bool(checks.failures))


if __name__ == '__main__':
    raise SystemExit(main())
