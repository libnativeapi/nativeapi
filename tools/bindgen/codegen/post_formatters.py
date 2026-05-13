"""Post-generation formatter runner.

Extracted from generator.py to keep concerns separated.
"""

from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any, Dict, List

from ..config import BindgenConfig


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


def run_post_formatters(cfg: BindgenConfig, out_dir: Path, config_path: Path) -> None:
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
