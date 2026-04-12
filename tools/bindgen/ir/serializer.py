from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List, Optional

from .model import (
    IRAlias,
    IRClass,
    IRConstant,
    IRDecl,
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


def _type_ref_kind(ir_type: IRType) -> str:
    builtin_kinds = {
        "void",
        "bool",
        "int8",
        "int16",
        "int32",
        "int64",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "float32",
        "float64",
        "size_t",
        "ssize_t",
        "intptr",
        "uintptr",
        "cstring",
    }
    if ir_type.kind in builtin_kinds:
        return ir_type.kind
    if ir_type.kind in {"pointer", "reference", "rvalue_reference", "array"}:
        return ir_type.kind
    if ir_type.kind == "alias":
        return "alias_ref"
    if ir_type.kind in {"struct", "enum", "class", "named", "typedef", "elaborated", "unknown"}:
        return "named"
    return ir_type.kind


def _serialize_type(ir_type: Optional[IRType]) -> Dict[str, Any]:
    if ir_type is None:
        return {"kind": "void"}

    kind = _type_ref_kind(ir_type)
    payload: Dict[str, Any] = {"kind": kind}

    if kind in {"pointer", "reference", "rvalue_reference"}:
        payload["to"] = _serialize_type(ir_type.to)
    elif kind == "array":
        payload["element"] = _serialize_type(ir_type.element)
        payload["length"] = ir_type.length if ir_type.length is not None else 0
    elif kind in {"named", "alias_ref"}:
        payload["name"] = ir_type.name or ""
        original_kind = ir_type.kind
        if original_kind not in {"named", "unknown"}:
            payload["type_kind"] = original_kind

    if ir_type.qualifiers:
        payload["qualifiers"] = list(ir_type.qualifiers)
    if ir_type.scoped is not None:
        payload["scoped"] = ir_type.scoped

    return payload


def _serialize_field(field: IRField) -> Dict[str, Any]:
    return {
        "name": field.name,
        "type": _serialize_type(field.type),
    }


def _serialize_param(param: IRParam) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "name": param.name,
        "type": _serialize_type(param.type),
    }
    if param.nullable:
        payload["nullable"] = True
    if param.direction:
        payload["direction"] = param.direction
    return payload


def _serialize_method(method: IRMethod) -> Dict[str, Any]:
    payload: Dict[str, Any] = {
        "name": method.name,
        "kind": method.kind,
        "return_type": _serialize_type(method.return_type),
        "params": [_serialize_param(param) for param in method.params],
    }
    if method.static:
        payload["static"] = True
    if method.const:
        payload["const"] = True
    if method.access:
        payload["access"] = method.access
    if method.variadic:
        payload["variadic"] = True
    return payload


def _serialize_decl_metadata(item: IRDecl) -> Dict[str, Any]:
    return {
        "kind": getattr(item, "kind"),
        "name": item.name,
        "source_path": item.source_path,
        "decl_index": item.decl_index,
    }


def _serialize_item(item: IRItem) -> Dict[str, Any]:
    payload = _serialize_decl_metadata(item)

    if isinstance(item, IRStruct):
        payload["fields"] = [_serialize_field(field) for field in item.fields]
        return payload
    if isinstance(item, IREnum):
        payload["values"] = [
            {"name": value.name, "value": value.value} for value in item.values
        ]
        if item.scoped:
            payload["scoped"] = True
        return payload
    if isinstance(item, IRAlias):
        payload["target"] = _serialize_type(item.target)
        return payload
    if isinstance(item, IRFunction):
        payload["return_type"] = _serialize_type(item.return_type)
        payload["params"] = [_serialize_param(param) for param in item.params]
        if item.callconv:
            payload["callconv"] = item.callconv
        if item.variadic:
            payload["variadic"] = True
        return payload
    if isinstance(item, IRClass):
        payload["fields"] = [_serialize_field(field) for field in item.fields]
        payload["methods"] = [_serialize_method(method) for method in item.methods]
        if item.bases:
            payload["bases"] = list(item.bases)
        return payload
    if isinstance(item, IRConstant):
        payload["type"] = _serialize_type(item.type)
        payload["value"] = item.value
        return payload
    raise ValueError(f"Unknown IR item type: {type(item)}")


def _serialize_file(ir_file: IRFile) -> Dict[str, List[Dict[str, Any]]]:
    payload: Dict[str, List[Dict[str, Any]]] = {}
    grouped_items = {
        "types": ir_file.types,
        "enums": ir_file.enums,
        "aliases": ir_file.aliases,
        "functions": ir_file.functions,
        "classes": ir_file.classes,
        "constants": ir_file.constants,
    }

    for key, items in grouped_items.items():
        if items:
            payload[key] = [_serialize_item(item) for item in items]

    return payload


def dump_ir_json(module: IRModule, path: Path) -> None:
    """Dump IR module to JSON file."""
    payload: Dict[str, Any] = {"files": {}}
    files_payload: Dict[str, Dict[str, List[Dict[str, Any]]]] = payload["files"]
    for file_path, ir_file in module.files.items():
        files_payload[file_path] = _serialize_file(ir_file)

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )


def _require_object(data: Any, context: str) -> Dict[str, Any]:
    if not isinstance(data, dict):
        raise ValueError(f"{context} must be an object")
    return data


def _require_list(data: Any, context: str) -> List[Any]:
    if not isinstance(data, list):
        raise ValueError(f"{context} must be an array")
    return data


def load_ir_json(path: Path) -> IRModule:
    """Load IR from a JSON file."""
    data = _require_object(json.loads(path.read_text(encoding="utf-8")), "root")
    files_data = _require_object(data.get("files"), "root.files")
    files: Dict[str, IRFile] = {}

    for file_path, file_data in files_data.items():
        files[file_path] = _parse_ir_file(
            _require_object(file_data, f"root.files[{file_path!r}]"), file_path
        )

    return IRModule(files=files)


def _parse_ir_type(data: Optional[Dict[str, Any]]) -> IRType:
    if data is None:
        return IRType(kind="void")

    payload = _require_object(data, "type")
    kind = str(payload.get("kind", ""))
    qualifiers = list(payload.get("qualifiers", []) or [])

    if kind in {"pointer", "reference", "rvalue_reference"}:
        return IRType(
            kind=kind,
            to=_parse_ir_type(payload.get("to")),
            qualifiers=qualifiers,
        )

    if kind == "array":
        return IRType(
            kind="array",
            element=_parse_ir_type(payload.get("element")),
            length=payload.get("length"),
            qualifiers=qualifiers,
        )

    if kind == "named":
        return IRType(
            kind=str(payload.get("type_kind", "named")),
            name=str(payload.get("name", "")),
            qualifiers=qualifiers,
            scoped=payload.get("scoped"),
        )

    if kind == "alias_ref":
        return IRType(
            kind="alias",
            name=str(payload.get("name", "")),
            qualifiers=qualifiers,
        )

    return IRType(
        kind=kind,
        name=payload.get("name"),
        qualifiers=qualifiers,
        scoped=payload.get("scoped"),
    )


def _parse_ir_field(data: Dict[str, Any]) -> IRField:
    payload = _require_object(data, "field")
    return IRField(
        name=str(payload.get("name", "")),
        type=_parse_ir_type(payload.get("type")),
    )


def _parse_ir_enum_value(data: Dict[str, Any]) -> IREnumValue:
    payload = _require_object(data, "enum value")
    return IREnumValue(
        name=str(payload.get("name", "")),
        value=int(payload.get("value", 0)),
    )


def _parse_ir_param(data: Dict[str, Any]) -> IRParam:
    payload = _require_object(data, "param")
    return IRParam(
        name=str(payload.get("name", "")),
        type=_parse_ir_type(payload.get("type")),
        nullable=bool(payload.get("nullable", False)),
        direction=payload.get("direction"),
    )


def _parse_ir_method(data: Dict[str, Any]) -> IRMethod:
    payload = _require_object(data, "method")
    return IRMethod(
        name=str(payload.get("name", "")),
        return_type=_parse_ir_type(payload.get("return_type")),
        params=[_parse_ir_param(param) for param in _require_list(payload.get("params", []), "method.params")],
        kind=str(payload.get("kind", "method")),
        static=bool(payload.get("static", False)),
        const=bool(payload.get("const", False)),
        access=payload.get("access"),
        variadic=bool(payload.get("variadic", False)),
    )


def _decl_metadata(data: Dict[str, Any], fallback_source_path: str) -> Dict[str, Any]:
    payload = _require_object(data, "item")
    return {
        "name": str(payload.get("name", "")),
        "source_path": str(payload.get("source_path") or fallback_source_path),
        "qualified_name": payload.get("qualified_name"),
        "decl_index": int(payload.get("decl_index", 0)),
    }


def _parse_ir_struct(data: Dict[str, Any], fallback_source_path: str) -> IRStruct:
    meta = _decl_metadata(data, fallback_source_path)
    return IRStruct(
        **meta,
        fields=[_parse_ir_field(field) for field in _require_list(data.get("fields", []), "struct.fields")],
    )


def _parse_ir_enum(data: Dict[str, Any], fallback_source_path: str) -> IREnum:
    meta = _decl_metadata(data, fallback_source_path)
    return IREnum(
        **meta,
        values=[_parse_ir_enum_value(value) for value in _require_list(data.get("values", []), "enum.values")],
        scoped=bool(data.get("scoped", False)),
    )


def _parse_ir_alias(data: Dict[str, Any], fallback_source_path: str) -> IRAlias:
    meta = _decl_metadata(data, fallback_source_path)
    return IRAlias(
        **meta,
        target=_parse_ir_type(data.get("target")),
    )


def _parse_ir_function(data: Dict[str, Any], fallback_source_path: str) -> IRFunction:
    meta = _decl_metadata(data, fallback_source_path)
    return IRFunction(
        **meta,
        return_type=_parse_ir_type(data.get("return_type")),
        params=[_parse_ir_param(param) for param in _require_list(data.get("params", []), "function.params")],
        callconv=data.get("callconv"),
        variadic=bool(data.get("variadic", False)),
    )


def _parse_ir_class(data: Dict[str, Any], fallback_source_path: str) -> IRClass:
    meta = _decl_metadata(data, fallback_source_path)
    return IRClass(
        **meta,
        fields=[_parse_ir_field(field) for field in _require_list(data.get("fields", []), "class.fields")],
        methods=[_parse_ir_method(method) for method in _require_list(data.get("methods", []), "class.methods")],
        bases=list(data.get("bases", []) or []),
    )


def _parse_ir_constant(data: Dict[str, Any], fallback_source_path: str) -> IRConstant:
    meta = _decl_metadata(data, fallback_source_path)
    return IRConstant(
        **meta,
        type=_parse_ir_type(data.get("type")),
        value=data.get("value", 0),
    )


def _parse_typed_items(
    items_data: Any,
    file_path: str,
    parser,
) -> List[IRItem]:
    items = []
    for item_data in _require_list(items_data, f"{file_path} items"):
        items.append(parser(_require_object(item_data, "item"), file_path))
    return items


def _parse_ir_file(data: Dict[str, Any], file_path: str) -> IRFile:
    return IRFile(
        types=[
            item for item in _parse_typed_items(data.get("types", []), file_path, _parse_ir_struct)
            if isinstance(item, IRStruct)
        ],
        enums=[
            item for item in _parse_typed_items(data.get("enums", []), file_path, _parse_ir_enum)
            if isinstance(item, IREnum)
        ],
        aliases=[
            item for item in _parse_typed_items(data.get("aliases", []), file_path, _parse_ir_alias)
            if isinstance(item, IRAlias)
        ],
        functions=[
            item for item in _parse_typed_items(data.get("functions", []), file_path, _parse_ir_function)
            if isinstance(item, IRFunction)
        ],
        classes=[
            item for item in _parse_typed_items(data.get("classes", []), file_path, _parse_ir_class)
            if isinstance(item, IRClass)
        ],
        constants=[
            item for item in _parse_typed_items(data.get("constants", []), file_path, _parse_ir_constant)
            if isinstance(item, IRConstant)
        ],
    )
