from __future__ import annotations

from typing import Dict

from ..ir.model import IRModule


def build_context(module: IRModule, mapping: Dict) -> Dict:
    return {
        "module": module,
        "types": module.types,
        "enums": module.enums,
        "functions": module.functions,
        "constants": module.constants,
        "aliases": module.aliases,
        "mapping": mapping,
    }
