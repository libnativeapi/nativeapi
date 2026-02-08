from __future__ import annotations

from dataclasses import dataclass, field
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
class MappingConfig:
    """Configuration for language mapping including type mappings."""

    # Language name (e.g., "dart", "rust", "python")
    language: str = "default"

    # Direct type name mappings: C type name -> target language type
    # e.g., {"int": "int", "float": "double", "char*": "String"}
    types: Dict[str, str] = field(default_factory=dict)

    # Format string for pointer types, use {inner} as placeholder
    # e.g., "Pointer<{inner}>" for Dart FFI
    pointer_format: str = "{inner}*"

    # Format string for const pointer types
    # e.g., "Pointer<{inner}>" (same as pointer in most FFI)
    const_pointer_format: str = "const {inner}*"

    # Format string for array types, use {element} and {length} as placeholders
    # e.g., "Array<{element}, {length}>" or "List<{element}>"
    array_format: str = "{element}[{length}]"

    # Format string for reference types
    # e.g., "{inner}&" or just "{inner}"
    reference_format: str = "{inner}&"

    # Default type to use when no mapping is found
    # If None, the original type name will be used
    default_type: Optional[str] = None

    # Whether to preserve original type name when no mapping found
    # If False and default_type is None, raises an error
    passthrough_unknown: bool = True

    # Prefix/suffix to add to mapped type names
    type_prefix: str = ""
    type_suffix: str = ""

    # Special mappings for void pointer (often used as opaque handle)
    void_pointer_type: Optional[str] = None

    # Special mapping for const char* (often used as string)
    const_char_pointer_type: Optional[str] = None

    # Additional custom options (for template use)
    options: Dict[str, Any] = field(default_factory=dict)


@dataclass
class BindgenConfig:
    entry_headers: List[str]
    include_paths: List[str]
    clang_flags: List[str]
    mapping: MappingConfig
    filters: FilterConfig


def _parse_mapping(data: Optional[Dict[str, Any]]) -> MappingConfig:
    """Parse mapping configuration from config dict."""
    if data is None:
        return MappingConfig()

    # Extract known fields, put the rest in options
    known_fields = {
        "language",
        "types",
        "pointer_format",
        "const_pointer_format",
        "array_format",
        "reference_format",
        "default_type",
        "passthrough_unknown",
        "type_prefix",
        "type_suffix",
        "void_pointer_type",
        "const_char_pointer_type",
        "options",
    }

    options = dict(data.get("options", {}) or {})
    # Any unknown keys go into options for template use
    for key, value in data.items():
        if key not in known_fields:
            options[key] = value

    return MappingConfig(
        language=data.get("language", "default"),
        types=dict(data.get("types", {}) or {}),
        pointer_format=data.get("pointer_format", "{inner}*"),
        const_pointer_format=data.get("const_pointer_format", "const {inner}*"),
        array_format=data.get("array_format", "{element}[{length}]"),
        reference_format=data.get("reference_format", "{inner}&"),
        default_type=data.get("default_type"),
        passthrough_unknown=data.get("passthrough_unknown", True),
        type_prefix=data.get("type_prefix", ""),
        type_suffix=data.get("type_suffix", ""),
        void_pointer_type=data.get("void_pointer_type"),
        const_char_pointer_type=data.get("const_char_pointer_type"),
        options=options,
    )


def load_config(path: Path) -> BindgenConfig:
    data = _load_yaml(path)
    filters = data.get("filters", {})
    mapping_data = data.get("mapping", {})

    return BindgenConfig(
        entry_headers=data.get("entry_headers", []),
        include_paths=data.get("include_paths", []),
        clang_flags=data.get("clang_flags", []),
        mapping=_parse_mapping(mapping_data),
        filters=FilterConfig(
            allowlist_regex=list(filters.get("allowlist_regex", []) or []),
            denylist_regex=list(filters.get("denylist_regex", []) or []),
            exclude_dirs=list(filters.get("exclude_dirs", []) or []),
        ),
    )
