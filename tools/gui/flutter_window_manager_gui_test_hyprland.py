#!/usr/bin/env python3
"""GUI test (Linux, Hyprland) of window_manager's widgets and close handling, on the
fixture window_manager_flutter.dart built into the window_manager example (the macOS
and Windows twins are flutter_window_manager_gui_test.py / .ps1).

A temporary window rule floats the fixture, since Hyprland tiles windows by default
and a tiled window can neither be dragged nor resized. Input is real: the pointer is
moved with hyprctl and pressed through zwlr_virtual_pointer (`wlpointer` from the kit);
every press is checked to land inside the fixture's own window, which must be the
focused one. No keyboard input.

Checked here: WindowCaption's title strip drags the window; VirtualWindowFrame's
corner resizes it (skipped while Hyprland reports the window maximized, which it does
for every mapped toplevel); with setPreventClose(true) the caption's close button, the GTK
header bar's close button and the compositor's close request each report
onWindowClose and keep the window; popUpWindowMenu() keeps the app running;
setIgnoreMouseEvents passes the pointer through for 8 s and then takes it back;
destroy() closes the window past setPreventClose and the app exits. Not checked,
because Hyprland does not do them: the caption's maximize button (the default config
suppresses every maximize request) and minimize (Wayland has no minimized state a
client can enter).

    flutter_window_manager_gui_test_hyprland.py   (env WINDOW_MANAGER_BUNDLE: the built
                                                   bundle directory)
"""

import glob
import json
import os
import re
import shutil
import subprocess
import sys
import time

HOME = os.path.expanduser('~')
SCRATCH = os.environ.get('REMOTE_SCRATCH') or os.path.join(HOME, 'tmp', 'claude')
KIT = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, KIT)
sys.path.insert(0, SCRATCH)
from uiprobe import App as Probe  # noqa: E402


def probe(log_path):
    """The app's views, or none while its VM service is not up yet."""
    try:
        app = Probe(log_path)
        return app.views() if app.vm_url() else []
    except Exception:  # noqa: BLE001
        return []

NAME = 'window_manager_example'
CLASS = 'org.leanflutter.plugins.window_manager_example'
CAPTION = 'WM GUI caption'
BUTTON = 46
HYPR_CONF = os.path.join(HOME, '.config', 'hypr', 'hyprland.lua')
RULE_MODULE = 'window_manager_gui_test'
RULE_FILE = os.path.join(HOME, '.config', 'hypr', RULE_MODULE + '.lua')
MARK = '-- tools/gui/flutter_window_manager_gui_test_hyprland.py (temporary, removed by the test)'
RULE = f'''{MARK}
hl.window_rule({{
  match = {{ class = "^{CLASS.replace('.', '\\\\.')}$" }},
  float = true,
  no_anim = true,
}})
'''


class Checks:
    def __init__(self):
        self.failed = 0

    def ok(self, cond, label, detail=''):
        if not cond:
            self.failed += 1
        print(f'{"PASS" if cond else "FAIL"} {label}' + (f' ({detail})' if detail != '' else ''),
              flush=True)
        return cond

    def skip(self, label, why):
        print(f'SKIP {label} ({why})', flush=True)


def sh(*args, check=True, timeout=20):
    return subprocess.run(list(args), capture_output=True, text=True, check=check, timeout=timeout)


def hypr_env():
    os.environ.setdefault('XDG_RUNTIME_DIR', f'/run/user/{os.getuid()}')
    if not os.environ.get('HYPRLAND_INSTANCE_SIGNATURE'):
        dirs = sorted(glob.glob(os.path.join(os.environ['XDG_RUNTIME_DIR'], 'hypr', '*')),
                      key=os.path.getmtime)
        if not dirs:
            sys.exit('no Hyprland instance under $XDG_RUNTIME_DIR/hypr')
        os.environ['HYPRLAND_INSTANCE_SIGNATURE'] = os.path.basename(dirs[-1])
    if not os.environ.get('WAYLAND_DISPLAY'):
        socks = [s for s in sorted(glob.glob(os.path.join(os.environ['XDG_RUNTIME_DIR'],
                                                          'wayland-*')))
                 if not s.endswith('.lock')]
        if socks:
            os.environ['WAYLAND_DISPLAY'] = os.path.basename(socks[-1])


def hyprctl(*args, as_json=True):
    out = sh('hyprctl', *(('-j',) if as_json else ()), *args).stdout
    return json.loads(out) if as_json else out.strip()


def dispatch(lua):
    out = hyprctl('dispatch', lua, as_json=False)
    if out != 'ok':
        raise RuntimeError(f'dispatch {lua}: {out}')


def ours():
    return [c for c in hyprctl('clients') if c.get('mapped', True)
            and (c.get('class') == CLASS or c.get('initialClass') == CLASS)]


def cursor():
    m = re.match(r'(-?\d+),\s*(-?\d+)', hyprctl('cursorpos', as_json=False))
    return int(m.group(1)), int(m.group(2))


def move_cursor(x, y, settle=0.15):
    dispatch(f'hl.dsp.cursor.move({{ x = {int(x)}, y = {int(y)} }})')
    time.sleep(settle)


def glide(start, end, steps=12, ms=500):
    for i in range(1, steps + 1):
        t = i / steps
        move_cursor(start[0] + (end[0] - start[0]) * t, start[1] + (end[1] - start[1]) * t,
                    settle=ms / steps / 1000)


def layout_extent():
    """The output layout in logical pixels, which absolute virtual pointer motion spans."""
    mons = hyprctl('monitors')
    return (max(round(m['x'] + m['width'] / m['scale']) for m in mons),
            max(round(m['y'] + m['height'] / m['scale']) for m in mons))


def wait_for(pred, seconds, step=0.2):
    end = time.time() + seconds
    while time.time() < end:
        value = pred()
        if value:
            return value
        time.sleep(step)
    return pred()


def assert_idle():
    a = cursor()
    time.sleep(1.0)
    if cursor() != a:
        sys.exit('ABORT the mouse is moving: someone is using this machine')


def install_rule(backup):
    shutil.copy2(HYPR_CONF, backup)
    with open(RULE_FILE, 'w') as f:
        f.write(RULE)
    with open(HYPR_CONF, 'a') as f:
        f.write(f'\n{MARK}\nrequire("hypr.{RULE_MODULE}")\n')
    hyprctl('reload', as_json=False)
    time.sleep(1.0)


def restore_rule(backup):
    if os.path.exists(backup):
        shutil.copy2(backup, HYPR_CONF)
        os.remove(backup)
    if os.path.exists(RULE_FILE):
        os.remove(RULE_FILE)
    hyprctl('reload', as_json=False)
    time.sleep(0.8)


def build_wlpointer():
    exe = os.path.join(SCRATCH, 'wlpointer')
    if os.path.exists(exe):
        return exe
    xml = os.path.join(SCRATCH, 'wlr-virtual-pointer-unstable-v1.xml')
    sh('wayland-scanner', 'client-header', xml, os.path.join(SCRATCH, 'wlr-virtual-pointer-client.h'))
    sh('wayland-scanner', 'private-code', xml, os.path.join(SCRATCH, 'wlr-virtual-pointer.c'))
    sh('cc', '-O1', '-I', SCRATCH, '-o', exe, os.path.join(SCRATCH, 'wlpointer.c'),
       os.path.join(SCRATCH, 'wlr-virtual-pointer.c'), '-lwayland-client', timeout=120)
    return exe


def main():
    hypr_env()
    bundle = os.environ.get('WINDOW_MANAGER_BUNDLE')
    if not bundle:
        sys.exit('set WINDOW_MANAGER_BUNDLE to the built bundle directory')
    wlpointer = build_wlpointer()
    assert_idle()
    checks = Checks()
    backup = os.path.join(SCRATCH, 'hyprland.lua.window_manager_gui_test.bak')
    install_rule(backup)
    log_path = os.path.join(SCRATCH, 'window_manager_gui_test.log')
    log = open(log_path, 'w')
    proc = subprocess.Popen([os.path.join(bundle, NAME)], stdout=log, stderr=subprocess.STDOUT)

    def client():
        found = ours()
        if not found:
            raise RuntimeError('the fixture window is gone')
        return found[0]

    def view():
        for v in probe(log_path):
            if v.has('TARGET'):
                return v
        raise RuntimeError('no Flutter view shows TARGET')

    def value(name):
        line = next(t for t, _ in view().texts if t.startswith(name + ' '))
        return line[len(name) + 1:]

    def origin():
        """Screen point of the view's (0, 0): the client's top left, below a header bar."""
        c = client()
        v = view()
        return c['at'][0] + c['size'][0] - v.size[0], c['at'][1] + c['size'][1] - v.size[1]

    def screen(point):
        ox, oy = origin()
        return round(ox + point[0]), round(oy + point[1])

    def guard(point):
        c = client()
        x, y = point
        inside = (c['at'][0] <= x < c['at'][0] + c['size'][0]
                  and c['at'][1] <= y < c['at'][1] + c['size'][1])
        if not inside:
            raise RuntimeError(f'{point} is outside the fixture window {c["at"]} {c["size"]}')
        active = (hyprctl('activewindow') or {}).get('address')
        if active != c['address']:
            raise RuntimeError('the fixture window is not the focused one')

    def click(point, settle=1.2):
        glide(cursor(), point, steps=8, ms=350)
        guard(point)
        sh(wlpointer, 'click')
        time.sleep(settle)

    def press(label, settle=1.2):
        click(screen(view().center(label)), settle)

    extent = layout_extent()

    def pointer_glide(a, b, steps, ms):
        # Real pointer motion: a compositor move or resize grab follows it, while a
        # hyprctl cursor warp only places the cursor.
        for i in range(1, steps + 1):
            t = i / steps
            sh(wlpointer, 'move', str(round(a[0] + (b[0] - a[0]) * t)),
               str(round(a[1] + (b[1] - a[1]) * t)), str(extent[0]), str(extent[1]))
            time.sleep(ms / steps / 1000)

    def drag(start, end):
        glide(cursor(), start, steps=8, ms=350)
        guard(start)
        sh(wlpointer, 'press')
        try:
            pointer_glide(start, (start[0] + 20, start[1] + 10), steps=5, ms=300)
            pointer_glide((start[0] + 20, start[1] + 10), end, steps=20, ms=800)
        finally:
            sh(wlpointer, 'release')
        time.sleep(1.5)

    def caption_button(index):
        v = view()
        _, cy = v.center(CAPTION)
        return screen((v.size[0] - BUTTON / 2 - index * BUTTON - 1, cy))

    try:
        if not wait_for(lambda: ours() and any(v.has('TARGET') for v in probe(log_path)), 40):
            raise RuntimeError(f'the fixture did not come up; see {log_path}')
        time.sleep(1.5)
        c = client()
        dispatch(f'hl.dsp.focus({{ window = "address:{c["address"]}" }})')
        time.sleep(0.6)
        checks.ok(c.get('floating'), 'the rule floats the fixture')
        checks.ok(view().has(CAPTION), 'the fixture starts with WindowCaption')

        # DragToMoveArea inside WindowCaption.
        before = client()
        start = screen(view().center(CAPTION))
        drag(start, (start[0] + 140, start[1] + 90))
        after = client()
        moved = (after['at'][0] - before['at'][0], after['at'][1] - before['at'][1])
        checks.ok(abs(moved[0] - 140) <= 10 and abs(moved[1] - 90) <= 10,
                  'dragging the caption moved the window', moved)

        # VirtualWindowFrame's bottom right corner (DragToResizeArea). Hyprland reports
        # every mapped toplevel maximized, floating or not, and VirtualWindowFrame drops
        # its resize edges for a maximized window (GDK would refuse the resize anyway).
        if value('maximized') == 'true':
            checks.skip('the corner resizes the window',
                        'Hyprland reports the floating window as maximized')
        else:
            before = client()
            v = view()
            corner = screen((v.size[0] - 3, v.size[1] - 3))
            drag(corner, (corner[0] + 80, corner[1] + 60))
            after = client()
            grown = (after['size'][0] - before['size'][0], after['size'][1] - before['size'][1])
            checks.ok(abs(grown[0] - 80) <= 12 and abs(grown[1] - 60) <= 12,
                      'the corner resized the window', grown)

        checks.skip('the caption maximize button',
                    "Hyprland's default config suppresses maximize requests")
        checks.skip('the caption minimize button', 'Wayland has no minimized state')

        # Prevented closes.
        press('Prevent close on')
        click(caption_button(0), settle=1.5)
        checks.ok(value('closeReports') == '1',
                  'with preventClose, the caption close button reports onWindowClose',
                  value('closeReports'))
        checks.ok(proc.poll() is None and ours(), 'and the window stays')

        dispatch(f'hl.dsp.window.close({{ window = "address:{client()["address"]}" }})')
        time.sleep(1.5)
        checks.ok(value('closeReports') == '2',
                  "with preventClose, the compositor's close request reports onWindowClose",
                  value('closeReports'))
        checks.ok(proc.poll() is None and ours(), 'and the window stays')

        press('Native title bar', settle=1.5)
        c = client()
        ox, oy = origin()
        header = oy - c['at'][1]
        if header < 20:
            checks.skip('the header bar close button', f'no header bar ({header} px)')
        else:
            # GTK's close button is the header bar's last item, square, at its right end.
            point = (round(c['at'][0] + c['size'][0] - header / 2), round(c['at'][1] + header / 2))
            click(point, settle=1.5)
            checks.ok(value('closeReports') == '3',
                      "with preventClose, the header bar's close button reports onWindowClose",
                      value('closeReports'))
            checks.ok(proc.poll() is None and ours(), 'and the window stays')
        press('Hidden title bar', settle=1.5)

        # popUpWindowMenu: whether Hyprland shows a menu or not, the app carries on.
        press('Menu in 1.5s', settle=2.5)
        checks.ok(proc.poll() is None, 'popUpWindowMenu() leaves the app running')

        # Ignore the mouse; no presses while it lasts.
        press('Ignore mouse 8s', settle=0.8)
        checks.ok(value('ignoring') == 'true', 'setIgnoreMouseEvents is on')
        hovers = int(value('hovers'))
        cx, cy = view().center('TARGET')
        for dx, dy in ((-80, -30), (60, 20), (-20, 40), (90, -10)):
            move_cursor(*screen((cx + dx, cy + dy)), settle=0.3)
        now = int(value('hovers'))
        print(f'INFO hovers while ignoring: {hovers} -> {now} (Wayland has no forwarding)',
              flush=True)
        time.sleep(8)
        checks.ok(value('ignoring') == 'false', 'the window takes the mouse back after 8 s')
        clicks = int(value('clicks'))
        press('TARGET')
        checks.ok(int(value('clicks')) == clicks + 1, 'a click lands again', value('clicks'))

        # destroy() past preventClose.
        checks.ok(value('preventClose') == 'true', 'preventClose is still on')
        press('destroy()', settle=0.2)
        try:
            proc.wait(10)
        except subprocess.TimeoutExpired:
            pass
        checks.ok(proc.poll() is not None, 'destroy() closed the window and the app exited',
                  proc.poll())
    except Exception as e:  # noqa: BLE001
        checks.ok(False, 'ran to the end', e)
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(5)
            except subprocess.TimeoutExpired:
                proc.kill()
        restore_rule(backup)
    print('OK' if not checks.failed else f'FAILED ({checks.failed})', flush=True)
    return 1 if checks.failed else 0


if __name__ == '__main__':
    sys.exit(main())
