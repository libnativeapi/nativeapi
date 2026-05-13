from __future__ import annotations

import re
from pathlib import Path
from typing import Optional

from .config import BindgenConfig
from .ir.model import (
    IRAlias,
    IRClass,
    IRConstant,
    IREnum,
    IREnumValue,
    IRField,
    IRFile,
    IRFunction,
    IRMethod,
    IRModule,
    IRParam,
    IRStruct,
    IRType,
)


def _compile_filters(cfg: BindgenConfig):
    allow = [re.compile(p) for p in cfg.filters.allowlist_regex]
    deny = [re.compile(p) for p in cfg.filters.denylist_regex]
    exclude = set(cfg.filters.exclude_dirs)
    return allow, deny, exclude


def _passes_name_filters(name: str, allow, deny) -> bool:
    if allow and not any(r.search(name) for r in allow):
        return False
    if deny and any(r.search(name) for r in deny):
        return False
    return True


def _is_ignored(cursor) -> bool:
    if cursor.raw_comment and "bindgen:ignore" in cursor.raw_comment:
        return True
    return False


def _is_excluded(cursor, exclude_dirs: set[str]) -> bool:
    if not exclude_dirs or cursor.location.file is None:
        return False
    path = Path(str(cursor.location.file)).resolve()
    return any(part in exclude_dirs for part in path.parts)


def _find_project_root(start: Path) -> Path:
    for base in [start] + list(start.parents):
        if (base / "src").is_dir():
            return base
    return start


def _cursor_path(cursor) -> Optional[str]:
    if cursor.location.file is None:
        return None
    path = Path(str(cursor.location.file)).resolve()
    root = _find_project_root(Path.cwd().resolve())
    rel_path = None
    try:
        rel_path = path.relative_to(root)
    except ValueError:
        rel_path = None

    if rel_path is not None:
        rel = str(rel_path).replace("\\", "/")
        if rel.startswith("src/"):
            return rel[4:]

    parts = path.parts
    if "src" in parts:
        idx = parts.index("src")
        # Skip "src" directory to avoid src/ prefix in output paths
        rest = parts[idx + 1:]
        if not rest:
            return "unknown"
        return Path(*rest).as_posix()
    return "unknown"


def _unqualified_type_name(spelling: str) -> str:
    name = (spelling or "").strip()
    if not name:
        return name
    if "::" in name:
        name = name.split("::")[-1]
    return name


def _is_std_string_type(spelling: str) -> bool:
    normalized = (spelling or "").strip()
    return (
        "std::string" in normalized
        or "std::basic_string<char" in normalized
        or "basic_string<char" in normalized
    )


# Mapping from lower-cased C type spelling -> IRType kind
_PRIMITIVE_TYPE_BY_NAME: dict[str, str] = {
    "void": "void",
    "bool": "bool",
    "_bool": "bool",
    "char": "int8",
    "signed char": "int8",
    "unsigned char": "uint8",
    "short": "int16",
    "short int": "int16",
    "signed short": "int16",
    "unsigned short": "uint16",
    "unsigned short int": "uint16",
    "int": "int32",
    "signed int": "int32",
    "unsigned int": "uint32",
    "long": "int64",
    "long int": "int64",
    "signed long": "int64",
    "unsigned long": "uint64",
    "unsigned long int": "uint64",
    "long long": "int64",
    "long long int": "int64",
    "signed long long": "int64",
    "unsigned long long": "uint64",
    "unsigned long long int": "uint64",
    "float": "float32",
    "double": "float64",
    "long double": "float64",
    "size_t": "size_t",
    "ssize_t": "ssize_t",
    "intptr_t": "intptr",
    "uintptr_t": "uintptr",
}


def _type_from_clang(tp) -> IRType:
    from clang import cindex  # type: ignore

    qualifiers = []
    if tp.is_const_qualified():
        qualifiers.append("const")
    if tp.is_volatile_qualified():
        qualifiers.append("volatile")

    spelling = (tp.spelling or "").strip()
    if _is_std_string_type(spelling):
        return IRType(kind="string", name="std::string", qualifiers=qualifiers)

    try:
        kind = tp.kind
    except ValueError:
        lower = spelling.lower()

        # Try lookup in the primitive type name map first
        ir_kind = _PRIMITIVE_TYPE_BY_NAME.get(lower)
        if ir_kind is not None:
            return IRType(kind=ir_kind, qualifiers=qualifiers)

        # Pointer/reference detection via string suffixes (fallback path)
        if lower.endswith("&&"):
            pointee = IRType(
                kind="unknown",
                name=_unqualified_type_name(spelling[:-2]),
            )
            return IRType(kind="rvalue_reference", to=pointee, qualifiers=qualifiers)
        if lower.endswith("&"):
            pointee = IRType(
                kind="unknown",
                name=_unqualified_type_name(spelling[:-1]),
            )
            return IRType(kind="reference", to=pointee, qualifiers=qualifiers)
        if lower.endswith("*"):
            base = spelling[:-1].strip()
            if base in ("char", "const char"):
                return IRType(kind="cstring", qualifiers=qualifiers)
            pointee = IRType(kind="unknown", name=_unqualified_type_name(base))
            return IRType(kind="pointer", to=pointee, qualifiers=qualifiers)

        return IRType(
            kind="unknown",
            name=_unqualified_type_name(spelling),
            qualifiers=qualifiers,
        )

    # ---- libclang TypeKind path ----
    _TYPEKIND_MAP: dict[int, str] = {
        cindex.TypeKind.VOID: "void",
        cindex.TypeKind.BOOL: "bool",
        cindex.TypeKind.CHAR_S: "int8",
        cindex.TypeKind.SCHAR: "int8",
        cindex.TypeKind.UCHAR: "uint8",
        cindex.TypeKind.SHORT: "int16",
        cindex.TypeKind.USHORT: "uint16",
        cindex.TypeKind.INT: "int32",
        cindex.TypeKind.UINT: "uint32",
        cindex.TypeKind.LONG: "int64",
        cindex.TypeKind.ULONG: "uint64",
        cindex.TypeKind.LONGLONG: "int64",
        cindex.TypeKind.ULONGLONG: "uint64",
        cindex.TypeKind.FLOAT: "float32",
        cindex.TypeKind.DOUBLE: "float64",
    }

    ir_kind = _TYPEKIND_MAP.get(kind)
    if ir_kind is not None:
        return IRType(kind=ir_kind, qualifiers=qualifiers)

    if kind == cindex.TypeKind.LVALUEREFERENCE:
        pointee = _type_from_clang(tp.get_pointee())
        return IRType(kind="reference", to=pointee, qualifiers=qualifiers)

    if kind == cindex.TypeKind.RVALUEREFERENCE:
        pointee = _type_from_clang(tp.get_pointee())
        return IRType(kind="rvalue_reference", to=pointee, qualifiers=qualifiers)

    if kind == cindex.TypeKind.POINTER:
        pointee = _type_from_clang(tp.get_pointee())
        if tp.get_pointee().kind == cindex.TypeKind.CHAR_S:
            return IRType(kind="cstring", qualifiers=qualifiers)
        return IRType(kind="pointer", to=pointee, qualifiers=qualifiers)

    if kind == cindex.TypeKind.CONSTANTARRAY:
        elem = _type_from_clang(tp.element_type)
        return IRType(
            kind="array", element=elem, length=tp.element_count, qualifiers=qualifiers
        )

    if kind == cindex.TypeKind.RECORD:
        return IRType(
            kind="struct",
            name=_unqualified_type_name(tp.spelling),
            qualifiers=qualifiers,
        )

    if kind == cindex.TypeKind.ENUM:
        return IRType(
            kind="enum",
            name=_unqualified_type_name(tp.spelling),
            qualifiers=qualifiers,
        )

    if kind in (cindex.TypeKind.ELABORATED, cindex.TypeKind.UNEXPOSED):
        canonical = tp.get_canonical()
        if canonical.kind != kind:
            return _type_from_clang(canonical)
        return IRType(
            kind="unknown",
            name=_unqualified_type_name(tp.spelling),
            qualifiers=qualifiers,
        )

    if kind == cindex.TypeKind.TYPEDEF:
        # Check known size typedefs
        lower = spelling.lower()
        ir_kind = _PRIMITIVE_TYPE_BY_NAME.get(lower)
        if ir_kind is not None:
            return IRType(kind=ir_kind, qualifiers=qualifiers)
        return IRType(
            kind="alias",
            name=_unqualified_type_name(tp.spelling),
            qualifiers=qualifiers,
        )

    return IRType(
        kind="unknown",
        name=_unqualified_type_name(tp.spelling),
        qualifiers=qualifiers,
    )


def _qualified_name(cursor) -> Optional[str]:
    from clang import cindex  # type: ignore

    parts = []
    current = cursor
    while current and current.kind != cindex.CursorKind.TRANSLATION_UNIT:
        if current.spelling and current.kind in (
            cindex.CursorKind.NAMESPACE,
            cindex.CursorKind.STRUCT_DECL,
            cindex.CursorKind.CLASS_DECL,
            cindex.CursorKind.ENUM_DECL,
            cindex.CursorKind.FUNCTION_DECL,
        ):
            parts.append(current.spelling)
        current = current.semantic_parent
    if not parts:
        return None
    return "::".join(reversed(parts))


def _struct_from_cursor(cursor, name_override: Optional[str] = None) -> IRStruct:
    from clang import cindex  # type: ignore

    fields = []
    for field in cursor.get_children():
        if field.kind != cindex.CursorKind.FIELD_DECL:
            continue
        fields.append(
            IRField(
                name=field.spelling,
                type=_type_from_clang(field.type),
            )
        )
    name = name_override or cursor.spelling
    return IRStruct(
        name=name,
        fields=fields,
        qualified_name=_qualified_name(cursor),
    )


def _enum_from_cursor(cursor, name_override: Optional[str] = None) -> IREnum:
    from clang import cindex  # type: ignore

    values = []
    for child in cursor.get_children():
        if child.kind != cindex.CursorKind.ENUM_CONSTANT_DECL:
            continue
        values.append(IREnumValue(name=child.spelling, value=child.enum_value))
    name = name_override or cursor.spelling
    return IREnum(
        name=name,
        values=values,
        scoped=cursor.is_scoped_enum(),
        qualified_name=_qualified_name(cursor),
    )


def _method_from_cursor(cursor, kind: str = "method") -> IRMethod:
    params = []
    for arg in cursor.get_arguments():
        params.append(IRParam(name=arg.spelling, type=_type_from_clang(arg.type)))
    variadic = False
    if hasattr(cursor, "type") and cursor.type is not None:
        if hasattr(cursor.type, "is_function_variadic"):
            variadic = cursor.type.is_function_variadic()
    return IRMethod(
        name=cursor.spelling,
        return_type=_type_from_clang(cursor.result_type),
        params=params,
        kind=kind,
        static=cursor.is_static_method(),
        const=cursor.is_const_method(),
        access=str(cursor.access_specifier).split(".")[-1].lower()
        if cursor.access_specifier
        else None,
        variadic=variadic,
    )


def _decl_index(cursor) -> int:
    location = getattr(cursor, "location", None)
    line = getattr(location, "line", 0) or 0
    column = getattr(location, "column", 0) or 0
    return (line * 1000) + column


def _make_class(cursor, fields, methods, bases, source_path, allow, deny):
    """Build an IRClass from a class/struct cursor."""
    name = cursor.spelling
    return IRClass(
        name=name,
        source_path=source_path,
        qualified_name=_qualified_name(cursor),
        decl_index=_decl_index(cursor),
        fields=fields,
        bases=bases,
        methods=methods,
    )


def _visit_normalizer(
    cursor,
    *,
    module: IRModule,
    allow: list,
    deny: list,
    exclude: set[str],
    _is_cpp_record,
) -> None:
    """Visit a cursor and populate the IR module."""
    from clang import cindex  # type: ignore

    if cursor.location.file is None:
        return
    if _is_excluded(cursor, exclude):
        return
    if _is_ignored(cursor):
        return

    source_path = _source_path(cursor)

    # ---- Namespace / linkage spec: recurse into children ----
    if cursor.kind in (
        cindex.CursorKind.NAMESPACE,
        cindex.CursorKind.LINKAGE_SPEC,
        cindex.CursorKind.UNEXPOSED_DECL,
    ):
        for child in cursor.get_children():
            _visit_normalizer(
                child,
                module=module,
                allow=allow,
                deny=deny,
                exclude=exclude,
                _is_cpp_record=_is_cpp_record,
            )
        return

    if cursor.kind == cindex.CursorKind.STRUCT_DECL and cursor.is_definition():
        if source_path is None:
            return
        name = cursor.spelling
        if not name or not _passes_name_filters(name, allow, deny):
            return

        if _is_cpp_record(cursor):
            fields, methods, bases = _extract_class_members(cursor, allow, deny)
            klass = _make_class(cursor, fields, methods, bases, source_path, allow, deny)
            _bucket_for(source_path, module).items.append(klass)
        else:
            item = _struct_from_cursor(cursor)
            item.source_path = source_path
            item.decl_index = _decl_index(cursor)
            _bucket_for(source_path, module).items.append(item)
        return

    if cursor.kind == cindex.CursorKind.CLASS_DECL and cursor.is_definition():
        if source_path is None:
            return
        name = cursor.spelling
        if not name or not _passes_name_filters(name, allow, deny):
            return
        fields, methods, bases = _extract_class_members(cursor, allow, deny)
        klass = _make_class(cursor, fields, methods, bases, source_path, allow, deny)
        _bucket_for(source_path, module).items.append(klass)
        return

    if cursor.kind == cindex.CursorKind.ENUM_DECL:
        if source_path is None:
            return
        name = cursor.spelling
        if not name or not _passes_name_filters(name, allow, deny):
            return
        item = _enum_from_cursor(cursor)
        item.source_path = source_path
        item.decl_index = _decl_index(cursor)
        _bucket_for(source_path, module).items.append(item)
        return

    if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
        if source_path is None:
            return
        name = cursor.spelling
        if not name or not _passes_name_filters(name, allow, deny):
            return
        if name.startswith("operator"):
            return
        params = []
        for arg in cursor.get_arguments():
            params.append(IRParam(name=arg.spelling, type=_type_from_clang(arg.type)))
        variadic = False
        if hasattr(cursor, "type") and cursor.type is not None:
            if hasattr(cursor.type, "is_function_variadic"):
                variadic = cursor.type.is_function_variadic()
        _bucket_for(source_path, module).items.append(
            IRFunction(
                name=name,
                source_path=source_path,
                qualified_name=_qualified_name(cursor),
                decl_index=_decl_index(cursor),
                return_type=_type_from_clang(cursor.result_type),
                params=params,
                variadic=variadic,
            )
        )
        return

    if cursor.kind == cindex.CursorKind.TYPEDEF_DECL:
        if source_path is None:
            return
        name = cursor.spelling
        if not name or not _passes_name_filters(name, allow, deny):
            return
        underlying = cursor.underlying_typedef_type
        decl = underlying.get_declaration()
        if (
            decl
            and decl.kind == cindex.CursorKind.STRUCT_DECL
            and decl.is_definition()
        ):
            if not decl.spelling:
                decl_source_path = _source_path(decl)
                if decl_source_path is None:
                    return
                item = _struct_from_cursor(decl, name_override=name)
                item.source_path = decl_source_path
                item.decl_index = _decl_index(cursor)
                _bucket_for(decl_source_path, module).items.append(item)
                return
        if decl and decl.kind == cindex.CursorKind.ENUM_DECL:
            if not decl.spelling:
                decl_source_path = _source_path(decl)
                if decl_source_path is None:
                    return
                item = _enum_from_cursor(decl, name_override=name)
                item.source_path = decl_source_path
                item.decl_index = _decl_index(cursor)
                _bucket_for(decl_source_path, module).items.append(item)
                return
        _bucket_for(source_path, module).items.append(
            IRAlias(
                name=name,
                source_path=source_path,
                decl_index=_decl_index(cursor),
                target=_type_from_clang(cursor.underlying_typedef_type),
            )
        )
        return

    if cursor.kind == cindex.CursorKind.MACRO_DEFINITION:
        if source_path is None:
            return
        if not cursor.spelling:
            return
        tokens = [t.spelling for t in cursor.get_tokens()]
        if len(tokens) == 2:
            name, value = tokens
            if not _passes_name_filters(name, allow, deny):
                return
            parsed = _parse_macro_value(value)
            if parsed is None:
                return
            _bucket_for(source_path, module).items.append(
                IRConstant(
                    name=name,
                    source_path=source_path,
                    decl_index=_decl_index(cursor),
                    type=parsed[0],
                    value=parsed[1],
                )
            )
        return


def _is_cpp_record(node) -> bool:
    """Check if a struct cursor has C++ features (methods, bases, etc.)."""
    from clang import cindex  # type: ignore

    for child in node.get_children():
        if child.kind in (
            cindex.CursorKind.CXX_METHOD,
            cindex.CursorKind.CONSTRUCTOR,
            cindex.CursorKind.DESTRUCTOR,
            cindex.CursorKind.CXX_BASE_SPECIFIER,
        ):
            return True
    return False


def _bucket_for(path: Optional[str], module: IRModule) -> IRFile:
    """Get or create an IRFile bucket for the given source path."""
    key = path or "unknown"
    bucket = module.files.get(key)
    if bucket is None:
        bucket = IRFile(items=[])
        module.files[key] = bucket
    return bucket


def _source_path(cursor) -> Optional[str]:
    """Resolve the source path for a cursor relative to the project root."""
    path = _cursor_path(cursor)
    if not path or path == "unknown":
        return None
    return path


def normalize_translation_unit(tu, cfg: BindgenConfig) -> IRModule:
    from clang import cindex  # type: ignore

    allow, deny, exclude = _compile_filters(cfg)
    module = IRModule()

    for cursor in tu.cursor.get_children():
        _visit_normalizer(
            cursor,
            module=module,
            allow=allow,
            deny=deny,
            exclude=exclude,
            _is_cpp_record=_is_cpp_record,
        )

    return module


def _extract_class_members(cursor, allow, deny):
    """Extract public fields, methods, and base classes from a struct/class cursor.

    Shared by both STRUCT_DECL and CLASS_DECL visit handlers to avoid
    ~90 lines of duplicated traversal logic.
    """
    from clang import cindex  # type: ignore

    fields: list[IRField] = []
    methods: list[IRMethod] = []
    bases: list[str] = []
    name = cursor.spelling

    for child in cursor.get_children():
        if child.kind == cindex.CursorKind.CXX_BASE_SPECIFIER:
            base = child.type.spelling
            if base:
                bases.append(base)
        elif child.kind == cindex.CursorKind.FIELD_DECL:
            if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                continue
            if not child.spelling or not _passes_name_filters(child.spelling, allow, deny):
                continue
            fields.append(
                IRField(name=child.spelling, type=_type_from_clang(child.type))
            )
        elif child.kind == cindex.CursorKind.CXX_METHOD:
            if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                continue
            if (
                not child.spelling
                or child.spelling.startswith("operator")
                or not _passes_name_filters(child.spelling, allow, deny)
            ):
                continue
            methods.append(_method_from_cursor(child))
        elif child.kind == cindex.CursorKind.CONSTRUCTOR:
            if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                continue
            if not _passes_name_filters(name, allow, deny):
                continue
            ctor = _method_from_cursor(child, kind="constructor")
            ctor.name = name
            ctor.return_type = IRType(kind="void")
            methods.append(ctor)
        elif child.kind == cindex.CursorKind.DESTRUCTOR:
            if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                continue
            if not _passes_name_filters(name, allow, deny):
                continue
            dtor = _method_from_cursor(child, kind="destructor")
            dtor.name = f"~{name}"
            dtor.return_type = IRType(kind="void")
            methods.append(dtor)

    return fields, methods, bases


def _parse_macro_value(value: str) -> Optional[tuple[IRType, int | float | str]]:
    if value.isdigit():
        return IRType(kind="int32"), int(value)
    try:
        if value.startswith("0x") or value.startswith("0X"):
            return IRType(kind="int32"), int(value, 16)
        if value.replace(".", "", 1).isdigit():
            return IRType(kind="float64"), float(value)
    except ValueError:
        return None
    if value.startswith('"') and value.endswith('"'):
        return IRType(kind="cstring"), value.strip('"')
    return None
