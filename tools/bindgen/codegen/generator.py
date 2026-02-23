from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any, Dict, List

from ..config import BindgenConfig
from ..ir.model import IRModule
from .context import MappedFile, build_context
from .naming import NameTransformer


def _load_jinja():
    try:
        from jinja2 import Environment, FileSystemLoader
    except Exception as exc:  # pragma: no cover
        raise RuntimeError(
            "jinja2 is required. Install with `pip install jinja2`."
        ) from exc
    return Environment, FileSystemLoader


def _normalize_formatters(mapping_options: Dict[str, Any]) -> List[Dict[str, Any]]:
    """Normalize formatter configs from mapping.options.formatters."""
    raw = mapping_options.get("formatters", [])
    if not isinstance(raw, list):
        print("[bindgen] warning: mapping.options.formatters must be a list, ignoring")
        return []

    normalized: List[Dict[str, Any]] = []
    for index, item in enumerate(raw):
        if not isinstance(item, dict):
            print(f"[bindgen] warning: formatter[{index}] must be an object, skipping")
            continue

        enabled = bool(item.get("enabled", True))
        if not enabled:
            continue

        cmd = item.get("cmd")
        if not isinstance(cmd, list) or not cmd or not all(
            isinstance(token, str) and token for token in cmd
        ):
            print(
                f"[bindgen] warning: formatter[{index}].cmd must be a non-empty string array, skipping"
            )
            continue

        normalized.append(
            {
                "name": str(item.get("name", f"formatter[{index}]")),
                "cmd": cmd,
                "continue_on_error": bool(item.get("continue_on_error", True)),
            }
        )

    return normalized


def _expand_tokens(cmd: List[str], placeholders: Dict[str, str]) -> List[str]:
    expanded: List[str] = []
    for token in cmd:
        value = token
        for key, replacement in placeholders.items():
            value = value.replace(key, replacement)
        expanded.append(value)
    return expanded


def _run_post_formatters(cfg: BindgenConfig, out_dir: Path, config_path: Path) -> None:
    """Run post-generation formatter commands."""
    formatters = _normalize_formatters(cfg.mapping.options or {})
    if not formatters:
        return

    placeholders = {
        "{out_dir}": str(out_dir.resolve()),
        "{config_dir}": str(config_path.resolve().parent),
        "{project_dir}": str(Path.cwd().resolve()),
    }
    run_cwd = str(config_path.resolve().parent)

    for formatter in formatters:
        name = formatter["name"]
        command = _expand_tokens(formatter["cmd"], placeholders)
        continue_on_error = formatter["continue_on_error"]

        print(f"[bindgen] formatter {name}: {' '.join(command)}")
        try:
            result = subprocess.run(
                command,
                cwd=run_cwd,
                capture_output=True,
                text=True,
            )
        except FileNotFoundError as exc:
            message = (
                f"[bindgen] formatter {name} failed: command not found: {command[0]}"
            )
            if continue_on_error:
                print(f"{message} (continuing)")
                continue
            raise RuntimeError(message) from exc
        except OSError as exc:
            message = f"[bindgen] formatter {name} failed: {exc}"
            if continue_on_error:
                print(f"{message} (continuing)")
                continue
            raise RuntimeError(message) from exc

        if result.stdout.strip():
            print(result.stdout.rstrip())

        if result.returncode != 0:
            stderr = result.stderr.strip()
            suffix = f": {stderr}" if stderr else ""
            message = (
                f"[bindgen] formatter {name} exited with {result.returncode}{suffix}"
            )
            if continue_on_error:
                print(f"{message} (continuing)")
                continue
            raise RuntimeError(message)


def _build_file_context(
    file_path: str,
    mapped_file: MappedFile,
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
        "namer": global_context["namer"],
        "raw": global_context["raw"],
        # All items (for cross-referencing)
        "all_items": global_context["items"],
        "all_types": global_context["types"],
        "all_enums": global_context["enums"],
        "all_functions": global_context["functions"],
        "all_classes": global_context["classes"],
        "all_constants": global_context["constants"],
        "all_aliases": global_context["aliases"],
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
        "has_constants": bool(mapped_file.constants),
        "has_aliases": bool(mapped_file.aliases),
    }


def _compute_output_path(
    ir_file_path: str,
    template_name: str,
    out_dir: Path,
    namer: NameTransformer,
) -> Path:
    """
    Compute output path from IR file path and template name.

    Rules:
    - IR path: src/foundation/geometry.h
    - Template: file/dart.j2 -> out/src/foundation/geometry.dart
    - Template: file/rs.j2 -> out/src/foundation/geometry.rs

    File name is transformed according to naming config.
    """
    from pathlib import PurePosixPath

    ir_path = PurePosixPath(ir_file_path)
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
    global_context = build_context(module, cfg.mapping)
    namer = global_context["namer"]

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
            mapped_file = global_context["files"][ir_file_path]
            file_context = _build_file_context(
                ir_file_path, mapped_file, global_context
            )

            for template_path in file_templates:
                template = env.get_template(f"file/{template_path.name}")
                output_path = _compute_output_path(
                    ir_file_path, template_path.name, out_dir, namer
                )

                # Create output directory if needed
                output_path.parent.mkdir(parents=True, exist_ok=True)

                rendered = template.render(**file_context)
                output_path.write_text(rendered + "\n", encoding="utf-8")

    # 3. Run post-generation formatters configured in mapping.options.formatters
    _run_post_formatters(cfg, out_dir, config_path)


def _register_filters(env) -> None:
    """Register custom Jinja2 filters for naming conventions."""
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

    # Register naming filters
    env.filters["snake_case"] = snake_case
    env.filters["camel_case"] = camel_case
    env.filters["pascal_case"] = pascal_case
    env.filters["screaming_snake_case"] = screaming_snake_case
    env.filters["kebab_case"] = kebab_case
    env.filters["strip_prefix"] = strip_prefix
    env.filters["strip_suffix"] = strip_suffix
    env.filters["add_prefix"] = add_prefix
    env.filters["add_suffix"] = add_suffix
