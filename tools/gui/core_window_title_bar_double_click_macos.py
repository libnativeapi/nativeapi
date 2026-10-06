#!/usr/bin/env python3
"""Guarded real title-bar double clicks; process-local preferences, no global edits."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.agents/skills/gui-test/scripts/macos'))
from guiapp import Abort, Checks, GuiApp, assert_idle, pause  # noqa: E402

TITLE = 'NativeAPI Title Bar Double Click'


def caption(frame):
    x, y, width, _ = frame
    return x + width // 2, y + 12


def main():
    assert_idle()
    checks = Checks()
    executable = ROOT / 'core/build/tests/window_title_bar_double_click_gui_macos_test'
    for action in ('Maximize', 'None', 'Fill', 'Minimize'):
        app = GuiApp(str(executable), args=[action])
        try:
            app.launch(flutter=False)
            before = app.window(TITLE)
            app.double_click(caption(before))
            pause(1.2)
            output = app.output()
            states = re.findall(r'STATE max=(\d) min=(\d) fullscreen=(\d)', output)
            if not states:
                raise Abort('fixture did not report its native state')
            maximized, minimized, fullscreen = map(int, states[-1])
            checks.check(f'{action} keeps window out of full screen', not fullscreen)
            if action == 'Maximize':
                after = app.window(TITLE)
                checks.check('automatic hidden caption zooms native window', maximized and after != before)
                app.double_click(caption(after))
                pause(1.2)
                checks.check('second double click restores native window', 'STATE max=0 min=0' in app.output()[len(output):])
                checks.check('restored frame matches original', app.window(TITLE) == before)
            elif action == 'Minimize':
                checks.check('Minimize preference miniaturizes native window', minimized)
            elif action == 'None':
                checks.check('None preference preserves geometry and state', app.window(TITLE) == before and not maximized and not minimized)
            elif 'FILL_SUPPORTED 1' in output:
                checks.check('native Fill changes geometry', app.window(TITLE) != before and not minimized)
            else:
                checks.check('unavailable Fill does not fall back to zoom', app.window(TITLE) == before and not maximized)
        except Abort as error:
            checks.check(f'{action} ran to completion', False, str(error))
            break
        finally:
            app.quit()
            print(f'{action} fixture log: {app.log_path}')
    return int(bool(checks.failures))


if __name__ == '__main__':
    sys.exit(main())
