#!/usr/bin/env python3
"""Build and run the headless JS event ownership fixture on macOS."""

import argparse
import shutil
import sys
from pathlib import Path

from js_event_delivery import run, verify


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--node", default=shutil.which("node"))
    args = parser.parse_args()
    if sys.platform != "darwin" or not args.node:
        parser.error("requires macOS and Node")
    workspace = Path(__file__).resolve().parents[2]
    package = workspace / "bindings/js"
    run(["npx", "cmake-js", "configure"], package)
    run(["cmake", "-S", ".", "-B", "build", "-DNATIVEAPI_JS_BUILD_TESTS=ON"], package)
    run(["cmake", "--build", "build", "--config", "Release"], package)
    verify(args.node, workspace / "tools/tests/js_event_delivery_runtime.mjs",
           package / "build/Release/nativeapi.node", workspace)


if __name__ == "__main__":
    main()
