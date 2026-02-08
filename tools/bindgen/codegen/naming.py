"""Naming convention utilities for code generation."""

from __future__ import annotations

import re
from typing import List, Optional

from ..config import NamingConfig


def to_snake_case(s: str) -> str:
    """Convert string to snake_case."""
    # Handle acronyms: XMLParser -> xml_parser
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", s)
    # Handle camelCase: camelCase -> camel_case
    s = re.sub(r"([a-z\d])([A-Z])", r"\1_\2", s)
    # Replace hyphens and spaces with underscores
    s = re.sub(r"[-\s]+", "_", s)
    return s.lower()


def to_camel_case(s: str) -> str:
    """Convert string to camelCase."""
    # First convert to snake_case to normalize
    s = to_snake_case(s)
    parts = s.split("_")
    if not parts:
        return s
    # First part lowercase, rest title case
    return parts[0].lower() + "".join(p.title() for p in parts[1:])


def to_pascal_case(s: str) -> str:
    """Convert string to PascalCase."""
    # First convert to snake_case to normalize
    s = to_snake_case(s)
    parts = s.split("_")
    return "".join(p.title() for p in parts)


def to_screaming_snake_case(s: str) -> str:
    """Convert string to SCREAMING_SNAKE_CASE."""
    return to_snake_case(s).upper()


def to_kebab_case(s: str) -> str:
    """Convert string to kebab-case."""
    return to_snake_case(s).replace("_", "-")


def apply_style(name: str, style: str) -> str:
    """
    Apply a naming style to a string.

    Args:
        name: The original name
        style: One of "snake_case", "camel_case", "pascal_case",
               "screaming_snake_case", "kebab_case", "original"

    Returns:
        The converted name
    """
    if style == "original" or not style:
        return name
    elif style == "snake_case":
        return to_snake_case(name)
    elif style == "camel_case":
        return to_camel_case(name)
    elif style == "pascal_case":
        return to_pascal_case(name)
    elif style == "screaming_snake_case":
        return to_screaming_snake_case(name)
    elif style == "kebab_case":
        return to_kebab_case(name)
    else:
        return name


def strip_affixes(name: str, prefixes: List[str], suffixes: List[str]) -> str:
    """
    Strip prefixes and suffixes from a name.

    Args:
        name: The original name
        prefixes: List of prefixes to strip (e.g., ["na_", "NA_"])
        suffixes: List of suffixes to strip (e.g., ["_t", "Handle"])

    Returns:
        The name with prefixes/suffixes removed
    """
    result = name

    # Strip prefixes (try longest first)
    for prefix in sorted(prefixes, key=len, reverse=True):
        if result.startswith(prefix):
            result = result[len(prefix) :]
            break

    # Strip suffixes (try longest first)
    for suffix in sorted(suffixes, key=len, reverse=True):
        if result.endswith(suffix):
            result = result[: -len(suffix)]
            break

    return result


def add_affixes(name: str, prefix: str, suffix: str) -> str:
    """
    Add prefix and suffix to a name.

    Args:
        name: The original name
        prefix: Prefix to add
        suffix: Suffix to add

    Returns:
        The name with prefix/suffix added
    """
    return f"{prefix}{name}{suffix}"


def transform_name(
    name: str,
    style: str,
    strip_prefixes: Optional[List[str]] = None,
    strip_suffixes: Optional[List[str]] = None,
    add_prefix: str = "",
    add_suffix: str = "",
) -> str:
    """
    Transform a name with full pipeline: strip -> style -> add.

    Args:
        name: The original name
        style: Naming style to apply
        strip_prefixes: Prefixes to strip before styling
        strip_suffixes: Suffixes to strip before styling
        add_prefix: Prefix to add after styling
        add_suffix: Suffix to add after styling

    Returns:
        The fully transformed name
    """
    if strip_prefixes is None:
        strip_prefixes = []
    if strip_suffixes is None:
        strip_suffixes = []

    # 1. Strip prefixes/suffixes
    result = strip_affixes(name, strip_prefixes, strip_suffixes)

    # 2. Apply naming style
    result = apply_style(result, style)

    # 3. Add prefix/suffix
    result = add_affixes(result, add_prefix, add_suffix)

    return result


class NameTransformer:
    """Helper class to transform names using NamingConfig."""

    def __init__(self, config: NamingConfig):
        self.config = config

    def _transform(self, name: str, style: str) -> str:
        """Apply transformation with config's strip/add settings."""
        return transform_name(
            name,
            style,
            strip_prefixes=self.config.strip_prefixes,
            strip_suffixes=self.config.strip_suffixes,
            add_prefix=self.config.add_prefix,
            add_suffix=self.config.add_suffix,
        )

    def file_name(self, name: str) -> str:
        """Transform a file name (without extension)."""
        return self._transform(name, self.config.file_name)

    def type_name(self, name: str) -> str:
        """Transform a type/struct name."""
        return self._transform(name, self.config.type_name)

    def enum_name(self, name: str) -> str:
        """Transform an enum name."""
        return self._transform(name, self.config.enum_name)

    def enum_value_name(self, name: str) -> str:
        """Transform an enum value name."""
        return self._transform(name, self.config.enum_value_name)

    def function_name(self, name: str) -> str:
        """Transform a function name."""
        return self._transform(name, self.config.function_name)

    def method_name(self, name: str) -> str:
        """Transform a method name."""
        return self._transform(name, self.config.method_name)

    def class_name(self, name: str) -> str:
        """Transform a class name."""
        return self._transform(name, self.config.class_name)

    def field_name(self, name: str) -> str:
        """Transform a field/property name."""
        return self._transform(name, self.config.field_name)

    def param_name(self, name: str) -> str:
        """Transform a parameter name."""
        return self._transform(name, self.config.param_name)

    def constant_name(self, name: str) -> str:
        """Transform a constant name."""
        return self._transform(name, self.config.constant_name)

    def alias_name(self, name: str) -> str:
        """Transform an alias name."""
        return self._transform(name, self.config.alias_name)
