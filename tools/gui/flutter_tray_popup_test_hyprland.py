#!/usr/bin/env python3
"""GUI test (Linux, Hyprland — the user's Omarchy laptop) of tray_icon_example's popup
mode: a click on the tray icon in the bar shows the window right under the icon, the
window loses the focus and hides, a click while it shows hides it.

On Wayland the app cannot place its window (no global coordinates, no anchoring to the
bar's surface) and the bar (quickshell) passes no icon position with `Activate`. What
makes "under the icon" work is a Hyprland window rule evaluated when the window maps,
with the cursor where it was for the click:

    hl.window_rule({
      match = { class = "^com\\.example\\.tray_icon_example$" },
      float = true, pin = true, no_anim = true,
      move = { "(cursor_x-window_w+20)", "(cursor_y+20)" },   -- right edge at the icon
    })

The example (TRAY_POPUP=1, or Properties → Popup) hides the window on blur and toggles it
from the click, so every show is a fresh map and the rule runs again. This test installs
the rule temporarily (a `require` line appended to ~/.config/hypr/hyprland.lua, a file next
to it, `hyprctl reload`), restores both at the end, and asserts through `hyprctl`:

  1. the window is floating and pinned (rule applied on launch)
  2. focusing another window hides it; the app prints `[popup] hidden (blur)`
  3. the icon is found in the bar (grim before/after the app registers its item), the
     cursor moved onto it (`hl.dsp.cursor.move`) and a real left click posted
     through zwlr_virtual_pointer (`wlpointer`, built from the kit; falls back to the
     item's D-Bus `Activate` when the tool cannot be built): the window maps with its
     right edge 20 px right of the cursor and its top 20 px below it, below the bar,
     focused, floating and pinned
  4. blur hides it again; `Activate` with the cursor in the middle of the screen (the
     keyboard case) maps it there — the rule follows the cursor every time
  5. a click on the icon while the window shows leaves it hidden

    .agents/skills/remote-hosts/scripts/remote.sh omarchy setup
    .agents/skills/remote-hosts/scripts/remote.sh omarchy desktop \
        tools/gui/flutter_tray_popup_test_hyprland.py 240

Needs the example built in debug in $REMOTE_WORKSPACE, its tray item pinned in the bar
(Omarchy's drawer hides new items until pinned: right-click the chevron → Pin), and a
Hyprland with Lua config (0.56+). It moves the mouse for ~20 s; no keyboard input.
"""

import glob
import json
import os
import re
import shutil
import signal
import subprocess
import sys
import time

NAME = 'tray_icon_example'
CLASS = 'com.example.tray_icon_example'
EDGE_GAP = 20  # the rule's offsets
HOME = os.path.expanduser('~')
SCRATCH = os.environ.get('REMOTE_SCRATCH') or os.path.join(HOME, 'tmp', 'claude')
WORKSPACE = os.environ.get('REMOTE_WORKSPACE') or os.path.dirname(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
KIT = os.path.dirname(os.path.abspath(__file__))
HYPR_CONF = os.path.join(HOME, '.config', 'hypr', 'hyprland.lua')
RULE_MODULE = 'tray_popup_test'
RULE_FILE = os.path.join(HOME, '.config', 'hypr', RULE_MODULE + '.lua')
MARK = '-- tools/gui/flutter_tray_popup_test_hyprland.py (temporary, removed by the test)'
RULE = f'''{MARK}
hl.window_rule({{
  match = {{ class = "^{CLASS.replace('.', '\\\\.')}$" }},
  tag = "-default-opacity",
  float = true,
  pin = true,
  no_anim = true,
  move = {{ "(cursor_x-window_w+{EDGE_GAP})", "(cursor_y+{EDGE_GAP})" }},
}})
'''


class Checks:
    def __init__(self):
        self.failed = 0
        self.passed = 0

    def ok(self, cond, label, detail=''):
        if cond:
            self.passed += 1
            print(f'PASS {label}' + (f' ({detail})' if detail else ''), flush=True)
        else:
            self.failed += 1
            print(f'FAIL {label}' + (f' ({detail})' if detail else ''), flush=True)
        return cond


def sh(*args, check=True, timeout=20, **kw):
    return subprocess.run(list(args), capture_output=True, text=True, check=check,
                          timeout=timeout, **kw)


def hypr_env():
    """The desktop runner imports WAYLAND_DISPLAY; hyprctl also wants the instance."""
    os.environ.setdefault('XDG_RUNTIME_DIR', f'/run/user/{os.getuid()}')
    if not os.environ.get('HYPRLAND_INSTANCE_SIGNATURE'):
        dirs = sorted(glob.glob(os.path.join(os.environ['XDG_RUNTIME_DIR'], 'hypr', '*')),
                      key=os.path.getmtime)
        if not dirs:
            sys.exit('no Hyprland instance under $XDG_RUNTIME_DIR/hypr')
        os.environ['HYPRLAND_INSTANCE_SIGNATURE'] = os.path.basename(dirs[-1])
    if not os.environ.get('WAYLAND_DISPLAY'):
        socks = sorted(glob.glob(os.path.join(os.environ['XDG_RUNTIME_DIR'], 'wayland-*')))
        socks = [s for s in socks if not s.endswith('.lock')]
        if socks:
            os.environ['WAYLAND_DISPLAY'] = os.path.basename(socks[-1])


def hyprctl(*args, as_json=True):
    out = sh('hyprctl', *(('-j',) if as_json else ()), *args).stdout
    return json.loads(out) if as_json else out.strip()


def clients():
    return [c for c in hyprctl('clients') if c.get('mapped', True)]


def ours():
    return [c for c in clients()
            if c.get('class') == CLASS or c.get('initialClass') == CLASS]


def others():
    return [c for c in clients()
            if c.get('class') != CLASS and c.get('initialClass') != CLASS]


def cursor():
    m = re.match(r'(-?\d+),\s*(-?\d+)', hyprctl('cursorpos', as_json=False))
    return int(m.group(1)), int(m.group(2))


def active_class():
    return (hyprctl('activewindow') or {}).get('class', '')


def monitor():
    mons = hyprctl('monitors')
    return next((m for m in mons if m.get('focused')), mons[0])


def bar_geometry():
    """The bar's layer surface (namespace containing 'bar'), logical pixels."""
    layers = hyprctl('layers')
    for mon in layers.values():
        for level in mon.get('levels', {}).values():
            for layer in level:
                if 'bar' in layer.get('namespace', ''):
                    return layer['x'], layer['y'], layer['w'], layer['h']
    return None


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
    b = cursor()
    if a != b:
        sys.exit(f'ABORT the mouse is moving ({a} -> {b}): someone is using this machine')


# --- the Hyprland rule -------------------------------------------------------------

def config_errors():
    out = hyprctl('configerrors', as_json=False)
    return '' if (not out or 'no errors' in out.lower()) else out


def install_rule(backup):
    shutil.copy2(HYPR_CONF, backup)
    with open(RULE_FILE, 'w') as f:
        f.write(RULE)
    with open(HYPR_CONF, 'a') as f:
        f.write(f'\n{MARK}\nrequire("hypr.{RULE_MODULE}")\n')
    hyprctl('reload', as_json=False)
    time.sleep(1.0)
    return config_errors()


def restore_rule(backup):
    if os.path.exists(backup):
        shutil.copy2(backup, HYPR_CONF)
    if os.path.exists(RULE_FILE):
        os.remove(RULE_FILE)
    hyprctl('reload', as_json=False)
    time.sleep(0.8)
    return config_errors()


# --- finding the icon in the bar ----------------------------------------------------

def grab_ppm(x, y, w, h, path):
    sh('grim', '-g', f'{x},{y} {w}x{h}', '-t', 'ppm', path, timeout=15)
    with open(path, 'rb') as f:
        data = f.read()
    # P6 <w> <h> <max>\n then raw RGB
    header = []
    pos = 0
    while len(header) < 4:
        while data[pos:pos + 1].isspace():
            pos += 1
        if data[pos:pos + 1] == b'#':
            while data[pos:pos + 1] not in (b'\n', b''):
                pos += 1
            continue
        start = pos
        while not data[pos:pos + 1].isspace():
            pos += 1
        header.append(data[start:pos])
    pos += 1
    width, height = int(header[1]), int(header[2])
    return width, height, data[pos:pos + width * height * 3]


def changed_blobs(before, after, min_x_frac=0.6, threshold=90):
    """Bounding boxes (physical px) of connected regions that differ between two
    captures of the same strip, only right of min_x_frac of its width (keeps the
    clock in the middle out of it)."""
    w, h, a = before
    w2, h2, b = after
    if (w, h) != (w2, h2):
        return []
    x0 = int(w * min_x_frac)
    changed = set()
    for y in range(h):
        row = y * w * 3
        for x in range(x0, w):
            i = row + x * 3
            if abs(a[i] - b[i]) + abs(a[i + 1] - b[i + 1]) + abs(a[i + 2] - b[i + 2]) > threshold:
                changed.add((x, y))
    blobs = []
    while changed:
        seed = changed.pop()
        stack = [seed]
        xs, ys = [seed[0]], [seed[1]]
        while stack:
            x, y = stack.pop()
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1),
                           (x + 2, y), (x - 2, y), (x, y + 2), (x, y - 2)):
                if (nx, ny) in changed:
                    changed.remove((nx, ny))
                    stack.append((nx, ny))
                    xs.append(nx)
                    ys.append(ny)
        blobs.append((min(xs), min(ys), max(xs) - min(xs) + 1, max(ys) - min(ys) + 1, len(xs)))
    return sorted(blobs, key=lambda b: -b[4])


# --- input --------------------------------------------------------------------------

def build_wlpointer():
    exe = os.path.join(SCRATCH, 'wlpointer')
    src = os.path.join(KIT, 'wlpointer.c')
    xml = os.path.join(KIT, 'wlr-virtual-pointer-unstable-v1.xml')
    if os.path.exists(exe) and os.path.getmtime(exe) >= os.path.getmtime(src):
        return exe
    if not (os.path.exists(src) and os.path.exists(xml) and shutil.which('wayland-scanner')):
        return None
    try:
        sh('wayland-scanner', 'client-header', xml,
           os.path.join(SCRATCH, 'wlr-virtual-pointer-client.h'))
        sh('wayland-scanner', 'private-code', xml,
           os.path.join(SCRATCH, 'wlr-virtual-pointer.c'))
        sh('cc', '-O1', '-I', SCRATCH, '-o', exe, src,
           os.path.join(SCRATCH, 'wlr-virtual-pointer.c'), '-lwayland-client', timeout=120)
    except subprocess.CalledProcessError as e:
        print(f'wlpointer build failed: {e.stderr.strip()}', flush=True)
        return None
    return exe


def dispatch(lua):
    """Hyprland 0.56+ with a Lua config takes dispatchers in Lua form
    (`hl.dsp.cursor.move({ x = 1, y = 2 })`), not the old `movecursor 1 2`."""
    out = hyprctl('dispatch', lua, as_json=False)
    if out != 'ok':
        raise RuntimeError(f'dispatch {lua}: {out}')


def move_cursor(x, y):
    dispatch(f'hl.dsp.cursor.move({{ x = {int(x)}, y = {int(y)} }})')
    time.sleep(0.4)
    return cursor()


def sni_items():
    out = sh('busctl', '--user', 'get-property', 'org.kde.StatusNotifierWatcher',
             '/StatusNotifierWatcher', 'org.kde.StatusNotifierWatcher',
             'RegisteredStatusNotifierItems').stdout
    return re.findall(r'org\.kde\.StatusNotifierItem-(\d+)-\d+', out)


def sni_item_name(pid):
    return f'org.kde.StatusNotifierItem-{pid}-1' if str(pid) in sni_items() else None


def wait_for_stale_items_gone(seconds=8):
    """An item of a process that just died lingers in the watcher (and the bar) until
    D-Bus reports the name gone; a capture of the bar taken before that still shows it,
    and the new icon would then not count as a change."""
    def alive(pid):
        return os.path.exists(f'/proc/{pid}')
    wait_for(lambda: all(alive(p) for p in sni_items()), seconds)
    time.sleep(1.0)  # the bar's own repaint


def dbus_activate(service):
    sh('gdbus', 'call', '--session', '--dest', service, '--object-path', '/StatusNotifierItem',
       '--method', 'org.kde.StatusNotifierItem.Activate', '0', '0')


# --- the test -----------------------------------------------------------------------

def main():
    hypr_env()
    checks = Checks()
    exe = os.path.join(WORKSPACE, 'examples', 'flutter_' + NAME, 'build', 'linux', 'x64',
                       'debug', 'bundle', NAME)
    if not os.path.exists(exe):
        sys.exit(f'not built: {exe}')
    assert_idle()
    mon = monitor()
    scale = float(mon.get('scale', 1.0))
    bar = bar_geometry()
    if not bar:
        sys.exit('no bar layer surface found')
    bx, by, bw, bh = bar
    print(f'monitor {mon["name"]} {mon["width"]}x{mon["height"]} scale {scale}; '
          f'bar at {bx},{by} {bw}x{bh}; cursor {cursor()}', flush=True)
    sh('pkill', '-f', 'bundle/' + NAME, check=False)
    time.sleep(0.5)
    wait_for_stale_items_gone()

    backup = os.path.join(SCRATCH, 'hyprland.lua.before-tray-popup-test')
    errors = install_rule(backup)
    checks.ok(errors == '', 'rule installed, config reloads without errors', errors[:200])

    log_path = os.path.join(SCRATCH, 'tray_popup_app.log')
    log = open(log_path, 'w')
    wlpointer = build_wlpointer()
    print(f'wlpointer: {wlpointer or "not available, D-Bus Activate instead of clicks"}', flush=True)

    def log_text():
        with open(log_path) as f:
            return f.read()

    def popup_lines():
        return re.findall(r'\[popup\] (.*)', log_text())

    def blur_by_focusing_other():
        other = others()
        if not other:
            return False
        dispatch(f'hl.dsp.focus({{ window = "address:{other[0]["address"]}" }})')
        return True

    app = None
    helper = None
    try:
        # Something to give the focus to. Opened before the app, so that its own
        # mapping (which takes the focus) is not what blurs the popup.
        if not others():
            helper = subprocess.Popen(['foot'], stdout=subprocess.DEVNULL,
                                      stderr=subprocess.DEVNULL, start_new_session=True)
            wait_for(others, 10)
            time.sleep(0.5)
        checks.ok(bool(others()), 'another window to give the focus to',
                  'helper foot' if helper else others()[0]['class'])

        # A capture of the bar's right part before the app's icon is there.
        before = grab_ppm(bx, by, bw, bh, os.path.join(SCRATCH, 'bar_before.ppm'))

        env = dict(os.environ, TRAY_POPUP='1')
        app = subprocess.Popen([exe], stdout=log, stderr=subprocess.STDOUT, env=env,
                               start_new_session=True)
        first = wait_for(lambda: (ours() or [None])[0], 40)
        if not checks.ok(first is not None, 'window mapped after launch'):
            return
        print(f'window: class {first["class"]!r} initialClass {first.get("initialClass")!r} '
              f'at {first["at"]} size {first["size"]} floating {first["floating"]} '
              f'pinned {first["pinned"]}', flush=True)
        checks.ok(first['floating'] and first['pinned'],
                  'rule applies on launch: floating and pinned')
        service = wait_for(lambda: sni_item_name(app.pid), 20)
        checks.ok(service is not None, 'tray item registered with the StatusNotifierWatcher',
                  service or '')
        time.sleep(2.0)
        after = grab_ppm(bx, by, bw, bh, os.path.join(SCRATCH, 'bar_after.ppm'))
        checks.ok(bool(ours()) and not popup_lines(),
                  'window still showing before anything takes the focus',
                  f'popup lines {popup_lines()}')

        # 2. blur hides
        blur_by_focusing_other()
        gone = wait_for(lambda: not ours(), 4)
        checks.ok(gone, 'losing the focus hides the window',
                  f'popup lines {popup_lines()}')
        checks.ok('hidden (blur)' in popup_lines(), 'app reports [popup] hidden (blur)')

        # 3. find the icon, click it
        blobs = changed_blobs(before, after)
        print(f'changed blobs (physical px, x y w h n): {blobs[:5]}', flush=True)
        icon_blob = next((b for b in blobs if b[2] >= 8 and b[3] >= 8), None)
        if not checks.ok(icon_blob is not None, 'tray icon located in the bar'):
            return
        icx = bx + (icon_blob[0] + icon_blob[2] / 2) / scale
        icy = by + (icon_blob[1] + icon_blob[3] / 2) / scale
        icon = (round(icx), round(icy))
        print(f'icon centre (logical) {icon}', flush=True)
        cur = move_cursor(*icon)
        checks.ok(abs(cur[0] - icon[0]) <= 1 and abs(cur[1] - icon[1]) <= 1,
                  'cursor moved onto the icon', f'{cur}')
        time.sleep(0.4)
        if wlpointer:
            sh(wlpointer, 'click')
            how = 'real click'
        else:
            dbus_activate(service)
            how = 'D-Bus Activate'
        win = wait_for(lambda: (ours() or [None])[0], 5)
        if not checks.ok(win is not None, f'{how} on the icon maps the window',
                         f'popup lines {popup_lines()}'):
            return
        time.sleep(0.8)
        win = (ours() or [win])[0]
        right = win['at'][0] + win['size'][0]
        top = win['at'][1]
        print(f'after click: cursor {cur}, window at {win["at"]} size {win["size"]} '
              f'floating {win["floating"]} pinned {win["pinned"]} active {active_class()!r}',
              flush=True)
        checks.ok(win['floating'] and win['pinned'], 'popup window is floating and pinned')
        checks.ok(abs(right - (cur[0] + EDGE_GAP)) <= 3,
                  'right edge 20 px right of the click', f'right {right}, cursor x {cur[0]}')
        checks.ok(abs(top - (cur[1] + EDGE_GAP)) <= 3,
                  'top 20 px below the click', f'top {top}, cursor y {cur[1]}')
        checks.ok(top >= by + bh, 'window is below the bar', f'top {top}, bar bottom {by + bh}')
        checks.ok(active_class() == CLASS, 'popup window has the focus', active_class())
        checks.ok(popup_lines()[-1:] == ['shown'], 'app reports [popup] shown', str(popup_lines()))

        # 4. blur again, then Activate with the cursor elsewhere: the rule follows it
        blur_by_focusing_other()
        checks.ok(wait_for(lambda: not ours(), 4), 'hidden again on blur')
        mid = move_cursor(mon['width'] // scale // 2, mon['height'] // scale // 2)
        time.sleep(0.6)  # past the example's "same gesture as the blur" window
        dbus_activate(service)
        win = wait_for(lambda: (ours() or [None])[0], 5)
        if checks.ok(win is not None, 'Activate maps the window again'):
            time.sleep(0.8)
            win = (ours() or [win])[0]
            right = win['at'][0] + win['size'][0]
            top = win['at'][1]
            checks.ok(abs(right - (mid[0] + EDGE_GAP)) <= 3 and abs(top - (mid[1] + EDGE_GAP)) <= 3,
                      'rule re-evaluated: window at the new cursor position',
                      f'cursor {mid}, right {right}, top {top}')

        # 5. a click while it shows leaves it hidden
        move_cursor(*icon)
        time.sleep(0.4)
        if wlpointer:
            sh(wlpointer, 'click')
        else:
            dbus_activate(service)
        checks.ok(wait_for(lambda: not ours(), 4), 'a click on the icon while shown hides it',
                  f'popup lines {popup_lines()[-3:]}')
        time.sleep(1.0)
        checks.ok(not ours(), 'and it stays hidden', f'popup lines {popup_lines()[-3:]}')
    finally:
        if app is not None:
            app.send_signal(signal.SIGTERM)
            try:
                app.wait(5)
            except subprocess.TimeoutExpired:
                app.kill()
        if helper is not None:
            helper.kill()  # foot ignores SIGTERM; it is an empty helper terminal
        log.close()
        errors = restore_rule(backup)
        checks.ok(errors == '', 'config restored, reloads without errors', errors[:200])
        with open(HYPR_CONF) as f:
            checks.ok(MARK not in f.read() and not os.path.exists(RULE_FILE),
                      'temporary rule removed from the config')
        print('--- app log (tail)', flush=True)
        try:
            print('\n'.join(log_text().splitlines()[-25:]), flush=True)
        except OSError:
            pass
    print(f'{checks.passed} passed, {checks.failed} failed', flush=True)
    sys.exit(checks.failed)


if __name__ == '__main__':
    main()
