#!/usr/bin/env python3
"""Verify GTK gutter alpha and the actual Wayland input regions sent to the compositor.

Run inside gui-test's isolated gnome_headless.py desktop. No input is sent.
"""
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def main():
    assert (os.environ.get('GUI_TEST_HEADLESS_GNOME') == '1' or
            os.environ.get('GUI_TEST_PRIVATE_WAYLAND') == '1'), 'Use an isolated test desktop'
    output = ROOT / 'core/build/window-manager-linux-reports' / str(time.time_ns())
    output.mkdir(parents=True, exist_ok=True)
    exe = output / 'gtk-reports'
    flags = shlex.split(subprocess.check_output(
        ['pkg-config', '--cflags', '--libs', 'gtk+-3.0', 'wayland-client', 'x11', 'xi'], text=True))
    subprocess.run(['c++', '-std=c++17', '-g', '-I'+str(ROOT / 'core/src'),
                    str(ROOT / 'tools/gui/window_manager_linux_reports.cpp'),
                    str(ROOT / 'core/build/src/libnativeapi.a'), *flags, '-ldl', '-pthread',
                    '-o', str(exe)], check=True)
    env = dict(os.environ, WAYLAND_DEBUG='client')
    with (output / 'stdout.log').open('w') as out, (output / 'wayland.log').open('w') as err:
        subprocess.run([str(exe)], stdout=out, stderr=err, env=env, check=True, timeout=30)
    reports = [json.loads(line.removeprefix('REPORT ')) for line in
               (output / 'stdout.log').read_text().splitlines() if line.startswith('REPORT ')]
    regions = {}
    inputs = {}
    phases = {}
    for line in (output / 'wayland.log').read_text().splitlines():
        create = re.search(r'create_region\(new id wl_region[@#](\d+)\)', line)
        if create:
            regions[int(create[1])] = []
        rect = re.search(r'wl_region[@#](\d+)\.(add|subtract)\((-?\d+), (-?\d+), (\d+), (\d+)\)', line)
        if rect:
            regions.setdefault(int(rect[1]), []).append((rect[2], *map(int, rect.groups()[2:])))
        shape = re.search(r'wl_surface[@#](\d+)\.set_input_region\((?:wl_region[@#](\d+)|(nil|null))\)', line)
        if shape:
            inputs[int(shape[1])] = list(regions[int(shape[2])]) if shape[2] else None
        if line.startswith('REGION_PHASE '):
            phases[line.split()[1]] = dict(inputs)
    failures = []
    passed = 0

    def check(ok, label, detail):
        nonlocal passed
        print(('PASS ' if ok else 'FAIL ') + label + ': ' + str(detail), flush=True)
        if ok:
            passed += 1
        else:
            failures.append(label)

    for report in reports:
        phase = report['phase']
        x, y, width, height = report['content']
        if phase == 'resized':
            check((width, height) == (560, 360), 'GTK applied the requested resize before assertions', report)
        snapshot = phases[phase]
        assert report['surface'] in snapshot, f'No Wayland input region captured: {report}'
        region = snapshot[report['surface']]

        def contains(px, py):
            accepted = region is None
            for operation, left, top, w, h in region or []:
                if left <= px < left+w and top <= py < top+h:
                    accepted = operation == 'add'
            return accepted

        corners = [(x+8, y+8), (x+width-9, y+8), (x+8, y+height-9), (x+width-9, y+height-9)]
        check(all(contains(*point) for point in corners), f'#595 {phase} all content corners accept input', region)
        if phase != 'normal':
            check(report['themeAlpha'] == 0, f'#594 {phase} GTK background is transparent', report['themeAlpha'])
            samples = [a for a in report['gutterAlpha'] if a >= 0]
            if samples:
                check(all(a < 255 for a in samples), f'#594 {phase} gutter is not opaque', report)
                check(not contains(x-20, y+height//2), f'#595 {phase} gutter passes input through', region)
        elif os.environ.get('NATIVEAPI_REPORT_HIGH_PRIORITY_THEME'):
            check(report['themeAlpha'] == 1, '#594 normal GTK background restores the theme', report['themeAlpha'])
        report['submittedInputRegion'] = region
    result = {'passed': passed, 'failed': len(failures), 'failures': failures, 'reports': reports}
    (output / 'result.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({'passed': passed, 'failed': len(failures), 'directory': str(output)}), flush=True)
    raise SystemExit(bool(failures))


if __name__ == '__main__':
    main()
