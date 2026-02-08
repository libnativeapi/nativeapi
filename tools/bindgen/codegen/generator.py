from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, List

from ..config import BindgenConfig
from ..ir.model import IRFile, IRModule
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

    mapping = dict(cfg.mapping)
    mapping.setdefault("language", "default")

    Environment, FileSystemLoader, BaseLoader, TemplateNotFound = _load_jinja()
    env = Environment(
        loader=FileSystemLoader(str(config_template_root)), autoescape=False
    )

    # Register any custom filters here if needed
    _register_filters(env)

    global_context = build_context(module, mapping)

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


def _register_filters(env) -> None:
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
