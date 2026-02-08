from __future__ import annotations

from typing import Any, Dict

from ..config import MappingConfig
from ..ir.model import IRModule


def build_context(module: IRModule, mapping: MappingConfig) -> Dict[str, Any]:
    files = module.files
    sorted_paths = sorted(files.keys())
    types = []
    enums = []
    functions = []
    classes = []
    constants = []
    aliases = []
    for path in sorted_paths:
        bucket = files[path]
        types.extend(bucket.types)
        enums.extend(bucket.enums)
        functions.extend(bucket.functions)
        classes.extend(bucket.classes)
        constants.extend(bucket.constants)
        aliases.extend(bucket.aliases)
    return {
        "module": module,
        "files": files,
        "file_paths": sorted_paths,
        "types": types,
        "enums": enums,
        "functions": functions,
        "classes": classes,
        "constants": constants,
        "aliases": aliases,
        "mapping": mapping,
    }
