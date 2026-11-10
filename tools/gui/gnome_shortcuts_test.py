#!/usr/bin/env python3
"""Opt-in GNOME Wayland shortcut test using real Mutter keyboard input.

Run in a logged-in, idle GNOME session with the live probe executable as argv[1].
Only Ctrl+Alt+Shift+F9..F12 are sent, only while our own empty GTK window has focus.
The GNOME shortcut list is compared before/after; unrelated settings are not reset.
"""
import os
from pathlib import Path
import subprocess
import sys
import threading
import time

import gi

os.environ["GDK_BACKEND"] = "wayland"
gi.require_version("Gtk", "3.0")
from gi.repository import Gio, GLib, Gtk, Gdk

sys.path.insert(0, str(Path(__file__).resolve().parents[2] /
                       ".agents/skills/gui-test/scripts/linux"))
from xinput import RemoteDesktop, idle_ms


class KeyboardInput(RemoteDesktop):
    def _node_of(self, _cast_session):
        # Subscribe before Start, then wait afterward: Mutter 46 announces the
        # node only once the linked RemoteDesktop session has started.
        def added(_c, _s, _p, _i, _signal, parameters):
            self.node = parameters.unpack()[0]

        self.bus.signal_subscribe(
            "org.gnome.Mutter.ScreenCast", "org.gnome.Mutter.ScreenCast.Stream",
            "PipeWireStreamAdded", self.stream, None,
            Gio.DBusSignalFlags.NONE, added)


def pump():
    for _ in range(100):
        if not Gtk.events_pending():
            break
        Gtk.main_iteration_do(False)
    time.sleep(0.02)


def wait(predicate, seconds=15):
    deadline = time.monotonic() + seconds
    while not predicate() and time.monotonic() < deadline:
        pump()
    assert predicate(), "Timed out waiting for GNOME test state"


def main():
    assert os.environ.get("XDG_SESSION_TYPE") == "wayland"
    idle = idle_ms()
    assert idle is not None and idle > 5000, "Desktop is in use; no input sent"
    settings = Gio.Settings.new("org.gnome.settings-daemon.plugins.media-keys")
    before = settings.get_strv("custom-keybindings")
    # An unplugged NUC needs a temporary virtual monitor before GTK can focus.
    remote = KeyboardInput()
    wait(lambda: remote.node is not None)
    print(f"Mutter reference stream: virtual={remote.virtual} node={remote.node}", flush=True)
    stream = None
    if remote.virtual:
        stream = subprocess.Popen([
            "gst-launch-1.0", "-q", "pipewiresrc", f"path={remote.node}", "!",
            "video/x-raw,width=1280,height=720", "!", "videoconvert", "!", "fakesink"],
            stdout=subprocess.DEVNULL)
    wait(lambda: Gdk.Display.get_default().get_n_monitors() > 0)
    window = Gtk.Window(title="Nativeapi GNOME shortcut test — controlled focus")
    window.set_default_size(540, 180)
    window.add(Gtk.Label(label="Testing four temporary global shortcuts.\n"
                         "This empty window owns keyboard focus during input."))
    window.show_all()
    window.present()
    wait(window.is_active)
    probe = subprocess.Popen([sys.argv[1]], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, text=True, bufsize=1)
    lines = []

    def read():
        for line in probe.stdout:
            lines.append(line.strip())
            print(line.rstrip(), flush=True)

    reader = threading.Thread(target=read, daemon=True)
    reader.start()
    session = remote.session.get_object_path()
    down = []
    try:
        wait(lambda: len([s for s in lines if s.startswith("REGISTERED ")]) == 4
             or any(s.startswith("FAILED ") for s in lines))
        assert not any(s.startswith("FAILED ") for s in lines), lines
        assert len(settings.get_strv("custom-keybindings")) == len(before) + 4
        def notify(code, pressed):
            remote._call(remote.session, "NotifyKeyboardKeycode",
                         GLib.Variant("(ub)", (code, pressed)))

        # gsd-media-keys consumes settings asynchronously after dconf sync.
        until = time.monotonic() + 2
        while time.monotonic() < until:
            pump()
        for index, keycode in enumerate((67, 68, 87, 88), 1):
            assert window.is_active(), "Focus left the controlled test window; no keys sent"
            try:
                for code in (29, 56, 42, keycode):
                    notify(code, True)
                    down.append(code)
                    time.sleep(0.06)
            finally:
                for code in reversed(down):
                    notify(code, False)
                down.clear()
            wait(lambda: f"ACTIVATED {index}" in lines)
            assert window.is_active(), "Global activation unexpectedly changed focus"
        assert [s for s in lines if s.startswith("ACTIVATED ")] == [
            f"ACTIVATED {index}" for index in range(1, 5)]
        print("PASS four real compositor shortcuts while another native Wayland window is focused")
    finally:
        if session:
            for code in reversed(down):
                notify(code, False)
        probe.stdin.write("stop\n")
        probe.stdin.flush()
        probe.wait(timeout=15)
        reader.join(timeout=2)
        wait(lambda: settings.get_strv("custom-keybindings") == before)
        window.destroy()
        if stream:
            stream.terminate()
            stream.wait(timeout=5)
        remote.stop()
        print("PASS clean shutdown restores the original GNOME shortcut list", flush=True)
    assert probe.returncode == 0


if __name__ == "__main__":
    main()
