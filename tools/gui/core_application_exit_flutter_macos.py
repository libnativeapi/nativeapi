#!/usr/bin/env python3
"""Run the nativeapi/Flutter exit regression without windows or input."""

import argparse
import plistlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run(command, *, cwd, timeout):
    subprocess.run(command, cwd=cwd, check=True, timeout=timeout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--flutter", default=shutil.which("flutter"))
    parser.add_argument(
        "--framework-dir",
        type=Path,
        help="Directory containing the matching debug FlutterMacOS.framework",
    )
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("this regression requires macOS")
    if not args.flutter:
        parser.error("Flutter was not found; specify --flutter")
    framework_dir = args.framework_dir
    if framework_dir is None:
        engine_dir = Path(args.flutter).resolve().parent / "cache/artifacts/engine"
        frameworks = sorted(
            framework
            for framework in engine_dir.glob(
                "darwin-*/FlutterMacOS.xcframework/macos-*/FlutterMacOS.framework"
            )
            if framework.relative_to(engine_dir).parts[0]
            in {"darwin-x64", "darwin-arm64"}
        )
        if not frameworks:
            parser.error(
                "debug FlutterMacOS.framework was not found; specify --framework-dir"
            )
        framework_dir = frameworks[0].parent
    framework_dir = framework_dir.resolve()
    if not (framework_dir / "FlutterMacOS.framework").is_dir():
        parser.error("--framework-dir must contain FlutterMacOS.framework")
    workspace = Path(__file__).resolve().parents[2]
    build_dir = workspace / "core/build"
    run(
        [
            "cmake",
            "-S",
            str(workspace / "core"),
            "-B",
            str(build_dir),
            "-DBUILD_TESTING=ON",
            "-U",
            "NATIVEAPI_TEST_FLUTTER_FRAMEWORK",
            f"-DNATIVEAPI_FLUTTER_FRAMEWORK_DIR={framework_dir}",
        ],
        cwd=workspace,
        timeout=120,
    )
    run(
        [
            "cmake",
            "--build",
            str(build_dir),
            "--target",
            "application_flutter_engine_macos_test",
            "-j6",
        ],
        cwd=workspace,
        timeout=300,
    )
    with tempfile.TemporaryDirectory(prefix="nativeapi-flutter-exit-") as temporary:
        bundle = Path(temporary) / "exit.bundle"
        contents = bundle / "Contents"
        contents.mkdir(parents=True)
        (contents / "Info.plist").write_bytes(
            plistlib.dumps(
                {
                    "CFBundleIdentifier": "dev.nativeapi.test-exit",
                    "CFBundleName": "NativeapiExitTest",
                    "CFBundlePackageType": "BNDL",
                    "CFBundleVersion": "1",
                }
            )
        )
        run(
            [
                args.flutter,
                "build",
                "bundle",
                "--debug",
                "--target-platform",
                "darwin",
                "--target",
                str(workspace / "tools/gui/application_exit_flutter.dart"),
                "--asset-dir",
                str(contents / "Resources/flutter_assets"),
            ],
            cwd=workspace / "bindings/dart/nativeapi_flutter",
            timeout=300,
        )
        run(
            [
                str(build_dir / "tests/application_flutter_engine_macos_test"),
                str(bundle),
            ],
            cwd=workspace,
            timeout=30,
        )


if __name__ == "__main__":
    main()
