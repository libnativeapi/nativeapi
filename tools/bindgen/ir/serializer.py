from __future__ import annotations

import json
from dataclasses import asdict
from pathlib import Path
from typing import Any, Dict, List

from .model import (
    IRAlias,
    IRClass,
    IRConstant,
    IREnum,
    IREnumValue,
    IRField,
    IRFile,
    IRFunction,
    IRItem,
    IRMethod,
    IRModule,
    IRParam,
    IRStruct,
    IRType,
)


def _serialize_item(item: IRItem) -> Dict[str, Any]:
    """Serialize an IR item to a dict, including its kind."""
    result = asdict(item)
    return result


def _serialize_file(ir_file: IRFile) -> List[Dict[str, Any]]:
    """Serialize an IRFile to a list of items."""
    return [_serialize_item(item) for item in ir_file.items]


def dump_ir_json(module: IRModule, path: Path) -> None:
    """Dump IR module to JSON file."""
    payload: Dict[str, List[Dict[str, Any]]] = {}
    for file_path, ir_file in module.files.items():
        payload[file_path] = _serialize_file(ir_file)

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )


def load_ir_json(path: Path) -> IRModule:
    """Load IR from a JSON file."""
    data = json.loads(path.read_text(encoding="utf-8"))
    files: Dict[str, IRFile] = {}

    for file_path, items_data in data.items():
        files[file_path] = _parse_ir_file(items_data)

    return IRModule(files=files)


def _parse_ir_type(data: Dict[str, Any]) -> IRType:
    """Parse an IRType from JSON data."""
    if data is None:
        return IRType(kind="void")

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
    )


def _parse_ir_enum_value(data: Dict[str, Any]) -> IREnumValue:
    """Parse an IREnumValue from JSON data."""
    return IREnumValue(
        name=data.get("name", ""),
        value=data.get("value", 0),
    )


def _parse_ir_param(data: Dict[str, Any]) -> IRParam:
    """Parse an IRParam from JSON data."""
    return IRParam(
        name=data.get("name", ""),
        type=_parse_ir_type(data.get("type", {})),
        nullable=data.get("nullable", False),
        direction=data.get("direction"),
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
    )


def _parse_ir_struct(data: Dict[str, Any]) -> IRStruct:
    """Parse an IRStruct from JSON data."""
    item = IRStruct(
        name=data.get("name", ""),
        fields=[_parse_ir_field(f) for f in data.get("fields", [])],
        qualified_name=data.get("qualified_name"),
    )
    return item


def _parse_ir_enum(data: Dict[str, Any]) -> IREnum:
    """Parse an IREnum from JSON data."""
    item = IREnum(
        name=data.get("name", ""),
        values=[_parse_ir_enum_value(v) for v in data.get("values", [])],
        scoped=data.get("scoped", False),
        qualified_name=data.get("qualified_name"),
    )
    return item


def _parse_ir_alias(data: Dict[str, Any]) -> IRAlias:
    """Parse an IRAlias from JSON data."""
    item = IRAlias(
        name=data.get("name", ""),
        target=_parse_ir_type(data.get("target", {})),
    )
    return item


def _parse_ir_function(data: Dict[str, Any]) -> IRFunction:
    """Parse an IRFunction from JSON data."""
    item = IRFunction(
        name=data.get("name", ""),
        return_type=_parse_ir_type(data.get("return_type", {})),
        params=[_parse_ir_param(p) for p in data.get("params", [])],
        callconv=data.get("callconv"),
        variadic=data.get("variadic", False),
        qualified_name=data.get("qualified_name"),
    )
    return item


def _parse_ir_class(data: Dict[str, Any]) -> IRClass:
    """Parse an IRClass from JSON data."""
    item = IRClass(
        name=data.get("name", ""),
        fields=[_parse_ir_field(f) for f in data.get("fields", [])],
        methods=[_parse_ir_method(m) for m in data.get("methods", [])],
        bases=data.get("bases", []),
        qualified_name=data.get("qualified_name"),
    )
    return item


def _parse_ir_constant(data: Dict[str, Any]) -> IRConstant:
    """Parse an IRConstant from JSON data."""
    item = IRConstant(
        name=data.get("name", ""),
        type=_parse_ir_type(data.get("type", {})),
        value=data.get("value", 0),
    )
    return item


def _parse_ir_item(data: Dict[str, Any]) -> IRItem:
    """Parse an IR item based on its kind."""
    kind = data.get("kind", "")

    if kind == "struct":
        return _parse_ir_struct(data)
    elif kind == "enum":
        return _parse_ir_enum(data)
    elif kind == "alias":
        return _parse_ir_alias(data)
    elif kind == "function":
        return _parse_ir_function(data)
    elif kind == "class":
        return _parse_ir_class(data)
    elif kind == "constant":
        return _parse_ir_constant(data)
    else:
        raise ValueError(f"Unknown IR item kind: {kind}")


def _parse_ir_file(items_data: List[Dict[str, Any]]) -> IRFile:
    """Parse an IRFile from a list of item dicts."""
    items = [_parse_ir_item(item_data) for item_data in items_data]
    return IRFile(items=items)
