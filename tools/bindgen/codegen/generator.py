from __future__ import annotations

from pathlib import Path
from typing import Dict

from ..config import BindgenConfig
from ..ir.model import IRModule
from .context import build_context


def _load_jinja():
    try:
        from jinja2 import Environment, FileSystemLoader  # type: ignore
    except Exception as exc:  # pragma: no cover
        raise RuntimeError("jinja2 is required. Install with `pip install jinja2`.") from exc
    return Environment, FileSystemLoader


def _load_yaml(path: Path) -> dict:
    try:
        import yaml  # type: ignore
    except Exception as exc:  # pragma: no cover
        raise RuntimeError("PyYAML is required for language configs.") from exc
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def generate_bindings(module: IRModule, cfg: BindgenConfig, out_dir: Path) -> None:
    for lang in cfg.languages:
        lang_dir = Path(__file__).resolve().parent.parent / "templates" / lang
        if not lang_dir.exists():
            raise RuntimeError(f"Unknown language template: {lang}")
        lang_cfg = _load_yaml(lang_dir / "lang.yaml")

        Environment, FileSystemLoader = _load_jinja()
        env = Environment(loader=FileSystemLoader(str(lang_dir)), autoescape=False)
        context = build_context(module, lang_cfg)

        for template_path in lang_dir.glob("*.j2"):
            template = env.get_template(template_path.name)
            output_name = template_path.stem
            rendered = template.render(**context)
            (out_dir / output_name).write_text(rendered + "\n", encoding="utf-8")
