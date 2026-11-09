#!/usr/bin/env python3
"""Plays gpui_tray_icon_example for a screen recording (macOS): animated tray icons
rendered by GPUI + DazzUI, frame by frame — the GPUI counterpart of
flutter_tray_icon_demo.py.

The example window is a fixed 800x600 at the screen centre; GPUI has no UI probe, so
clicks go to layout coordinates: sidebar 176 px, the header 52 px, the preview strip
112 px, gallery tiles in two rows of four below it.

    tools/gui/gpui_tray_icon_demo.py [--record [OUT.mp4]] [--build] [--countdown N]
                                     [--pace F] [--keep-open]

With --record the main display is captured (cursor and clicks included) and saved as an
H.264 MP4 that fits within 1920x1200 — by default to
tools/gui/output/gpui_tray_icon_demo-macos.mp4. No keyboard input, no audio.

Built on the gui-test and record-demo skills (.agents/skills). Needs the example built
in debug mode (`--build` does it), and for the terminal that runs this, in System
Settings > Privacy & Security: Accessibility and, with --record, Screen Recording.
"""

import argparse
import sys
import time

from common import build_gpui_example, gpui_example, output_path
from guiapp import Abort, GuiApp, assert_idle, pause
from recorder import Recorder

NAME = 'tray_icon_example'
TITLE = 'Tray Icon'
# Speed of the whole scenario (--pace): motions and pauses are scaled by it; pauses that
# wait for something to appear keep a floor.
PACE = 0.5

# The example's layout, in content points from the window's top-left. The window is a
# fixed 800x600; the sidebar is 176 px and the header 52 px on both sides; the preview
# strip is 112 px. The gallery starts under it: a section heading, then two rows of
# four tiles ~70 px tall; the rows live in a scrollable pane, so this is the
# un-scrolled top.
SIDEBAR = 88            # centre of the sidebar's first group card
HEADER_Y = 26           # centre of the main column's header row
# Tab content: section heading ~ y 166+34, gallery rows at ~ y 226 and ~ y 302.
GALLERY = [(row, col) for row in range(2) for col in range(4)]


def scaled(ms, floor=120):
    return max(floor, int(ms * PACE))


def beat(seconds):
    pause(max(seconds * PACE, 0.7 if seconds >= 1 else 0.2))


class PacedApp(GuiApp):
    def move(self, point, ms=600):
        super().move(point, scaled(ms))

    def click(self, point, ms=450):
        super().click(point, scaled(ms))


def play(app, keep_open):
    app.keep_open = keep_open
    app.launch(flutter=False, min_windows=1)
    try:
        # The window is centred; drag it to the left so the menu bar (with the tray
        # icons) and the window are comfortably in one view.
        x, y, w, h = app.window(TITLE)
        app.drag((x + w / 2, y + 16), (x + w / 2 - 200, y + 130, 350), (x - 300, y + 180, 500))
        x, y, w, h = app.window(TITLE)

        origin = (x, y)

        def at(px, py):
            return (origin[0] + px, origin[1] + py)

        # Measured on a 2x screenshot of the built example: content coordinates of
        # everything the demo presses. Rows: header (tabs), sidebar cards, gallery
        # tiles, and the chips below the gallery.
        P = {
            'tab-properties': (568, 26),
            'tab-checklist': (652, 26),
            'tab-content': (486, 26),
            'window-to-icon': (595, 166 + 26),   # state block's right side
            'refresh': (692, 166 + 26),
            # Gallery tiles: two rows of four over the content width (800-176=624,
            # padding 10, gaps 6): tile ~146 wide, ~70 tall.
            'tile-0': (242, 240), 'tile-1': (398, 240), 'tile-2': (554, 240), 'tile-3': (702, 240),
            'tile-4': (242, 316), 'tile-5': (398, 316), 'tile-6': (554, 316), 'tile-7': (702, 316),
            # Presets / frames rows below the gallery (~ y 400 and ~ y 448), chips flow
            # left to right from x ~ 266.
            'preset-download': (292, 404), 'preset-recording': (400, 404),
            'preset-syncing': (500, 404), 'preset-three': (600, 404),
            'still-asset': (280, 452), 'still-drawn': (336, 452), 'still-base64': (404, 452),
            'rate-10': (280, 500), 'rate-60': (420, 500),
            'scale-1x': (282, 548), 'scale-3x': (346, 548),
            'color-auto': (276, 580), 'color-blue': (336, 580), 'color-red': (486, 580),
        }

        def press(key, hold=1.0):
            point = at(*P[key])
            app.click(point)
            beat(hold)

        # 0. The tray icon next to its window, wherever this platform keeps it.
        press('tab-properties', hold=0.8)
        press('window-to-icon', hold=1.5)
        press('tab-content', hold=0.8)

        # 1. The gallery: every tile plays in the tray and in the preview at once.
        for tile in ('tile-0', 'tile-4', 'tile-5', 'tile-6'):
            press(tile, hold=3.0)
        press('tile-7', hold=4.0)  # the counter badge, drawn by the CPU

        # 2. Rate and resolution, on a shape where it shows.
        press('tile-3', hold=2.0)          # Progress
        press('rate-10', hold=3.0)
        press('rate-60', hold=4.0)
        press('preset-download', hold=6.0)  # the title follows the ring: "42%"
        press('preset-recording', hold=5.0)
        press('preset-syncing', hold=4.0)

        # 3. Three icons, three animations, all running.
        press('preset-three', hold=7.0)

        # 4. Colour and resolution, live.
        press('color-blue', hold=2.5)
        press('color-red', hold=2.5)
        press('scale-3x', hold=2.5)
        press('scale-1x', hold=1.0)

        # End on the proof: what the example verified by itself.
        press('tab-checklist', hold=4.0)
    finally:
        app.quit()


def main():
    global PACE
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--record', metavar='OUT.mp4', nargs='?', const='',
                        help='record the main display (default: tools/gui/output/'
                             '<script name>-macos.mp4)')
    parser.add_argument('--build', action='store_true', help='build the example first')
    parser.add_argument('--countdown', type=int, default=5)
    parser.add_argument('--pace', type=float, default=PACE,
                        help=f'speed factor for motions and pauses (default {PACE})')
    parser.add_argument('--keep-open', action='store_true',
                        help='leave the example running at the end')
    args = parser.parse_args()
    PACE = args.pace

    if args.build:
        build_gpui_example(NAME)
    for i in range(args.countdown, 0, -1):
        print(f'Starting in {i}… (hands off the mouse)', flush=True)
        time.sleep(1)
    try:
        assert_idle()
    except Abort as e:
        raise SystemExit(f'Stopped: {e}')

    recorder = Recorder(args.record or output_path(__file__, 'macos')) \
        if args.record is not None else None
    if recorder:
        recorder.start()
        print('● recording', flush=True)
    failed = None
    try:
        play(gpui_example(NAME, args=['-ApplePersistenceIgnoreState', 'YES']), args.keep_open)
        beat(1.0)
    except Abort as e:
        failed = e
    finally:
        if recorder:
            recorder.stop()
    if failed:
        raise SystemExit(f'Stopped: {failed}')
    if recorder:
        recorder.convert()


if __name__ == '__main__':
    sys.exit(main())
