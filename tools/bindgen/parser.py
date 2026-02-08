from __future__ import annotations

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
    args.extend(cfg.clang_flags)
    if not any(a == "-x" or a.startswith("-x") for a in args):
        args.extend(["-x", "c++"])

    discovered = _discover_entry_headers()
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


def _discover_entry_headers() -> List[str]:
    root = _find_project_root(Path.cwd().resolve())
    src_dir = root / "src"
    if not src_dir.is_dir():
        return []

    headers: List[Path] = []
    for path in src_dir.rglob("*.h"):
        if "platform" in path.parts:
            continue
        headers.append(path)

    return [str(p) for p in sorted(headers)]


def _default_include_paths() -> List[str]:
    root = _find_project_root(Path.cwd().resolve())
    src_dir = root / "src"
    if src_dir.is_dir():
        return [str(src_dir)]
    return []
