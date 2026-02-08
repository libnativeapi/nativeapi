from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional


def _load_yaml(path: Path) -> dict:
    try:
        import yaml  # type: ignore
    except Exception as exc:  # pragma: no cover - best effort error path
        raise RuntimeError(
            "PyYAML is required to read bindgen.yaml. Install with `pip install pyyaml`."
        ) from exc
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


@dataclass
class FilterConfig:
    allowlist_regex: List[str]
    denylist_regex: List[str]
    exclude_dirs: List[str]


@dataclass
class BindgenConfig:
    entry_headers: List[str]
    include_paths: List[str]
    clang_flags: List[str]
    mapping: Dict[str, Any]
    filters: FilterConfig


def load_config(path: Path) -> BindgenConfig:
    data = _load_yaml(path)
    filters = data.get("filters", {})
    mapping = data.get("mapping", {})
    if mapping is None:
        mapping = {}
    return BindgenConfig(
        entry_headers=data.get("entry_headers", []),
        include_paths=data.get("include_paths", []),
        clang_flags=data.get("clang_flags", []),
        mapping=dict(mapping),
        filters=FilterConfig(
            allowlist_regex=list(filters.get("allowlist_regex", []) or []),
            denylist_regex=list(filters.get("denylist_regex", []) or []),
            exclude_dirs=list(filters.get("exclude_dirs", []) or []),
        ),
    )
