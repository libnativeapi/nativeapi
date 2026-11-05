#!/usr/bin/env python3
"""Test NSH's native integration through app-scoped Accessibility actions.

No mouse or keyboard events. Requires an unlocked macOS desktop and Accessibility
permission for the terminal. Each AX action targets a child of the launched PID.

    python3 tools/gui/nsh_examples_test_macos.py --build
"""

import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / ".agents/skills/gui-test/scripts/macos"))
from guiapp import GuiApp, assert_idle  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", action="store_true", help="Build the examples first")
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("This desktop test requires macOS")
    if args.build:
        subprocess.run([
            "cargo", "build", "--manifest-path", "examples/nsh_shared/Cargo.toml",
            "--workspace", "--locked",
        ], cwd=ROOT, check=True)

    with tempfile.TemporaryDirectory(prefix="nsh-ax-") as temporary:
        driver = Path(temporary) / "nsh-ax"
        subprocess.run([
            "clang", "-fobjc-arc", "-O2", "-framework", "AppKit", "-framework",
            "ApplicationServices", str(Path(__file__).with_name("nsh_accessibility_macos.m")),
            "-o", str(driver),
        ], check=True)
        assert_idle()
        checks = 0

        def ax(app, *arguments):
            return subprocess.check_output([
                str(driver), str(app.proc.pid), *arguments,
            ], text=True, timeout=8)

        def expect(app, expected):
            nonlocal checks
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                dump = ax(app, "dump")
                if expected in dump:
                    checks += 1
                    print("PASS", expected, flush=True)
                    return
                time.sleep(0.1)
            raise AssertionError(f"Missing {expected!r}:\n{dump}")

        def press(app, label):
            ax(app, "press", label)

        for name in ("tray_icon", "shortcut", "message_dialog"):
            app = GuiApp(str(ROOT / f"examples/nsh_shared/target/debug/nsh_{name}_example"))
            try:
                app.launch(flutter=False)
                if name == "tray_icon":
                    expect(app, "Tray: visible")
                    press(app, "Toggle tray visibility")
                    expect(app, "Tray: hidden")
                    press(app, "Toggle tray visibility")
                    expect(app, "Tray: visible")
                elif name == "shortcut":
                    press(app, "Register")
                    expect(app, "Registered Ctrl+Shift+Space.")
                    press(app, "Invoke callback")
                    expect(app, "Activations: 1")
                    press(app, "Toggle enabled")
                    expect(app, "Shortcut disabled.")
                    press(app, "Invoke callback")
                    expect(app, "Shortcut is disabled.")
                    expect(app, "Activations: 1")
                    press(app, "Toggle enabled")
                    expect(app, "Shortcut enabled.")
                    press(app, "Invoke callback")
                    expect(app, "Activations: 2")
                    press(app, "Unregister")
                    expect(app, "Shortcut unregistered.")
                    press(app, "Invoke callback")
                    expect(app, "Register a shortcut first.")
                    # Close with a live registration to exercise shutdown cleanup.
                    press(app, "Register")
                    expect(app, "Registered Ctrl+Shift+Space.")
                else:
                    press(app, "Count in NSH")
                    expect(app, "NSH count: 1")
                    press(app, "Show confirmation")
                    expect(app, "confirmation buttons and results are unavailable.")
                    press(app, "Show information")
                    expect(app, "Hello from nativeapi")
                    ax(app, "default")
                    expect(app, "Dialog dismissed. This platform does not report a button result.")
                ax(app, "close")
                assert app.proc.wait(timeout=8) == 0, f"{name} failed to exit cleanly"
                checks += 1
                print("PASS graceful shutdown:", name, flush=True)
            finally:
                app.quit()
                print("App log:", app.log_path, flush=True)

        print(json.dumps({"checks": checks, "result": "passed"}), flush=True)


if __name__ == "__main__":
    main()
