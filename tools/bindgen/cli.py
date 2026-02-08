import argparse
from pathlib import Path

from .codegen.generator import generate_bindings
from .config import load_config
from .ir.serializer import dump_ir_json
from .normalizer import normalize_translation_unit
from .parser import parse_headers


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(prog="bindgen")
    parser.add_argument("--config", required=True, help="Path to config.yaml")
    parser.add_argument("--out", required=True, help="Output directory")
    parser.add_argument("--dump-ir", help="Write IR JSON to path")

    args = parser.parse_args(argv)

    config_path = Path(args.config)
    cfg = load_config(config_path)

    tu = parse_headers(cfg)
    module = normalize_translation_unit(tu, cfg)

    if args.dump_ir:
        dump_ir_json(module, Path(args.dump_ir))

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    generate_bindings(module, cfg, out_dir, config_path)

    return 0
