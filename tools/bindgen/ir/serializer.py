from __future__ import annotations

import json
from dataclasses import asdict
from pathlib import Path

from .model import IRModule


def dump_ir_json(module: IRModule, path: Path) -> None:
    payload = asdict(module)["files"]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
