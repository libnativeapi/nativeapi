#!/usr/bin/env python3
"""GUI test (macOS) of window_manager's widgets and close handling, on the fixture
window_manager_flutter.dart built into the window_manager example:

- WindowCaption: its title strip drags the window, its maximize button maximizes and
  restores, its minimize button minimizes (the fixture restores after 2 s), and with
  setPreventClose(true) its close button reports onWindowClose and keeps the window.
- With the native title bar, the window's own close button (found through
  Accessibility, pressed with a real click) is prevented the same way.
- popUpWindowMenu() answers without a menu on macOS.
- setIgnoreMouseEvents(true, forward: true): hovering the window still reaches it.
  Nothing is pressed while the window ignores the mouse: a click would land in
  whatever window lies below.
- destroy() closes the window past setPreventClose and the app exits.

    tools/gui/flutter_window_manager_gui_test.py [--build] [--keep-open]

Set WINDOW_MANAGER_DIR to the window_manager checkout (default:
bindings/dart/window_manager). It takes over the mouse for about a minute.
"""

import os
import re
import shutil
import subprocess
import sys

from common import WORKSPACE
from guiapp import Abort, Checks, GuiApp, assert_idle, flutter_executable, pause

PACKAGE = os.environ.get('WINDOW_MANAGER_DIR',
                         os.path.join(WORKSPACE, 'bindings', 'dart', 'window_manager'))
PROJECT = os.path.join(PACKAGE, 'example')
NAME = 'window_manager_example'
FIXTURE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'window_manager_flutter.dart')
CAPTION = 'WM GUI caption'
BUTTON = 46  # WindowCaptionButton's minimum width


def build():
    target = os.path.join(PROJECT, 'lib', 'gui_test_main.dart')
    shutil.copy(FIXTURE, target)
    try:
        subprocess.run(['flutter', 'build', 'macos', '--debug', '-t', 'lib/gui_test_main.dart'],
                       cwd=PROJECT, check=True)
    finally:
        os.remove(target)


def close_button(pid):
    """Centre of the first window's close button (AX), in screen points."""
    script = ('tell application "System Events" to tell (first process whose unix id is '
              f'{pid}) to get {{position, size}} of (first button of window 1 whose '
              'subrole is "AXCloseButton")')
    out = subprocess.run(['osascript', '-e', script], capture_output=True, text=True,
                         check=True).stdout
    x, y, w, h = (int(v) for v in re.findall(r'-?\d+', out))
    return x + w // 2, y + h // 2


def main():
    if '--build' in sys.argv:
        build()
    assert_idle()
    app = GuiApp(flutter_executable(PROJECT, name=NAME),
                 args=['-ApplePersistenceIgnoreState', 'YES'])
    app.keep_open = '--keep-open' in sys.argv
    checks = Checks()
    app.launch(min_windows=1)
    try:
        def look():
            title, frame = app.windows()[0]
            view = next(v for v in app.views() if v.has('TARGET'))
            return frame, view

        def value(name):
            _, view = look()
            line = next(t for t, _ in view.texts if t.startswith(name + ' '))
            return line[len(name) + 1:]

        def press(label, settle=1.2):
            frame, view = look()
            app.click(tuple(round(c) for c in app.to_screen(frame, view, view.center(label))))
            pause(settle)

        def caption_button(index):
            """index 0: close, 1: maximize / restore, 2: minimize."""
            frame, view = look()
            _, cy = view.center(CAPTION)
            x = view.size[0] - BUTTON / 2 - index * BUTTON - 2
            return tuple(round(c) for c in app.to_screen(frame, view, (x, cy)))

        frame, _ = look()
        checks.check('the fixture starts with WindowCaption', look()[1].has(CAPTION))

        # DragToMoveArea inside WindowCaption.
        frame, view = look()
        start = tuple(round(c) for c in app.to_screen(frame, view, view.center(CAPTION)))
        target = (start[0] + 140, start[1] + 90)
        app.drag(start, (start[0] + 40, start[1] + 20, 400), (*target, 800))
        pause(1.5)
        moved, _ = look()
        checks.near('dragging the caption moved the window',
                    moved[:2], (frame[0] + 140, frame[1] + 90), tolerance=8)

        # Maximize, then restore, with the caption's own button.
        before, _ = look()
        app.click(caption_button(1))
        pause(1.8)
        maximized, _ = look()
        checks.check('the caption maximize button enlarged the window',
                     maximized[2] > before[2] + 50 and maximized[3] > before[3] + 50,
                     (before, maximized))
        checks.check('and isMaximized says so', value('maximized') == 'true', value('maximized'))
        app.click(caption_button(1))
        pause(1.8)
        restored, _ = look()
        checks.near('pressing it again restored the size', restored[2:], before[2:], tolerance=4)
        checks.check('and the legacy events saw maximize, unmaximize',
                     'maximize' in value('events') and 'unmaximize' in value('events'),
                     value('events'))

        # Minimize; the fixture restores after 2 s.
        app.click(caption_button(2))
        pause(4)
        events = value('events')
        checks.check('the caption minimize button minimized and the window came back',
                     'minimize' in events and 'restore' in events, events)

        # Prevented close through the caption's close button.
        press('Prevent close on')
        app.click(caption_button(0))
        pause(1.5)
        checks.check('with preventClose, the caption close button reports onWindowClose',
                     value('closeReports') == '1', value('closeReports'))
        checks.check('and the window stays', app.proc.poll() is None and len(app.windows()) == 1)

        # Prevented close through the window's own close button.
        press('Native title bar', settle=1.5)
        x, y = close_button(app.proc.pid)
        app.click((x, y))
        pause(1.5)
        checks.check('with preventClose, the native close button reports onWindowClose',
                     value('closeReports') == '2', value('closeReports'))
        checks.check('and the window stays', app.proc.poll() is None and len(app.windows()) == 1)
        press('Hidden title bar', settle=1.5)

        # popUpWindowMenu: no system menu on macOS, but no failure either.
        press('Menu in 1.5s', settle=2.5)
        checks.check('popUpWindowMenu() leaves the app running', app.proc.poll() is None)

        # Ignore the mouse, forwarding moves; no presses while it lasts.
        hovers = int(value('hovers'))
        press('Ignore mouse 8s', settle=0.8)
        checks.check('setIgnoreMouseEvents is on', value('ignoring') == 'true')
        frame, view = look()
        cx, cy = view.center('TARGET')
        for dx, dy in ((-80, -30), (60, 20), (-20, 40), (90, -10)):
            app.move(tuple(round(c) for c in app.to_screen(frame, view, (cx + dx, cy + dy))), ms=300)
        pause(0.5)
        checks.check('with forward, hovering still reaches the window',
                     int(value('hovers')) > hovers, (hovers, value('hovers')))
        pause(8)
        checks.check('and the window takes the mouse back after 8 s', value('ignoring') == 'false')
        clicks = int(value('clicks'))
        press('TARGET')
        checks.check('a click lands again', int(value('clicks')) == clicks + 1, value('clicks'))

        # destroy() past preventClose.
        checks.check('preventClose is still on', value('preventClose') == 'true')
        frame, view = look()
        app.click(tuple(round(c) for c in app.to_screen(frame, view, view.center('destroy()'))))
        try:
            app.proc.wait(10)
        except subprocess.TimeoutExpired:
            pass
        checks.check('destroy() closed the window and the app exited', app.proc.poll() is not None,
                     app.proc.poll())
    except Abort as e:
        checks.check('ran to the end', False, e)
    finally:
        app.quit()
    print(f'{checks.failures} failure(s); app log: {app.log_path}')
    return 1 if checks.failures else 0


if __name__ == '__main__':
    sys.exit(main())
