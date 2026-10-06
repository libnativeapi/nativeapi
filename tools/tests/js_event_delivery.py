#!/usr/bin/env python3
"""Run the JS event/ownership regressions with a built fixture on any desktop."""

import argparse
import shutil
import subprocess
from pathlib import Path


def run(command, cwd, **kwargs):
    try:
        return subprocess.run(command, cwd=cwd, check=True, timeout=240, **kwargs)
    except subprocess.CalledProcessError as error:
        if error.stdout:
            print(error.stdout)
        if error.stderr:
            print(error.stderr)
        raise


def verify(node, test, fixture, workspace):
    run([node, str(test), str(fixture)], workspace)
    for mode in ["pending", "queued", "owned", "owned-accepted", "owned-disposed"]:
        result = run(
            [node, str(test), str(fixture), mode], workspace,
            capture_output=True, text=True,
        )
        expected = 1 if mode == "owned-accepted" else 0
        if f"cleanup-outcome={expected} live=0 baseline=0" not in result.stdout:
            raise RuntimeError(f"{mode}: cleanup was not verified: {result.stdout}")
        if mode == "queued" and "queued-callback-started" in result.stdout:
            raise RuntimeError("queued shutdown case invoked JS before cleanup")
        print(f"PASS {mode}: environment cleanup resolves votes and frees handles")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--node", default=shutil.which("node"))
    parser.add_argument("--fixture", required=True, type=Path)
    args = parser.parse_args()
    if not args.node or not args.fixture.is_file():
        parser.error("requires Node and a built test-enabled nativeapi.node")
    workspace = Path(__file__).resolve().parents[2]
    verify(args.node, workspace / "tools/tests/js_event_delivery_runtime.mjs",
           args.fixture.resolve(), workspace)


if __name__ == "__main__":
    main()
