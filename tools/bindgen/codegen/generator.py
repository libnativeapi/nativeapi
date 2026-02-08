from __future__ import annotations

from pathlib import Path

from ..config import BindgenConfig
from ..ir.model import IRModule
from .context import build_context


def _load_jinja():
    try:
        from jinja2 import Environment, FileSystemLoader  # type: ignore
    except Exception as exc:  # pragma: no cover
        raise RuntimeError(
            "jinja2 is required. Install with `pip install jinja2`."
        ) from exc
    return Environment, FileSystemLoader


def generate_bindings(
    module: IRModule, cfg: BindgenConfig, out_dir: Path, config_path: Path
) -> None:
    # Templates are discovered from config.yaml sibling directory:
    #   <config_dir>/template/*.j2
    config_template_root = config_path.resolve().parent / "template"
    if not config_template_root.exists():
        raise RuntimeError(f"Template directory not found: {config_template_root}")

    mapping = dict(cfg.mapping)
    mapping.setdefault("language", "default")

    Environment, FileSystemLoader = _load_jinja()
    env = Environment(
        loader=FileSystemLoader(str(config_template_root)), autoescape=False
    )
    context = build_context(module, mapping)

    for template_path in config_template_root.glob("*.j2"):
        template = env.get_template(template_path.name)
        output_name = template_path.stem
        rendered = template.render(**context)
        (out_dir / output_name).write_text(rendered + "\n", encoding="utf-8")
