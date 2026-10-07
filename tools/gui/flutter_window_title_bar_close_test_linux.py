#!/usr/bin/env python3
"""GUI test (Linux, GNOME) of cancellable window close (#65) in
flutter_window_title_bar_example: with Closing set to Keep open the example
cancels every WindowCloseRequestedEvent, so a real click on the header bar's
close button leaves the window open and counts one cancelled request; with
Allow the same click closes the window and the app exits with code 0. The
Linux twin of flutter_window_title_bar_close_test.ps1 (Windows).

    .agents/skills/remote-hosts/scripts/remote.sh linux desktop \
        tools/gui/flutter_window_title_bar_close_test_linux.py 200

The close button is GTK's own (the Flutter runner uses a header bar under
GNOME), found by its accessible name through AT-SPI. The system menu is
gnome-shell's, outside the app's windows, so it is not driven here. The
example is expected built in debug in the host's checkout ($REMOTE_WORKSPACE),
or set TITLE_BAR_EXAMPLE_EXE. The app runs under GDK_BACKEND=x11.
"""

import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))  # the flat kit on a remote host

from guiapp import Abort, Checks, GuiApp, assert_idle, flutter_executable, pause  # noqa: E402

import gi  # noqa: E402

gi.require_version('Atspi', '2.0')
from gi.repository import Atspi  # noqa: E402

NAME = 'window_title_bar_example'


def executable():
    if os.environ.get('TITLE_BAR_EXAMPLE_EXE'):
        return os.environ['TITLE_BAR_EXAMPLE_EXE']
    workspace = os.environ.get('REMOTE_WORKSPACE') or os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    return flutter_executable(
        os.path.join(workspace, 'examples', 'flutter_window_title_bar_example'), NAME)


def close_button(pid, timeout=10):
    """Screen center of the header bar's close button, through AT-SPI."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        desktop = Atspi.get_desktop(0)
        for i in range(desktop.get_child_count()):
            app = desktop.get_child_at_index(i)
            if app is None or app.get_process_id() != pid:
                continue
            stack = [app]
            while stack:
                node = stack.pop()
                if (node.get_role() == Atspi.Role.PUSH_BUTTON and
                        (node.get_name() or '').lower() == 'close'):
                    r = node.get_extents(Atspi.CoordType.SCREEN)
                    if r.width > 0 and r.height > 0:
                        return r.x + r.width / 2, r.y + r.height / 2
                stack.extend(node.get_child_at_index(j) for j in range(node.get_child_count()))
        time.sleep(0.5)
    raise Abort('no accessible close button: is the AT-SPI bus running?')


def accessibles(pid):
    """Every accessible of the app, depth first."""
    desktop = Atspi.get_desktop(0)
    for i in range(desktop.get_child_count()):
        app = desktop.get_child_at_index(i)
        if app is None or app.get_process_id() != pid:
            continue
        stack = [app]
        while stack:
            node = stack.pop()
            yield node
            stack.extend(node.get_child_at_index(j) for j in range(node.get_child_count()))


def view_origin(app, timeout=10):
    """Screen position of the Flutter view.

    Under GNOME the runner puts a GTK header bar above the view, inside the X11
    client area, so the view does not start where the harness's to_screen()
    assumes. Its accessible, the one exactly the view's size, tells.
    """
    width, height = app.views()[0].size
    deadline = time.time() + timeout
    while time.time() < deadline:
        for node in accessibles(app.proc.pid):
            r = node.get_extents(Atspi.CoordType.SCREEN)
            if abs(r.width - width) <= 1 and abs(r.height - height) <= 1:
                return r.x, r.y
        time.sleep(0.5)
    raise Abort(f'no accessible of the view size {width}x{height}')


def text_center(app, text):
    x, y = app.views()[0].center(text)
    ox, oy = view_origin(app)
    return ox + x, oy + y


def has_text(app, text):
    return any(t == text for t, _ in app.views()[0].texts)


def cancelled(app):
    view = app.views()[0]
    for text, _ in view.texts:
        if text.startswith('Cancelled closes: '):
            return int(text.split(':')[1])
    raise Abort('the cancelled-closes counter is missing')


def main():
    assert_idle()
    checks = Checks()
    app = GuiApp(executable())
    app.launch()
    try:
        app.click(text_center(app, 'Keep open'))
        pause(0.8)
        checks.check('Keep open is chosen',
                     has_text(app, 'Close requests are cancelled; try the × or the menu'))
        checks.check('the counter starts at 0', cancelled(app) == 0)

        app.click(close_button(app.proc.pid))
        pause(1.5)
        checks.check('with Keep open, the close button leaves the window open',
                     app.proc.poll() is None and len(app.windows()) == 1)
        checks.check('and the example cancelled one request', cancelled(app) == 1)

        app.click(close_button(app.proc.pid))
        pause(1.5)
        checks.check('a second click is cancelled too', app.proc.poll() is None and
                     cancelled(app) == 2)

        app.click(text_center(app, 'Allow'))
        pause(0.8)
        checks.check('Allow is chosen', has_text(app, 'Closing the window closes it'))
        app.click(close_button(app.proc.pid))
        try:
            code = app.proc.wait(10)
        except Exception:
            code = None
        checks.check('with Allow, the close button closes the window and the app exits',
                     code is not None, f'exit code {code}')
        checks.check('with exit code 0', code == 0, f'exit code {code}')
    except Abort as error:
        checks.check('the scenario ran to the end', False, str(error))
    finally:
        app.quit()
    print('FAILED' if checks.failures else 'OK', flush=True)
    return 1 if checks.failures else 0


if __name__ == '__main__':
    sys.exit(main())
