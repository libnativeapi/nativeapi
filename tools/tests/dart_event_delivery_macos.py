#!/usr/bin/env python3
"""Exercise generated Dart async event callbacks from a native worker thread."""

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run(command, cwd):
    subprocess.run(command, cwd=cwd, check=True, timeout=240)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dart", default=shutil.which("dart"))
    args = parser.parse_args()
    if sys.platform != "darwin" or not args.dart:
        parser.error("requires macOS and Dart")
    workspace = Path(__file__).resolve().parents[2]
    package = workspace / "bindings/dart/nativeapi"
    test = package / "test/event_delivery_runtime.dart"
    run([args.dart, "run", str(test), "--prepare"], package)
    # Dart copies the hook output and changes its install_name. Link that exact
    # runtime asset, otherwise dyld loads a second core with separate singletons.
    library = workspace / ".dart_tool/lib/libcnativeapi.dylib"
    if not library.is_file():
        parser.error("cnativeapi runtime asset was not built")
    with tempfile.TemporaryDirectory(prefix="nativeapi-dart-delivery-") as temporary:
        fixture = Path(temporary) / "event_delivery_fixture.dylib"
        run(
            [
                "clang++",
                "-std=c++17",
                "-x",
                "objective-c++",
                "-dynamiclib",
                "-pthread",
                str(workspace / "tools/tests/dart_event_delivery_fixture.cpp"),
                f"-I{workspace / 'core/src'}",
                "-x",
                "none",
                str(library),
                "-framework",
                "Cocoa",
                "-o",
                str(fixture),
            ],
            workspace,
        )
        run([args.dart, "run", str(test), str(fixture)], package)


if __name__ == "__main__":
    main()
