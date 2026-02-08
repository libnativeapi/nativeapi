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
    for path in cfg.include_paths:
        args.append(f"-I{path}")
    args.extend(cfg.clang_flags)
    if not any(a == "-x" or a.startswith("-x") for a in args):
        args.extend(["-x", "c++"])

    if not cfg.entry_headers:
        raise ParseError("No entry_headers specified in config.")

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
