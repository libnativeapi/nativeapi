from __future__ import annotations

import re
from pathlib import Path
from typing import Iterable, Optional

from .config import BindgenConfig
from .ir.model import (
    IRAlias,
    IRClass,
    IRConstant,
    IREnum,
    IREnumValue,
    IRField,
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


def _type_from_clang(tp) -> IRType:
    from clang import cindex  # type: ignore

    qualifiers = []
    if tp.is_const_qualified():
        qualifiers.append("const")
    if tp.is_volatile_qualified():
        qualifiers.append("volatile")

    kind = tp.kind
    if kind == cindex.TypeKind.VOID:
        return IRType(kind="void", qualifiers=qualifiers)
    if kind == cindex.TypeKind.BOOL:
        return IRType(kind="bool", qualifiers=qualifiers)
    if kind == cindex.TypeKind.CHAR_S:
        return IRType(kind="int8", qualifiers=qualifiers)
    if kind == cindex.TypeKind.SCHAR:
        return IRType(kind="int8", qualifiers=qualifiers)
    if kind == cindex.TypeKind.UCHAR:
        return IRType(kind="uint8", qualifiers=qualifiers)
    if kind == cindex.TypeKind.SHORT:
        return IRType(kind="int16", qualifiers=qualifiers)
    if kind == cindex.TypeKind.USHORT:
        return IRType(kind="uint16", qualifiers=qualifiers)
    if kind == cindex.TypeKind.INT:
        return IRType(kind="int32", qualifiers=qualifiers)
    if kind == cindex.TypeKind.UINT:
        return IRType(kind="uint32", qualifiers=qualifiers)
    if kind == cindex.TypeKind.LONG:
        return IRType(kind="int64", qualifiers=qualifiers)
    if kind == cindex.TypeKind.ULONG:
        return IRType(kind="uint64", qualifiers=qualifiers)
    if kind == cindex.TypeKind.LONGLONG:
        return IRType(kind="int64", qualifiers=qualifiers)
    if kind == cindex.TypeKind.ULONGLONG:
        return IRType(kind="uint64", qualifiers=qualifiers)
    if kind == cindex.TypeKind.FLOAT:
        return IRType(kind="float32", qualifiers=qualifiers)
    if kind == cindex.TypeKind.DOUBLE:
        return IRType(kind="float64", qualifiers=qualifiers)

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
        return IRType(kind="struct", name=tp.spelling, qualifiers=qualifiers)

    if kind == cindex.TypeKind.ENUM:
        return IRType(kind="enum", name=tp.spelling, qualifiers=qualifiers)

    if kind == cindex.TypeKind.TYPEDEF:
        if tp.spelling == "size_t":
            return IRType(kind="size_t", qualifiers=qualifiers)
        if tp.spelling == "ssize_t":
            return IRType(kind="ssize_t", qualifiers=qualifiers)
        if tp.spelling == "intptr_t":
            return IRType(kind="intptr", qualifiers=qualifiers)
        if tp.spelling == "uintptr_t":
            return IRType(kind="uintptr", qualifiers=qualifiers)
        return IRType(kind="alias", name=tp.spelling, qualifiers=qualifiers)

    return IRType(kind="unknown", name=tp.spelling, qualifiers=qualifiers)


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
        fields.append(IRField(name=field.spelling, type=_type_from_clang(field.type)))
    name = name_override or cursor.spelling
    return IRStruct(name=name, fields=fields, qualified_name=_qualified_name(cursor))


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


def _method_from_cursor(cursor) -> IRMethod:
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
        static=cursor.is_static_method(),
        const=cursor.is_const_method(),
        access=str(cursor.access_specifier).split(".")[-1].lower()
        if cursor.access_specifier
        else None,
        variadic=variadic,
    )


def normalize_translation_unit(tu, cfg: BindgenConfig) -> IRModule:
    from clang import cindex  # type: ignore

    allow, deny, exclude = _compile_filters(cfg)
    module = IRModule(headers=cfg.entry_headers)

    def _is_cpp_record(node) -> bool:
        for child in node.get_children():
            if child.kind in (
                cindex.CursorKind.CXX_METHOD,
                cindex.CursorKind.CONSTRUCTOR,
                cindex.CursorKind.DESTRUCTOR,
                cindex.CursorKind.CXX_BASE_SPECIFIER,
            ):
                return True
        return False

    def _visit(cursor) -> None:
        if cursor.location.file is None:
            return
        if _is_excluded(cursor, exclude):
            return
        if _is_ignored(cursor):
            return

        if cursor.kind in (
            cindex.CursorKind.NAMESPACE,
            cindex.CursorKind.LINKAGE_SPEC,
            cindex.CursorKind.UNEXPOSED_DECL,
        ):
            for child in cursor.get_children():
                _visit(child)
            return

        if cursor.kind == cindex.CursorKind.STRUCT_DECL and cursor.is_definition():
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                return
            if _is_cpp_record(cursor):
                methods = []
                for child in cursor.get_children():
                    if child.kind == cindex.CursorKind.CXX_METHOD:
                        if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                            continue
                        if not child.spelling or not _passes_name_filters(
                            child.spelling, allow, deny
                        ):
                            continue
                        methods.append(_method_from_cursor(child))
                module.classes.append(
                    IRClass(
                        name=name,
                        qualified_name=_qualified_name(cursor),
                        methods=methods,
                    )
                )
                return
            module.types.append(_struct_from_cursor(cursor))
            return

        if cursor.kind == cindex.CursorKind.CLASS_DECL and cursor.is_definition():
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                return
            methods = []
            for child in cursor.get_children():
                if child.kind == cindex.CursorKind.CXX_METHOD:
                    if child.access_specifier != cindex.AccessSpecifier.PUBLIC:
                        continue
                    if not child.spelling or not _passes_name_filters(
                        child.spelling, allow, deny
                    ):
                        continue
                    methods.append(_method_from_cursor(child))
            module.classes.append(
                IRClass(
                    name=name,
                    qualified_name=_qualified_name(cursor),
                    methods=methods,
                )
            )
            return

        if cursor.kind == cindex.CursorKind.ENUM_DECL:
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                return
            module.enums.append(_enum_from_cursor(cursor))
            return

        if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                return
            params = []
            for arg in cursor.get_arguments():
                params.append(
                    IRParam(name=arg.spelling, type=_type_from_clang(arg.type))
                )
            variadic = False
            if hasattr(cursor, "type") and cursor.type is not None:
                if hasattr(cursor.type, "is_function_variadic"):
                    variadic = cursor.type.is_function_variadic()
            module.functions.append(
                IRFunction(
                    name=name,
                    qualified_name=_qualified_name(cursor),
                    return_type=_type_from_clang(cursor.result_type),
                    params=params,
                    variadic=variadic,
                )
            )
            return

        if cursor.kind == cindex.CursorKind.TYPEDEF_DECL:
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
                    module.types.append(_struct_from_cursor(decl, name_override=name))
                    return
            if decl and decl.kind == cindex.CursorKind.ENUM_DECL:
                if not decl.spelling:
                    module.enums.append(_enum_from_cursor(decl, name_override=name))
                    return
            module.aliases.append(
                IRAlias(
                    name=name, target=_type_from_clang(cursor.underlying_typedef_type)
                )
            )
            return

        if cursor.kind == cindex.CursorKind.MACRO_DEFINITION:
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
                module.constants.append(
                    IRConstant(name=name, type=parsed[0], value=parsed[1])
                )
            return

    for cursor in tu.cursor.get_children():
        _visit(cursor)

    module.types.sort(key=lambda t: t.name)
    module.enums.sort(key=lambda e: e.name)
    module.functions.sort(key=lambda f: f.name)
    module.classes.sort(key=lambda c: c.name)
    module.constants.sort(key=lambda c: c.name)
    module.aliases.sort(key=lambda a: a.name)
    return module


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
