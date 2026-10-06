#!/usr/bin/env python3
"""Build a real Flutter app and test secondary close/exit on private displays.

Run on Linux with Flutter, clang, GTK3 development files, Xvfb, Weston and
dbus-run-session installed. No desktop input or existing display is used.
The saved work directory includes the generated app, native assets and logs.
Flutter-only controls run the same lifecycle without nativeapi native assets;
SDK warnings remain visible, and additional nativeapi diagnostics fail the test.
"""
import argparse
from collections import Counter
import os
from pathlib import Path
import shutil
import re
import signal
import subprocess
import tempfile
import time


def run(command, log, *, cwd=None, env=None, timeout=900):
    print("RUN:", " ".join(map(str, command)), flush=True)
    with log.open("w") as output:
        process = subprocess.Popen(
            list(map(str, command)), cwd=cwd, env=env, stdout=output,
            stderr=subprocess.STDOUT, start_new_session=True,
        )
        try:
            result = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            # Only this fixture's fresh process group, including its private
            # display, is terminated. Existing desktop processes are untouched.
            os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
            raise RuntimeError(f"timeout {timeout}s: {log}\n{log.read_text()[-8000:]}")
    if result:
        raise RuntimeError(f"exit {result}: {log}\n{log.read_text()[-8000:]}")
    return log.read_text()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workspace", type=Path,
                        default=Path(__file__).resolve().parents[2])
    parser.add_argument("--flutter", default="flutter")
    parser.add_argument("--work-dir", type=Path)
    parser.add_argument("--fatal-warnings", action="store_true",
                        help="Also make Flutter/GTK SDK diagnostics fatal")
    args = parser.parse_args()
    work = (args.work_dir or Path(tempfile.mkdtemp(prefix="nativeapi-flutter-exit-"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    print(f"Artifacts: {work}", flush=True)
    source = args.workspace.resolve()
    if work == source or source in work.parents:
        raise ValueError("Use a separate scratch directory outside the source workspace")
    ignore = shutil.ignore_patterns(".git", ".dart_tool", "build", "._*", "cxx_impl")
    shutil.copytree(source / "core/src", work / "core/src", ignore=ignore, dirs_exist_ok=True)
    for package in ("cnativeapi", "nativeapi", "nativeapi_flutter"):
        package_source = source / "bindings/dart" / package
        package_target = work / "bindings/dart" / package
        package_target.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(package_source / "pubspec.yaml", package_target / "pubspec.yaml")
        # Copy runtime/build inputs only. Package examples belong to the source
        # repository's larger pub workspace and are not part of this fixture.
        for directory in ("lib", "hook", "src"):
            if (package_source / directory).exists():
                shutil.copytree(package_source / directory, package_target / directory,
                                ignore=ignore, dirs_exist_ok=True)
    app = work / "app"
    control = work / "control"
    env = os.environ.copy()
    env["FLUTTER_SUPPRESS_ANALYTICS"] = "true"
    run([args.flutter, "--suppress-analytics", "--no-version-check", "create", "--no-pub",
         "--platforms=linux", "--empty", "--org=dev.nativeapi.test",
         "--project-name=nativeapi_linux_exit", app], work / "create.log", env=env)
    run([args.flutter, "--suppress-analytics", "--no-version-check", "create", "--no-pub",
         "--platforms=linux", "--empty", "--org=dev.nativeapi.test",
         "--project-name=nativeapi_linux_exit_control", control], work / "control-create.log", env=env)
    (work / "pubspec.yaml").write_text(
        "name: nativeapi_linux_exit_workspace\nenvironment:\n  sdk: ^3.13.0\n"
        "workspace:\n  - bindings/dart/cnativeapi\n  - bindings/dart/nativeapi\n"
        "  - bindings/dart/nativeapi_flutter\n  - app\n  - control\n")
    (app / "pubspec.yaml").write_text(
        "name: nativeapi_linux_exit\npublish_to: none\nresolution: workspace\n"
        "environment:\n  sdk: ^3.13.0\ndependencies:\n"
        "  flutter:\n    sdk: flutter\n  nativeapi: ^0.4.1\n"
        "  nativeapi_flutter: ^0.4.1\nflutter:\n  uses-material-design: true\n")
    shutil.copyfile(source / "tools/gui/application_exit_flutter_linux.dart", app / "lib/main.dart")
    (control / "pubspec.yaml").write_text(
        "name: nativeapi_linux_exit_control\npublish_to: none\nresolution: workspace\n"
        "environment:\n  sdk: ^3.13.0\ndependencies:\n"
        "  flutter:\n    sdk: flutter\n")
    shutil.copyfile(source / "tools/gui/application_exit_flutter_linux_control.dart",
                    control / "lib/main.dart")
    # The generated empty-app lint configuration references flutter_lints, which
    # is unnecessary for this fixture. Analyze the real entry point explicitly.
    (app / "analysis_options.yaml").write_text("analyzer:\n  errors:\n    avoid_print: ignore\n")
    shutil.copyfile(app / "analysis_options.yaml", control / "analysis_options.yaml")
    run([args.flutter, "--suppress-analytics", "--no-version-check", "pub", "get"], work / "pub.log", cwd=app, env=env)
    run([args.flutter, "--suppress-analytics", "--no-version-check", "analyze", "lib/main.dart"],
        work / "analyze.log", cwd=app, env=env)
    run([args.flutter, "--suppress-analytics", "--no-version-check", "build", "linux", "--debug"],
        work / "build.log", cwd=app, env=env)
    run([args.flutter, "--suppress-analytics", "--no-version-check", "analyze", "lib/main.dart"],
        work / "control-analyze.log", cwd=control, env=env)
    run([args.flutter, "--suppress-analytics", "--no-version-check", "build", "linux", "--debug"],
        work / "control-build.log", cwd=control, env=env)
    bundle = app / "build/linux/x64/debug/bundle"
    executable = bundle / "nativeapi_linux_exit"
    control_bundle = control / "build/linux/x64/debug/bundle"
    control_executable = control_bundle / "nativeapi_linux_exit_control"
    if list(control_bundle.rglob("*cnativeapi*")):
        raise RuntimeError("Flutter-only control unexpectedly includes nativeapi native assets")
    runtime = work / "runtime"
    runtime.mkdir(mode=0o700, exist_ok=True)
    runtime.chmod(0o700)
    test_env = env.copy()
    for key in ("DISPLAY", "WAYLAND_DISPLAY", "XAUTHORITY", "DBUS_SESSION_BUS_ADDRESS",
                "NO_AT_BRIDGE", "G_DEBUG"):
        test_env.pop(key, None)
    test_env.update(XDG_RUNTIME_DIR=str(runtime), LIBGL_ALWAYS_SOFTWARE="1")
    if args.fatal_warnings:
        test_env["G_DEBUG"] = "fatal-warnings"

    def diagnostics(output):
        # Keep every warning in the log and fail if nativeapi adds diagnostics
        # beyond the same two-window lifecycle in a Flutter-only application.
        messages = []
        for line in output.splitlines():
            if re.search(r"(?:WARNING|CRITICAL) \*\*:", line):
                messages.append(re.split(r"\*\*: (?:\d\d:\d\d:\d\d\.\d+: )?", line, 1)[-1])
        return Counter(messages)

    def verify(backend, launcher, extra):
        backend_env = dict(test_env, GDK_BACKEND=backend, **extra)
        control_log = work / f"{backend}-control.log"
        control_output = run([*launcher, control_executable], control_log,
                             cwd=control_bundle, env=backend_env, timeout=45)
        if ("FLUTTER_LINUX_CONTROL_EXIT_READY" not in control_output or
                "FLUTTER_LINUX_EXIT_FAIL" in control_output or
                control_output.count("CONTROL_PASS:") != 2):
            raise RuntimeError(f"Flutter-only control failed: {control_log}\n{control_output}")
        for mode, options in (("listeners", []), ("live-manager", ["--live-manager"])):
            log = work / (f"{backend}.log" if mode == "listeners" else f"{backend}-{mode}.log")
            output = run([*launcher, executable, *options], log, cwd=bundle, env=backend_env, timeout=45)
            if "FLUTTER_LINUX_EXIT_READY" not in output or "FLUTTER_LINUX_EXIT_FAIL" in output:
                raise RuntimeError(f"Flutter regression failed: {log}\n{output}")
            if output.count("PASS:") != 18:
                raise RuntimeError(f"Expected 18 checks: {log}\n{output}")
            if re.search(r"The implicit view cannot be removed|(?:epoxy|GLX).*assert",
                         output, re.IGNORECASE):
                raise RuntimeError(f"Original engine teardown failure recurred: {log}\n{output}")
            if mode == "live-manager" and "FLUTTER_LINUX_LIVE_MANAGER_EXIT_READY" not in output:
                raise RuntimeError(f"Live-manager mode did not run: {log}")
            extra_diagnostics = diagnostics(output) - diagnostics(control_output)
            if extra_diagnostics:
                raise RuntimeError(f"nativeapi adds diagnostics: {extra_diagnostics}\n{log}")
            if diagnostics(control_output):
                print(f"SDK diagnostics also occur in Flutter-only control: {dict(diagnostics(control_output))}", flush=True)
            print(f"PASS: {backend}/{mode}: 18 checks and real GTK process exit 0", flush=True)
    verify("x11", ["dbus-run-session", "--", "xvfb-run", "-a", "-s",
                   "-screen 0 1280x960x24"], {})
    weston_log = work / "weston.log"
    with (work / "weston-launch.log").open("w") as output:
        compositor = subprocess.Popen(
            ["weston", "--backend=headless-backend.so", "--socket=nativeapi-flutter-exit",
             "--no-config", "--idle-time=0", "--width=1280", "--height=960",
             f"--log={weston_log}"], env=test_env, stdout=output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 10
            while not (runtime / "nativeapi-flutter-exit").exists():
                if compositor.poll() is not None or time.monotonic() >= deadline:
                    raise RuntimeError(f"Weston did not start: {weston_log}")
                time.sleep(0.05)
            verify("wayland", ["dbus-run-session", "--"],
                   {"WAYLAND_DISPLAY": "nativeapi-flutter-exit"})
        finally:
            if compositor.poll() is None:
                compositor.terminate()
            try:
                compositor.wait(timeout=10)
            except subprocess.TimeoutExpired:
                compositor.kill()
                compositor.wait()
    print("ALL PASS: real Flutter secondary close and normal exit on X11 + Wayland", flush=True)


if __name__ == "__main__":
    main()
