from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any, List, Optional


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
    export_macro: Optional[str]
    allowlist_regex: List[str]
    denylist_regex: List[str]


@dataclass
class PlatformConfig:
    os: str
    abi: str


@dataclass
class BindgenConfig:
    entry_headers: List[str]
    include_paths: List[str]
    clang_flags: List[str]
    languages: List[str]
    filters: FilterConfig
    platform: PlatformConfig


def load_config(path: Path) -> BindgenConfig:
    data = _load_yaml(path)
    filters = data.get("filters", {})
    platform = data.get("platform", {})
    return BindgenConfig(
        entry_headers=data.get("entry_headers", []),
        include_paths=data.get("include_paths", []),
        clang_flags=data.get("clang_flags", []),
        languages=data.get("languages", []),
        filters=FilterConfig(
            export_macro=filters.get("export_macro"),
            allowlist_regex=list(filters.get("allowlist_regex", []) or []),
            denylist_regex=list(filters.get("denylist_regex", []) or []),
        ),
        platform=PlatformConfig(
            os=platform.get("os", ""),
            abi=platform.get("abi", ""),
        ),
    )
