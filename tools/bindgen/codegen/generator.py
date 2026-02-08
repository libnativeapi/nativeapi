from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, Optional

from ..config import BindgenConfig, MappingConfig
from ..ir.model import IRFile, IRModule, IRType
from .context import build_context


def _load_jinja():
    try:
        from jinja2 import BaseLoader, Environment, FileSystemLoader, TemplateNotFound
    except Exception as exc:  # pragma: no cover
        raise RuntimeError(
            "jinja2 is required. Install with `pip install jinja2`."
        ) from exc
    return Environment, FileSystemLoader, BaseLoader, TemplateNotFound


def _build_file_context(
    file_path: str,
    ir_file: IRFile,
    global_context: Dict[str, Any],
) -> Dict[str, Any]:
    """Build context for a single file template."""
    from pathlib import PurePosixPath

    p = PurePosixPath(file_path)

    return {
        # Global context
        "module": global_context["module"],
        "files": global_context["files"],
        "file_paths": global_context["file_paths"],
        "mapping": global_context["mapping"],
        # All items (for cross-referencing)
        "all_types": global_context["types"],
        "all_enums": global_context["enums"],
        "all_functions": global_context["functions"],
        "all_classes": global_context["classes"],
        "all_constants": global_context["constants"],
        "all_aliases": global_context["aliases"],
        # Current file items
        "file": ir_file,
        "file_path": file_path,
        "file_dir": str(p.parent) if p.parent != PurePosixPath(".") else "",
        "file_name": p.name,
        "file_stem": p.stem,
        "file_ext": p.suffix,
        "types": ir_file.types,
        "enums": ir_file.enums,
        "functions": ir_file.functions,
        "classes": ir_file.classes,
        "constants": ir_file.constants,
        "aliases": ir_file.aliases,
        # Convenience flags
        "is_empty": (
            not ir_file.types
            and not ir_file.enums
            and not ir_file.functions
            and not ir_file.classes
            and not ir_file.constants
            and not ir_file.aliases
        ),
        "has_types": bool(ir_file.types),
        "has_enums": bool(ir_file.enums),
        "has_functions": bool(ir_file.functions),
        "has_classes": bool(ir_file.classes),
        "has_constants": bool(ir_file.constants),
        "has_aliases": bool(ir_file.aliases),
    }


def _register_partials(env, partials_dir: Path) -> None:
    """Register partial templates as macros that can be included."""
    if not partials_dir.exists():
        return

    # Partials are available via {% include 'partials/xxx.j2' %}
    # No special registration needed since we use FileSystemLoader


def _compute_output_path(
    ir_file_path: str,
    template_name: str,
    out_dir: Path,
) -> Path:
    """
    Compute output path from IR file path and template name.

    Rules:
    - IR path: src/foundation/geometry.h
    - Template: file/dart.j2 -> out/src/foundation/geometry.dart
    - Template: file/rs.j2 -> out/src/foundation/geometry.rs
    """
    from pathlib import PurePosixPath

    ir_path = PurePosixPath(ir_file_path)
    # Template stem becomes the new extension
    template_stem = Path(template_name).stem  # e.g., "dart" from "dart.j2"

    # Build output path: out_dir / ir_dir / ir_stem.template_stem
    output_name = f"{ir_path.stem}.{template_stem}"
    output_path = out_dir / ir_path.parent / output_name

    return output_path


def generate_bindings(
    module: IRModule, cfg: BindgenConfig, out_dir: Path, config_path: Path
) -> None:
    """
    Generate bindings using templates.

    Template directory structure:
    - template/*.j2           - Global templates (rendered once)
    - template/file/*.j2      - Per-file templates (rendered for each IR file)
    - template/partials/*.j2  - Partial templates (type.j2, enum.j2, function.j2, etc.)

    Partials can be included in file templates:
    {% include 'partials/type.j2' %}
    """
    # Templates are discovered from config.yaml sibling directory:
    #   <config_dir>/template/*.j2
    config_template_root = config_path.resolve().parent / "template"
    if not config_template_root.exists():
        raise RuntimeError(f"Template directory not found: {config_template_root}")

    Environment, FileSystemLoader, BaseLoader, TemplateNotFound = _load_jinja()
    env = Environment(
        loader=FileSystemLoader(str(config_template_root)), autoescape=False
    )

    # Register custom filters with mapping config
    _register_filters(env, cfg.mapping)

    global_context = build_context(module, cfg.mapping)

    # 1. Render global templates (template/*.j2)
    for template_path in config_template_root.glob("*.j2"):
        template = env.get_template(template_path.name)
        output_name = template_path.stem
        rendered = template.render(**global_context)
        (out_dir / output_name).write_text(rendered + "\n", encoding="utf-8")

    # 2. Render per-file templates (template/file/*.j2)
    file_template_dir = config_template_root / "file"
    if file_template_dir.exists():
        file_templates = list(file_template_dir.glob("*.j2"))

        for ir_file_path in global_context["file_paths"]:
            ir_file = global_context["files"][ir_file_path]
            file_context = _build_file_context(ir_file_path, ir_file, global_context)

            for template_path in file_templates:
                template = env.get_template(f"file/{template_path.name}")
                output_path = _compute_output_path(
                    ir_file_path, template_path.name, out_dir
                )

                # Create output directory if needed
                output_path.parent.mkdir(parents=True, exist_ok=True)

                rendered = template.render(**file_context)
                output_path.write_text(rendered + "\n", encoding="utf-8")


def _map_ir_type(ir_type: IRType, mapping_cfg: MappingConfig) -> str:
    """
    Map an IRType to a target language type string using the mapping config.

    Handles different IRType kinds:
    - primitive: int, float, void, bool, etc.
    - pointer: T* -> pointer_format with {inner}
    - array: T[N] -> array_format with {element} and {length}
    - named: user-defined types (struct, enum, class, typedef)
    - reference: T& -> reference_format with {inner}
    """
    kind = ir_type.kind

    # Handle void type
    if kind == "void":
        return mapping_cfg.types.get("void", "void")

    # Handle pointer types
    if kind == "pointer":
        inner_type = ir_type.to
        if inner_type is None:
            inner_str = "void"
        else:
            inner_str = _map_ir_type(inner_type, mapping_cfg)

        # Check for special void* mapping
        if inner_type and inner_type.kind == "void" and mapping_cfg.void_pointer_type:
            return mapping_cfg.void_pointer_type

        # Check for const char* (string) mapping
        is_const = "const" in ir_type.qualifiers
        if (
            inner_type
            and inner_type.kind == "primitive"
            and inner_type.name == "char"
            and is_const
            and mapping_cfg.const_char_pointer_type
        ):
            return mapping_cfg.const_char_pointer_type

        # Check for direct mapping of pointer type (e.g., "int*" -> "IntPtr")
        if inner_type and inner_type.name:
            ptr_key = f"{inner_type.name}*"
            if ptr_key in mapping_cfg.types:
                return mapping_cfg.types[ptr_key]

        # Apply pointer format
        if is_const:
            return mapping_cfg.const_pointer_format.format(inner=inner_str)
        else:
            return mapping_cfg.pointer_format.format(inner=inner_str)

    # Handle array types
    if kind == "array":
        element_type = ir_type.element
        if element_type is None:
            element_str = "void"
        else:
            element_str = _map_ir_type(element_type, mapping_cfg)

        length = ir_type.length if ir_type.length is not None else 0
        return mapping_cfg.array_format.format(element=element_str, length=length)

    # Handle reference types
    if kind == "reference":
        inner_type = ir_type.to
        if inner_type is None:
            inner_str = "void"
        else:
            inner_str = _map_ir_type(inner_type, mapping_cfg)
        return mapping_cfg.reference_format.format(inner=inner_str)

    # Handle primitive types
    if kind == "primitive":
        type_name = ir_type.name or "int"

        # Check for const qualifier in type name lookup
        is_const = "const" in ir_type.qualifiers
        if is_const:
            const_key = f"const {type_name}"
            if const_key in mapping_cfg.types:
                return mapping_cfg.types[const_key]

        # Look up in type mappings
        if type_name in mapping_cfg.types:
            mapped = mapping_cfg.types[type_name]
            return f"{mapping_cfg.type_prefix}{mapped}{mapping_cfg.type_suffix}"

        # Passthrough or default
        if mapping_cfg.passthrough_unknown:
            return f"{mapping_cfg.type_prefix}{type_name}{mapping_cfg.type_suffix}"
        elif mapping_cfg.default_type:
            return mapping_cfg.default_type
        else:
            return type_name

    # Handle named types (struct, enum, class, typedef, etc.)
    if kind in ("named", "struct", "enum", "class", "typedef", "elaborated"):
        type_name = ir_type.name or "unknown"

        # Look up in type mappings
        if type_name in mapping_cfg.types:
            mapped = mapping_cfg.types[type_name]
            return f"{mapping_cfg.type_prefix}{mapped}{mapping_cfg.type_suffix}"

        # Passthrough or default
        if mapping_cfg.passthrough_unknown:
            return f"{mapping_cfg.type_prefix}{type_name}{mapping_cfg.type_suffix}"
        elif mapping_cfg.default_type:
            return mapping_cfg.default_type
        else:
            return type_name

    # Handle function pointer types
    if kind == "function_pointer":
        # For function pointers, we might want a special format
        # For now, just return a generic function pointer type
        if "function_pointer" in mapping_cfg.types:
            return mapping_cfg.types["function_pointer"]
        return "FunctionPointer"

    # Fallback: use the kind or name
    if ir_type.name:
        if ir_type.name in mapping_cfg.types:
            return mapping_cfg.types[ir_type.name]
        if mapping_cfg.passthrough_unknown:
            return f"{mapping_cfg.type_prefix}{ir_type.name}{mapping_cfg.type_suffix}"

    if mapping_cfg.default_type:
        return mapping_cfg.default_type

    return kind


def _register_filters(env, mapping_cfg: MappingConfig) -> None:
    """Register custom Jinja2 filters."""
    import re

    def snake_case(s: str) -> str:
        """Convert to snake_case."""
        s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", s)
        s = re.sub(r"([a-z\d])([A-Z])", r"\1_\2", s)
        return s.lower().replace("-", "_")

    def camel_case(s: str) -> str:
        """Convert to camelCase."""
        parts = re.split(r"[_\-\s]+", s)
        if not parts:
            return s
        return parts[0].lower() + "".join(p.title() for p in parts[1:])

    def pascal_case(s: str) -> str:
        """Convert to PascalCase."""
        parts = re.split(r"[_\-\s]+", s)
        return "".join(p.title() for p in parts)

    def screaming_snake_case(s: str) -> str:
        """Convert to SCREAMING_SNAKE_CASE."""
        return snake_case(s).upper()

    def kebab_case(s: str) -> str:
        """Convert to kebab-case."""
        return snake_case(s).replace("_", "-")

    def strip_prefix(s: str, prefix: str) -> str:
        """Strip prefix from string."""
        if s.startswith(prefix):
            return s[len(prefix) :]
        return s

    def strip_suffix(s: str, suffix: str) -> str:
        """Strip suffix from string."""
        if s.endswith(suffix):
            return s[: -len(suffix)]
        return s

    def add_prefix(s: str, prefix: str) -> str:
        """Add prefix to string."""
        return prefix + s

    def add_suffix(s: str, suffix: str) -> str:
        """Add suffix to string."""
        return s + suffix

    # Type mapping filters
    def map_type(ir_type) -> str:
        """
        Map an IRType to a target language type string.

        Usage in templates:
            {{ param.type | map_type }}
            {{ field.type | map_type }}
        """
        if ir_type is None:
            return "void"

        # Handle IRType dataclass
        if hasattr(ir_type, "kind"):
            return _map_ir_type(ir_type, mapping_cfg)

        # Handle dict representation (from JSON)
        if isinstance(ir_type, dict):
            # Convert dict to IRType-like object for processing
            kind = ir_type.get("kind", "unknown")
            name = ir_type.get("name")
            qualifiers = ir_type.get("qualifiers", [])

            # Simple lookup for primitive/named types
            if kind in ("primitive", "named", "struct", "enum", "class", "typedef"):
                type_name = name or kind
                if type_name in mapping_cfg.types:
                    mapped = mapping_cfg.types[type_name]
                    return f"{mapping_cfg.type_prefix}{mapped}{mapping_cfg.type_suffix}"
                if mapping_cfg.passthrough_unknown:
                    return (
                        f"{mapping_cfg.type_prefix}{type_name}{mapping_cfg.type_suffix}"
                    )
                return mapping_cfg.default_type or type_name

            # Handle pointer
            if kind == "pointer":
                inner = ir_type.get("to")
                inner_str = map_type(inner) if inner else "void"

                # Check for void*
                if (
                    inner
                    and inner.get("kind") == "void"
                    and mapping_cfg.void_pointer_type
                ):
                    return mapping_cfg.void_pointer_type

                # Check for const char*
                is_const = "const" in qualifiers
                if (
                    inner
                    and inner.get("kind") == "primitive"
                    and inner.get("name") == "char"
                    and is_const
                    and mapping_cfg.const_char_pointer_type
                ):
                    return mapping_cfg.const_char_pointer_type

                if is_const:
                    return mapping_cfg.const_pointer_format.format(inner=inner_str)
                return mapping_cfg.pointer_format.format(inner=inner_str)

            # Handle array
            if kind == "array":
                element = ir_type.get("element")
                element_str = map_type(element) if element else "void"
                length = ir_type.get("length", 0)
                return mapping_cfg.array_format.format(
                    element=element_str, length=length
                )

            # Handle reference
            if kind == "reference":
                inner = ir_type.get("to")
                inner_str = map_type(inner) if inner else "void"
                return mapping_cfg.reference_format.format(inner=inner_str)

            return name or kind

        # Handle string (type name directly)
        if isinstance(ir_type, str):
            if ir_type in mapping_cfg.types:
                return mapping_cfg.types[ir_type]
            if mapping_cfg.passthrough_unknown:
                return f"{mapping_cfg.type_prefix}{ir_type}{mapping_cfg.type_suffix}"
            return mapping_cfg.default_type or ir_type

        return str(ir_type)

    def map_type_name(name: str) -> str:
        """
        Simple type name mapping lookup.

        Usage in templates:
            {{ "int" | map_type_name }}
            {{ type_name | map_type_name }}
        """
        if name in mapping_cfg.types:
            return mapping_cfg.types[name]
        if mapping_cfg.passthrough_unknown:
            return f"{mapping_cfg.type_prefix}{name}{mapping_cfg.type_suffix}"
        return mapping_cfg.default_type or name

    def format_type(
        ir_type,
        pointer_fmt: Optional[str] = None,
        array_fmt: Optional[str] = None,
        ref_fmt: Optional[str] = None,
    ) -> str:
        """
        Map an IRType with custom format strings.

        Usage in templates:
            {{ param.type | format_type(pointer_fmt="*{inner}") }}
            {{ field.type | format_type(array_fmt="[{length}]{element}") }}
        """
        # Create a modified config with custom formats
        custom_cfg = MappingConfig(
            language=mapping_cfg.language,
            types=mapping_cfg.types,
            pointer_format=pointer_fmt or mapping_cfg.pointer_format,
            const_pointer_format=pointer_fmt or mapping_cfg.const_pointer_format,
            array_format=array_fmt or mapping_cfg.array_format,
            reference_format=ref_fmt or mapping_cfg.reference_format,
            default_type=mapping_cfg.default_type,
            passthrough_unknown=mapping_cfg.passthrough_unknown,
            type_prefix=mapping_cfg.type_prefix,
            type_suffix=mapping_cfg.type_suffix,
            void_pointer_type=mapping_cfg.void_pointer_type,
            const_char_pointer_type=mapping_cfg.const_char_pointer_type,
        )

        if ir_type is None:
            return "void"

        if hasattr(ir_type, "kind"):
            return _map_ir_type(ir_type, custom_cfg)

        # Fallback to regular map_type
        return map_type(ir_type)

    def is_pointer_type(ir_type) -> bool:
        """Check if an IRType is a pointer type."""
        if ir_type is None:
            return False
        if hasattr(ir_type, "kind"):
            return ir_type.kind == "pointer"
        if isinstance(ir_type, dict):
            return ir_type.get("kind") == "pointer"
        return False

    def is_array_type(ir_type) -> bool:
        """Check if an IRType is an array type."""
        if ir_type is None:
            return False
        if hasattr(ir_type, "kind"):
            return ir_type.kind == "array"
        if isinstance(ir_type, dict):
            return ir_type.get("kind") == "array"
        return False

    def is_void_type(ir_type) -> bool:
        """Check if an IRType is void."""
        if ir_type is None:
            return True
        if hasattr(ir_type, "kind"):
            return ir_type.kind == "void"
        if isinstance(ir_type, dict):
            return ir_type.get("kind") == "void"
        return False

    def get_inner_type(ir_type):
        """Get the inner type of a pointer or reference."""
        if ir_type is None:
            return None
        if hasattr(ir_type, "to"):
            return ir_type.to
        if isinstance(ir_type, dict):
            return ir_type.get("to")
        return None

    def get_element_type(ir_type):
        """Get the element type of an array."""
        if ir_type is None:
            return None
        if hasattr(ir_type, "element"):
            return ir_type.element
        if isinstance(ir_type, dict):
            return ir_type.get("element")
        return None

    # Register all filters
    env.filters["snake_case"] = snake_case
    env.filters["camel_case"] = camel_case
    env.filters["pascal_case"] = pascal_case
    env.filters["screaming_snake_case"] = screaming_snake_case
    env.filters["kebab_case"] = kebab_case
    env.filters["strip_prefix"] = strip_prefix
    env.filters["strip_suffix"] = strip_suffix
    env.filters["add_prefix"] = add_prefix
    env.filters["add_suffix"] = add_suffix

    # Type mapping filters
    env.filters["map_type"] = map_type
    env.filters["map_type_name"] = map_type_name
    env.filters["format_type"] = format_type

    # Type inspection filters
    env.filters["is_pointer_type"] = is_pointer_type
    env.filters["is_array_type"] = is_array_type
    env.filters["is_void_type"] = is_void_type
    env.filters["get_inner_type"] = get_inner_type
    env.filters["get_element_type"] = get_element_type
