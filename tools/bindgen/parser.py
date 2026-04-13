from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import List

from .config import BindgenConfig


class ParseError(RuntimeError):
    pass


def _load_clang():
    try:
        from clang import cindex  # type: ignore
    except Exception as exc:  # pragma: no cover - depends on system
        raise ParseError(
            "clang.cindex is required. Install libclang and the clang Python bindings."
        ) from exc

    # Best-effort: point clang.cindex at the bundled libclang if available.
    if not cindex.Config.loaded:
        try:
            import os
            from pathlib import Path

            import clang  # type: ignore

            libclang_env = os.environ.get("LIBCLANG_PATH")
            if libclang_env:
                cindex.Config.set_library_path(libclang_env)
            else:
                candidate = (
                    Path(clang.__file__).resolve().parent / "native" / "libclang.dylib"
                )
                if candidate.exists():
                    cindex.Config.set_library_file(str(candidate))
        except Exception:
            # If this fails, clang will still attempt to find libclang via default paths.
            pass

    return cindex


def parse_headers(cfg: BindgenConfig):
    cindex = _load_clang()
    index = cindex.Index.create()
    args: List[str] = []
    include_paths = cfg.include_paths
    if not include_paths:
        include_paths = _default_include_paths()
        cfg.include_paths = include_paths
    for path in include_paths:
        args.append(f"-I{path}")
    args.extend(_default_system_include_args())
    args.extend(cfg.clang_flags)
    if not any(a == "-x" or a.startswith("-x") for a in args):
        args.extend(["-x", "c++"])

    discovered = _discover_entry_headers(cfg.filters.exclude_dirs)
    if discovered:
        cfg.entry_headers = discovered
    if not cfg.entry_headers:
        raise ParseError(
            "No headers discovered under src/. Add entry_headers or check project root."
        )

    # Parse all headers into a single translation unit by including them in an ad-hoc file.
    header_includes = "\n".join([f'#include "{h}"' for h in cfg.entry_headers])
    tmp_source = "// bindgen entry\n" + header_includes + "\n"

    try:
        tu = index.parse(
            path="bindgen_entry.cpp",
            args=args,
            unsaved_files=[("bindgen_entry.cpp", tmp_source)],
            options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )
    except cindex.TranslationUnitLoadError as exc:
        raise ParseError(
            "Error parsing translation unit. Check include_paths and clang_flags (e.g. -x c++)."
        ) from exc

    return tu


def _find_project_root(start: Path) -> Path:
    for base in [start] + list(start.parents):
        if (base / "src").is_dir():
            return base
    return start


def _discover_entry_headers(exclude_dirs: List[str] | None = None) -> List[str]:
    root = _find_project_root(Path.cwd().resolve())
    src_dir = root / "src"
    if not src_dir.is_dir():
        return []

    exclude = set(exclude_dirs or [])
    headers: List[Path] = []
    for path in src_dir.rglob("*.h"):
        if "platform" in path.parts:
            continue
        if exclude and any(part in exclude for part in path.parts):
            continue
        headers.append(path)

    def _sort_key(p: Path) -> tuple[int, str]:
        # Put subdirectories (e.g., src/foundation/...) before files directly under src/.
        is_root_file = 1 if p.parent == src_dir else 0
        return (is_root_file, str(p))

    return [str(p) for p in sorted(headers, key=_sort_key)]


def _default_include_paths() -> List[str]:
    root = _find_project_root(Path.cwd().resolve())
    src_dir = root / "src"
    if src_dir.is_dir():
        return [str(src_dir)]
    return []


def _default_system_include_args() -> List[str]:
    if sys.platform != "darwin":
        return []

    args: List[str] = []
    sdk_path = _run_command(["xcrun", "--show-sdk-path"])
    if sdk_path:
        args.extend(["-isysroot", sdk_path])

        sdk_root = Path(sdk_path)
        candidate_paths = [
            sdk_root / "usr/include/c++/v1",
            sdk_root / "usr/include",
            sdk_root / "System/Library/Frameworks",
            sdk_root / "System/Library/SubFrameworks",
        ]
        for path in candidate_paths:
            if path.exists():
                args.append(f"-I{path}")

    clang_path = _run_command(["xcrun", "--find", "clang++"])
    if clang_path:
        toolchain_root = Path(clang_path).resolve().parents[2]
        clang_version_dir = toolchain_root / "usr/lib/clang"
        if clang_version_dir.is_dir():
            versions = sorted(clang_version_dir.iterdir(), reverse=True)
            for version in versions:
                include_dir = version / "include"
                if include_dir.exists():
                    args.append(f"-I{include_dir}")
                    break
        toolchain_include = toolchain_root / "usr/include"
        if toolchain_include.exists():
            args.append(f"-I{toolchain_include}")

    return args


def _run_command(command: List[str]) -> str:
    try:
        result = subprocess.run(
            command,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return ""
    return result.stdout.strip()
