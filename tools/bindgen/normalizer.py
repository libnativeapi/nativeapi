from __future__ import annotations

import re
from typing import Iterable, Optional

from .config import BindgenConfig
from .ir.model import (
    IRAlias,
    IRConstant,
    IREnum,
    IREnumValue,
    IRField,
    IRFunction,
    IRModule,
    IRParam,
    IRStruct,
    IRType,
)


def _compile_filters(cfg: BindgenConfig):
    allow = [re.compile(p) for p in cfg.filters.allowlist_regex]
    deny = [re.compile(p) for p in cfg.filters.denylist_regex]
    return allow, deny


def _passes_name_filters(name: str, allow, deny) -> bool:
    if allow and not any(r.search(name) for r in allow):
        return False
    if deny and any(r.search(name) for r in deny):
        return False
    return True


def _has_export_macro(cursor, macro: Optional[str]) -> bool:
    if not macro:
        return True
    tokens = [t.spelling for t in cursor.get_tokens()]
    return macro in tokens


def _is_ignored(cursor) -> bool:
    if cursor.raw_comment and "bindgen:ignore" in cursor.raw_comment:
        return True
    return False


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


def normalize_translation_unit(tu, cfg: BindgenConfig) -> IRModule:
    from clang import cindex  # type: ignore

    allow, deny = _compile_filters(cfg)
    module = IRModule(headers=cfg.entry_headers)

    for cursor in tu.cursor.get_children():
        if cursor.location.file is None:
            continue
        if _is_ignored(cursor):
            continue

        if cursor.kind == cindex.CursorKind.STRUCT_DECL and cursor.is_definition():
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                continue
            fields = []
            for field in cursor.get_children():
                if field.kind != cindex.CursorKind.FIELD_DECL:
                    continue
                fields.append(
                    IRField(name=field.spelling, type=_type_from_clang(field.type))
                )
            module.types.append(IRStruct(name=name, fields=fields))

        elif cursor.kind == cindex.CursorKind.ENUM_DECL:
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                continue
            values = []
            for child in cursor.get_children():
                if child.kind != cindex.CursorKind.ENUM_CONSTANT_DECL:
                    continue
                values.append(IREnumValue(name=child.spelling, value=child.enum_value))
            module.enums.append(
                IREnum(name=name, values=values, scoped=cursor.is_scoped_enum())
            )

        elif cursor.kind == cindex.CursorKind.FUNCTION_DECL:
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                continue
            if not _has_export_macro(cursor, cfg.filters.export_macro):
                continue
            if cursor.is_variadic():
                variadic = True
            else:
                variadic = False
            params = []
            for arg in cursor.get_arguments():
                params.append(
                    IRParam(name=arg.spelling, type=_type_from_clang(arg.type))
                )
            module.functions.append(
                IRFunction(
                    name=name,
                    return_type=_type_from_clang(cursor.result_type),
                    params=params,
                    variadic=variadic,
                )
            )

        elif cursor.kind == cindex.CursorKind.TYPEDEF_DECL:
            name = cursor.spelling
            if not name or not _passes_name_filters(name, allow, deny):
                continue
            module.aliases.append(
                IRAlias(
                    name=name, target=_type_from_clang(cursor.underlying_typedef_type)
                )
            )

        elif cursor.kind == cindex.CursorKind.MACRO_DEFINITION:
            if not cursor.spelling:
                continue
            tokens = [t.spelling for t in cursor.get_tokens()]
            if len(tokens) == 2:
                name, value = tokens
                if not _passes_name_filters(name, allow, deny):
                    continue
                parsed = _parse_macro_value(value)
                if parsed is None:
                    continue
                module.constants.append(
                    IRConstant(name=name, type=parsed[0], value=parsed[1])
                )

    module.types.sort(key=lambda t: t.name)
    module.enums.sort(key=lambda e: e.name)
    module.functions.sort(key=lambda f: f.name)
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
