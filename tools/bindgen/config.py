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
class NamingConfig:
    """Configuration for naming conventions."""

    # Naming style for output file names (without extension)
    # Options: "snake_case", "camel_case", "pascal_case", "kebab_case", "screaming_snake_case", "original"
    file_name: str = "original"

    # Naming style for type/struct names
    type_name: str = "original"

    # Naming style for enum names
    enum_name: str = "original"

    # Naming style for enum value names
    enum_value_name: str = "original"

    # Naming style for function names
    function_name: str = "original"

    # Naming style for method names
    method_name: str = "original"

    # Naming style for class names
    class_name: str = "original"

    # Naming style for field/property names
    field_name: str = "original"

    # Naming style for parameter names
    param_name: str = "original"

    # Naming style for constant names
    constant_name: str = "original"

    # Naming style for alias names
    alias_name: str = "original"

    # Prefix/suffix stripping before applying naming style
    # e.g., strip "na_" prefix from "na_window_create" before converting
    strip_prefixes: List[str] = field(default_factory=list)
    strip_suffixes: List[str] = field(default_factory=list)

    # Prefix/suffix to add after applying naming style
    add_prefix: str = ""
    add_suffix: str = ""


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

    # Naming conventions configuration
    naming: NamingConfig = field(default_factory=NamingConfig)

    # Additional custom options (for template use)
    options: Dict[str, Any] = field(default_factory=dict)


@dataclass
class BindgenConfig:
    entry_headers: List[str]
    include_paths: List[str]
    clang_flags: List[str]
    mapping: MappingConfig
    filters: FilterConfig


def _parse_naming(data: Optional[Dict[str, Any]]) -> NamingConfig:
    """Parse naming configuration from config dict."""
    if data is None:
        return NamingConfig()

    return NamingConfig(
        file_name=data.get("file_name", "original"),
        type_name=data.get("type_name", "original"),
        enum_name=data.get("enum_name", "original"),
        enum_value_name=data.get("enum_value_name", "original"),
        function_name=data.get("function_name", "original"),
        method_name=data.get("method_name", "original"),
        class_name=data.get("class_name", "original"),
        field_name=data.get("field_name", "original"),
        param_name=data.get("param_name", "original"),
        constant_name=data.get("constant_name", "original"),
        alias_name=data.get("alias_name", "original"),
        strip_prefixes=list(data.get("strip_prefixes", []) or []),
        strip_suffixes=list(data.get("strip_suffixes", []) or []),
        add_prefix=data.get("add_prefix", ""),
        add_suffix=data.get("add_suffix", ""),
    )


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
        "naming",
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
        naming=_parse_naming(data.get("naming")),
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
