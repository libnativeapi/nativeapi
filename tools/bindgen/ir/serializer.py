from __future__ import annotations

import json
from dataclasses import asdict
from pathlib import Path
from typing import Any, Dict, List, Optional

from .model import (
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


def dump_ir_json(module: IRModule, path: Path) -> None:
    payload = asdict(module)["files"]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )


def load_ir_json(path: Path) -> IRModule:
    """Load IR from a JSON file."""
    data = json.loads(path.read_text(encoding="utf-8"))
    files: Dict[str, IRFile] = {}

    for file_path, file_data in data.items():
        files[file_path] = _parse_ir_file(file_data)

    return IRModule(files=files)


def _parse_ir_type(data: Dict[str, Any]) -> IRType:
    """Parse an IRType from JSON data."""
    return IRType(
        kind=data.get("kind", ""),
        name=data.get("name"),
        to=_parse_ir_type(data["to"]) if data.get("to") else None,
        element=_parse_ir_type(data["element"]) if data.get("element") else None,
        length=data.get("length"),
        qualifiers=data.get("qualifiers", []),
        scoped=data.get("scoped"),
    )


def _parse_ir_field(data: Dict[str, Any]) -> IRField:
    """Parse an IRField from JSON data."""
    return IRField(
        name=data.get("name", ""),
        type=_parse_ir_type(data.get("type", {})),
        source_path=data.get("source_path"),
    )


def _parse_ir_struct(data: Dict[str, Any]) -> IRStruct:
    """Parse an IRStruct from JSON data."""
    return IRStruct(
        name=data.get("name", ""),
        fields=[_parse_ir_field(f) for f in data.get("fields", [])],
        qualified_name=data.get("qualified_name"),
        source_path=data.get("source_path"),
    )


def _parse_ir_enum_value(data: Dict[str, Any]) -> IREnumValue:
    """Parse an IREnumValue from JSON data."""
    return IREnumValue(
        name=data.get("name", ""),
        value=data.get("value", 0),
    )


def _parse_ir_enum(data: Dict[str, Any]) -> IREnum:
    """Parse an IREnum from JSON data."""
    return IREnum(
        name=data.get("name", ""),
        values=[_parse_ir_enum_value(v) for v in data.get("values", [])],
        scoped=data.get("scoped", False),
        qualified_name=data.get("qualified_name"),
        source_path=data.get("source_path"),
    )


def _parse_ir_alias(data: Dict[str, Any]) -> IRAlias:
    """Parse an IRAlias from JSON data."""
    return IRAlias(
        name=data.get("name", ""),
        target=_parse_ir_type(data.get("target", {})),
        source_path=data.get("source_path"),
    )


def _parse_ir_param(data: Dict[str, Any]) -> IRParam:
    """Parse an IRParam from JSON data."""
    return IRParam(
        name=data.get("name", ""),
        type=_parse_ir_type(data.get("type", {})),
        nullable=data.get("nullable", False),
        direction=data.get("direction"),
    )


def _parse_ir_function(data: Dict[str, Any]) -> IRFunction:
    """Parse an IRFunction from JSON data."""
    return IRFunction(
        name=data.get("name", ""),
        return_type=_parse_ir_type(data.get("return_type", {})),
        params=[_parse_ir_param(p) for p in data.get("params", [])],
        callconv=data.get("callconv"),
        variadic=data.get("variadic", False),
        qualified_name=data.get("qualified_name"),
        source_path=data.get("source_path"),
    )


def _parse_ir_method(data: Dict[str, Any]) -> IRMethod:
    """Parse an IRMethod from JSON data."""
    return IRMethod(
        name=data.get("name", ""),
        return_type=_parse_ir_type(data.get("return_type", {})),
        params=[_parse_ir_param(p) for p in data.get("params", [])],
        kind=data.get("kind", "method"),
        static=data.get("static", False),
        const=data.get("const", False),
        access=data.get("access"),
        variadic=data.get("variadic", False),
        source_path=data.get("source_path"),
    )


def _parse_ir_class(data: Dict[str, Any]) -> IRClass:
    """Parse an IRClass from JSON data."""
    return IRClass(
        name=data.get("name", ""),
        fields=[_parse_ir_field(f) for f in data.get("fields", [])],
        methods=[_parse_ir_method(m) for m in data.get("methods", [])],
        bases=data.get("bases", []),
        qualified_name=data.get("qualified_name"),
        source_path=data.get("source_path"),
    )


def _parse_ir_constant(data: Dict[str, Any]) -> IRConstant:
    """Parse an IRConstant from JSON data."""
    return IRConstant(
        name=data.get("name", ""),
        type=_parse_ir_type(data.get("type", {})),
        value=data.get("value", 0),
        source_path=data.get("source_path"),
    )


def _parse_ir_file(data: Dict[str, Any]) -> IRFile:
    """Parse an IRFile from JSON data."""
    return IRFile(
        types=[_parse_ir_struct(t) for t in data.get("types", [])],
        enums=[_parse_ir_enum(e) for e in data.get("enums", [])],
        functions=[_parse_ir_function(f) for f in data.get("functions", [])],
        classes=[_parse_ir_class(c) for c in data.get("classes", [])],
        constants=[_parse_ir_constant(c) for c in data.get("constants", [])],
        aliases=[_parse_ir_alias(a) for a in data.get("aliases", [])],
    )
