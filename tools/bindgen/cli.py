import argparse
import json
import sys
from pathlib import Path

from .config import load_config
from .ir.serializer import dump_ir_json
from .parser import parse_headers
from .normalizer import normalize_translation_unit
from .codegen.generator import generate_bindings


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(prog="bindgen")
    parser.add_argument("--config", required=True, help="Path to bindgen.yaml")
    parser.add_argument("--out", required=True, help="Output directory")
    parser.add_argument("--lang", action="append", help="Limit to one or more languages")
    parser.add_argument("--dump-ir", help="Write IR JSON to path")
    parser.add_argument("--platform", help="Override platform name")

    args = parser.parse_args(argv)

    cfg = load_config(Path(args.config))
    if args.lang:
        cfg.languages = args.lang
    if args.platform:
        cfg.platform.os = args.platform

    tu = parse_headers(cfg)
    module = normalize_translation_unit(tu, cfg)

    if args.dump_ir:
        dump_ir_json(module, Path(args.dump_ir))

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    generate_bindings(module, cfg, out_dir)

    return 0
