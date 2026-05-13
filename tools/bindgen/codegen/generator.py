from __future__ import annotations

from pathlib import Path
from typing import Any, Dict, List

from ..config import BindgenConfig
from ..ir.model import IRModule
from .context import MappedFile, TemplateContext, build_context
from .naming import NameTransformer
from .post_formatters import run_post_formatters


def _load_jinja():
    try:
        from jinja2 import Environment, FileSystemLoader
    except Exception as exc:  # pragma: no cover
        raise RuntimeError(
            "jinja2 is required. Install with `pip install jinja2`."
        ) from exc
    return Environment, FileSystemLoader


def _run_post_formatters(cfg: BindgenConfig, out_dir: Path, config_path: Path) -> None:
    """Run post-generation formatter commands. Delegates to post_formatters module."""
    run_post_formatters(cfg, out_dir, config_path)


def _build_file_context(
    file_path: str,
    mapped_file: MappedFile,
    ctx: "TemplateContext",
) -> Dict[str, Any]:
    """Build context dict for a single file Jinja2 template."""
    from pathlib import PurePosixPath

    def _iter_callable_bridges():
        yield from mapped_file.functions
        for cls in mapped_file.classes:
            yield from cls.methods

    callables = list(_iter_callable_bridges())
    uses_ui = any(
        callable_item.return_bridge in {"offset", "size", "rect"}
        for callable_item in callables
    )
    has_handle_classes = any(not cls.is_singleton or cls.singleton_has_handle for cls in mapped_file.classes)
    uses_ffi = any(
        callable_item.return_bridge in {"string", "struct"}
        or callable_item.pre_call_lines
        or callable_item.post_call_lines
        for callable_item in callables
    ) or any(
        item.has_string_fields for item in mapped_file.types
    )
    uses_pkgffi = uses_ffi

    p = PurePosixPath(file_path)

    return {
        # Global context (flattened for Jinja2)
        "module": ctx.module,
        "files": ctx.files,
        "file_paths": ctx.file_paths,
        "mapping": ctx.mapping,
        "namer": ctx.namer,
        "raw": ctx.raw,
        # All items (for cross-referencing)
        "all_items": ctx.items,
        "all_types": ctx.types,
        "all_enums": ctx.enums,
        "all_functions": ctx.functions,
        "all_classes": ctx.classes,
        "all_constants": ctx.constants,
        "all_aliases": ctx.aliases,
        # Current file items (mapped)
        "file": mapped_file,
        "file_path": file_path,
        "file_dir": str(p.parent) if p.parent != PurePosixPath(".") else "",
        "file_name": p.name,
        "file_stem": p.stem,
        "file_ext": p.suffix,
        "items": mapped_file.items,
        "types": mapped_file.types,
        "enums": mapped_file.enums,
        "functions": mapped_file.functions,
        "classes": mapped_file.classes,
        "constants": mapped_file.constants,
        "aliases": mapped_file.aliases,
        # Convenience flags
        "is_empty": not mapped_file.items,
        "has_types": bool(mapped_file.types),
        "has_enums": bool(mapped_file.enums),
        "has_functions": bool(mapped_file.functions),
        "has_classes": bool(mapped_file.classes),
        "has_handle_classes": has_handle_classes,
        "has_constants": bool(mapped_file.constants),
        "has_aliases": bool(mapped_file.aliases),
        "uses_ffi": uses_ffi,
        "uses_pkgffi": uses_pkgffi,
        "uses_ui": uses_ui,
    }


def _compute_output_path(
    ir_file_path: str,
    template_name: str,
    out_dir: Path,
    namer: NameTransformer,
    strip_prefix: str = "",
) -> Path:
    """
    Compute output path from IR file path and template name.

    If *strip_prefix* is given, the first occurrence of it in the IR file
    path is removed before computing the output sub-directory.  This allows
    stripping an outer prefix such as ``Sources/CNativeAPI/src`` so that
    the output lands directly under *out_dir*.

    Rules:
    - IR path: src/foundation/geometry.h, strip_prefix="src/"
           -> out/foundation/geometry.dart
    - Template: file/dart.j2 -> out/geometry.dart (stem "dart")

    File name is transformed according to naming config.
    """
    from pathlib import PurePosixPath

    adjusted = ir_file_path
    if strip_prefix:
        idx = adjusted.find(strip_prefix)
        if idx >= 0:
            adjusted = adjusted[idx + len(strip_prefix):]

    ir_path = PurePosixPath(adjusted)
    # Template stem becomes the new extension
    template_stem = Path(template_name).stem  # e.g., "dart" from "dart.j2"

    # Apply naming transformation to the file stem
    transformed_stem = namer.file_name(ir_path.stem)

    # Build output path: out_dir / ir_dir / transformed_stem.template_stem
    output_name = f"{transformed_stem}.{template_stem}"
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

    Environment, FileSystemLoader = _load_jinja()
    env = Environment(
        loader=FileSystemLoader(str(config_template_root)), autoescape=False
    )

    # Register custom filters
    _register_filters(env)

    # Build context with preprocessed (mapped) data
    ctx = build_context(module, cfg.mapping)
    namer = ctx.namer

    # 1. Render global templates (template/*.j2)
    for template_path in config_template_root.glob("*.j2"):
        template = env.get_template(template_path.name)
        output_name = template_path.stem
        rendered = template.render(**vars(ctx))
        (out_dir / output_name).write_text(rendered + "\n", encoding="utf-8")

    # 2. Render per-file templates (template/file/*.j2)
    file_template_dir = config_template_root / "file"
    if file_template_dir.exists():
        file_templates = list(file_template_dir.glob("*.j2"))

        for ir_file_path in ctx.file_paths:
            mapped_file = ctx.files[ir_file_path]
            file_context = _build_file_context(
                ir_file_path, mapped_file, ctx
            )

            # Resolve output path prefix to strip from IR file paths
            output_strip_prefix = (
                (cfg.mapping.options or {}).get("output_path_prefix", "")
            )

            for template_path in file_templates:
                template = env.get_template(f"file/{template_path.name}")
                output_path = _compute_output_path(
                    ir_file_path, template_path.name, out_dir, namer,
                    strip_prefix=output_strip_prefix,
                )

                # Create output directory if needed
                output_path.parent.mkdir(parents=True, exist_ok=True)

                rendered = template.render(**file_context)
                output_path.write_text(rendered + "\n", encoding="utf-8")

    # 3. Run post-generation formatters configured in mapping.options.formatters
    _run_post_formatters(cfg, out_dir, config_path)


def _register_filters(env) -> None:
    """Register custom Jinja2 filters for naming conventions.

    Delegates to naming.py to avoid duplicating naming logic.
    """
    from .naming import (
        to_snake_case,
        to_camel_case,
        to_pascal_case,
        to_screaming_snake_case,
        to_kebab_case,
    )

    def _strip_prefix(s: str, prefix: str) -> str:
        """Strip prefix from string."""
        if s.startswith(prefix):
            return s[len(prefix) :]
        return s

    def _strip_suffix(s: str, suffix: str) -> str:
        """Strip suffix from string."""
        if s.endswith(suffix):
            return s[: -len(suffix)]
        return s

    def _add_prefix(s: str, prefix: str) -> str:
        """Add prefix to string."""
        return prefix + s

    def _add_suffix(s: str, suffix: str) -> str:
        """Add suffix to string."""
        return s + suffix

    # Register naming filters
    env.filters["snake_case"] = to_snake_case
    env.filters["camel_case"] = to_camel_case
    env.filters["pascal_case"] = to_pascal_case
    env.filters["screaming_snake_case"] = to_screaming_snake_case
    env.filters["kebab_case"] = to_kebab_case
    env.filters["strip_prefix"] = _strip_prefix
    env.filters["strip_suffix"] = _strip_suffix
    env.filters["add_prefix"] = _add_prefix
    env.filters["add_suffix"] = _add_suffix
