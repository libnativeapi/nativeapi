#!/usr/bin/env python3
"""Check native startup fullscreen and minimize requests on an isolated Wayland desktop.

Run inside gnome_headless.py. No input is sent. The minimize assertions use the
real xdg_toplevel protocol because Wayland has no minimized-state notification.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('--output-dir', type=Path, default=Path('build/window_startup_protocol'))
    args = parser.parse_args()
    if not os.environ.get('WAYLAND_DISPLAY'):
        parser.error('A Wayland desktop is required')
    args.output_dir.mkdir(parents=True, exist_ok=True)
    for mode in ('normal', 'fullscreen', 'cancel-fullscreen', 'minimize', 'cancel-minimize', 'show-minimize',
                 'fullscreen-then-maximize'):
        result = subprocess.run([str(args.executable.resolve()), mode],
                                env={**os.environ, 'GDK_BACKEND': 'wayland', 'WAYLAND_DEBUG': 'client'},
                                capture_output=True, text=True, timeout=20)
        (args.output_dir / f'{mode}.protocol.log').write_text(result.stderr)
        print(result.stdout, end='', flush=True)
        if result.returncode != 0:
            raise RuntimeError(f'{mode}: exit {result.returncode}; protocol logs: {args.output_dir}')
        requests = re.findall(r'xdg_toplevel@\d+\.set_minimized\(', result.stderr)
        if mode == 'minimize' and len(requests) != 1:
            raise AssertionError(f'Expected one minimized request, found {len(requests)}; logs: {args.output_dir}')
        if mode in ('cancel-minimize', 'show-minimize') and requests:
            raise AssertionError(f'Cancelled minimize was submitted; logs: {args.output_dir}')
        if mode == 'fullscreen':
            monitor = re.search(r'MONITOR (\d+) (\d+)', result.stdout)
            first_geometry = re.search(
                r'xdg_surface@\d+\.set_window_geometry\(-?\d+, -?\d+, (\d+), (\d+)\)',
                result.stderr)
            if not monitor or not first_geometry or monitor.groups() != first_geometry.groups():
                raise AssertionError(f'First fullscreen frame has normal geometry; logs: {args.output_dir}')
        if mode == 'cancel-fullscreen' and re.search(r'xdg_toplevel@\d+\.set_fullscreen\(', result.stderr):
            raise AssertionError(f'Cancelled fullscreen was submitted; logs: {args.output_dir}')
    print('PASS native startup state regression')


if __name__ == '__main__':
    main()
