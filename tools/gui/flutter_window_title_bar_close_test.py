#!/usr/bin/env python3
"""GUI test (macOS) of cancellable close requests (#65) from Flutter, the twin of
flutter_window_title_bar_close_test.ps1: with Closing set to Keep open,
flutter_window_title_bar_example cancels every WindowCloseRequestedEvent, so real
clicks on the window's own close button (found through Accessibility) leave it open
and are counted; with Allow, the button closes the window and the app exits with 0.

On macOS 26 the close button sends the private -[NSWindow __close] rather than
performClose:, so this is the test that catches a close path core does not confirm.

    tools/gui/flutter_window_title_bar_close_test.py [--build]

Owner-guarded clicks on the example's own window only, no keyboard. Takes over the
mouse for about 15 s.
"""

import re
import subprocess
import sys

from common import example
from guiapp import Abort, Checks, assert_idle, build_flutter, pause

NAME = 'window_title_bar_example'


def close_button(pid):
    """Centre of the first window's close button (Accessibility), in screen points."""
    script = ('tell application "System Events" to tell (first process whose unix id is '
              f'{pid}) to get {{position, size}} of (first button of window 1 whose '
              'subrole is "AXCloseButton")')
    out = subprocess.run(['osascript', '-e', script], capture_output=True, text=True,
                         check=True).stdout
    x, y, w, h = (int(v) for v in re.findall(r'-?\d+', out))
    return x + w // 2, y + h // 2


def main():
    app = example(NAME, args=['-ApplePersistenceIgnoreState', 'YES'])
    if '--build' in sys.argv:
        build_flutter(app.executable.split('/build/')[0])
    assert_idle()
    checks = Checks()
    app.launch(min_windows=1)
    try:
        def look():
            _, frame = app.windows()[0]
            view = next(v for v in app.views() if v.has('Keep open'))
            return frame, view

        def press(label):
            frame, view = look()
            app.click(tuple(round(c) for c in app.to_screen(frame, view, view.center(label))))
            pause(0.8)

        def cancelled():
            _, view = look()
            line = next(t for t, _ in view.texts if t.startswith('Cancelled closes: '))
            return int(line.split(':')[1])

        press('Keep open')
        checks.check('the counter starts at 0', cancelled() == 0, cancelled())
        for count in (1, 2):
            app.click(close_button(app.proc.pid))
            pause(1.5)
            alive = app.proc.poll() is None and len(app.windows()) == 1
            checks.check(f'with Keep open, close button click {count} leaves the window open',
                         alive)
            if not alive:
                raise Abort('the window closed')
            checks.check(f'and the example cancelled {count} request(s)', cancelled() == count,
                         cancelled())

        press('Allow')
        app.click(close_button(app.proc.pid))
        try:
            app.proc.wait(10)
        except subprocess.TimeoutExpired:
            pass
        checks.check('with Allow, the close button closes the window and the app exits',
                     app.proc.poll() is not None)
        if app.proc.poll() is not None:
            checks.check('with exit code 0', app.proc.returncode == 0, app.proc.returncode)
    except Abort as e:
        checks.check('ran to the end', False, e)
    finally:
        app.quit()
    print(f'{checks.failures} failure(s); app log: {app.log_path}')
    return 1 if checks.failures else 0


if __name__ == '__main__':
    sys.exit(main())
