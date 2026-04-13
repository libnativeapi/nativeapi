from __future__ import annotations

import json
from dataclasses import dataclass, field, fields, is_dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Union

from ..config import MappingConfig
from ..ir.model import (
    IRAlias,
    IRClass,
    IRConstant,
    IREnum,
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
from .naming import NameTransformer, to_snake_case


@dataclass
class MappedType:
    """A type that has been mapped to the target language."""

    mapped: str
    raw: IRType
    kind: str = ""
    name: Optional[str] = None
    is_pointer: bool = False
    is_array: bool = False
    is_void: bool = False
    is_const: bool = False
    inner: Optional["MappedType"] = None
    element: Optional["MappedType"] = None
    length: Optional[int] = None
    native_type: str = ""
    bridge_kind: str = "plain"

    def __str__(self) -> str:
        return self.mapped


@dataclass
class MappedField:
    """A field with mapped type."""

    name: str
    type: MappedType
    raw: Optional[IRField] = None


@dataclass
class MappedParam:
    """A parameter with mapped type."""

    name: str
    type: MappedType
    api_type: str = ""
    call_args: List[str] = field(default_factory=list)
    nullable: bool = False
    direction: Optional[str] = None
    raw: Optional[IRParam] = None


@dataclass
class MappedStruct:
    """A struct with mapped field types."""

    kind: str = "struct"
    name: str = ""
    fields: List[MappedField] = field(default_factory=list)
    qualified_name: Optional[str] = None
    native_type: str = ""
    free_symbol: str = ""
    has_string_fields: bool = False
    raw: Optional[IRStruct] = None


@dataclass
class MappedEnumValue:
    """An enum value."""

    name: str = ""
    value: int = 0


@dataclass
class MappedEnum:
    """An enum (no type mapping needed, but consistent interface)."""

    kind: str = "enum"
    name: str = ""
    values: List[MappedEnumValue] = field(default_factory=list)
    scoped: bool = False
    qualified_name: Optional[str] = None
    native_type: str = ""
    raw: Optional[IREnum] = None


@dataclass
class MappedAlias:
    """A type alias with mapped target type."""

    kind: str = "alias"
    name: str = ""
    target: Optional[MappedType] = None
    raw: Optional[IRAlias] = None


@dataclass
class MappedFunction:
    """A function with mapped types and generation-ready metadata."""

    kind: str = "function"
    name: str = ""
    api_name: str = ""
    return_type: Optional[MappedType] = None
    api_return_type: str = ""
    return_bridge: str = "void"
    params: List[MappedParam] = field(default_factory=list)
    call_symbol: str = ""
    call_args: List[str] = field(default_factory=list)
    pre_call_lines: List[str] = field(default_factory=list)
    post_call_lines: List[str] = field(default_factory=list)
    string_free_symbol: str = ""
    struct_free_symbol: str = ""
    native_return_type: str = ""
    callconv: Optional[str] = None
    variadic: bool = False
    qualified_name: Optional[str] = None
    raw: Optional[IRFunction] = None


@dataclass
class MappedMethod:
    """A method with mapped types and generation-ready metadata."""

    name: str = ""
    raw_name: str = ""
    api_name: str = ""
    return_type: Optional[MappedType] = None
    api_return_type: str = ""
    return_bridge: str = "void"
    params: List[MappedParam] = field(default_factory=list)
    call_symbol: str = ""
    call_args: List[str] = field(default_factory=list)
    pre_call_lines: List[str] = field(default_factory=list)
    post_call_lines: List[str] = field(default_factory=list)
    string_free_symbol: str = ""
    struct_free_symbol: str = ""
    native_return_type: str = ""
    skip: bool = False
    is_property: bool = False
    property_name: str = ""
    method_kind: str = "method"
    static: bool = False
    needs_instance_handle: bool = False
    const: bool = False
    access: Optional[str] = None
    variadic: bool = False
    raw: Optional[IRMethod] = None


@dataclass
class MappedClass:
    """A class with mapped types."""

    kind: str = "class"
    name: str = ""
    fields: List[MappedField] = field(default_factory=list)
    methods: List[MappedMethod] = field(default_factory=list)
    bases: List[str] = field(default_factory=list)
    handle_alias: str = ""
    create_symbol: str = ""
    destroy_symbol: str = ""
    is_singleton: bool = False
    singleton_symbol: str = ""
    qualified_name: Optional[str] = None
    raw: Optional[IRClass] = None


@dataclass
class MappedConstant:
    """A constant with mapped type."""

    kind: str = "constant"
    name: str = ""
    type: Optional[MappedType] = None
    value: Any = 0
    raw: Optional[IRConstant] = None


MappedItem = Union[
    MappedStruct, MappedEnum, MappedAlias, MappedFunction, MappedClass, MappedConstant
]


@dataclass
class MappedFile:
    """A file with all mapped items in declaration order."""

    items: List[MappedItem] = field(default_factory=list)
    raw: Optional[IRFile] = None

    @property
    def types(self) -> List[MappedStruct]:
        return [item for item in self.items if isinstance(item, MappedStruct)]

    @property
    def enums(self) -> List[MappedEnum]:
        return [item for item in self.items if isinstance(item, MappedEnum)]

    @property
    def aliases(self) -> List[MappedAlias]:
        return [item for item in self.items if isinstance(item, MappedAlias)]

    @property
    def functions(self) -> List[MappedFunction]:
        return [item for item in self.items if isinstance(item, MappedFunction)]

    @property
    def classes(self) -> List[MappedClass]:
        return [item for item in self.items if isinstance(item, MappedClass)]

    @property
    def constants(self) -> List[MappedConstant]:
        return [item for item in self.items if isinstance(item, MappedConstant)]


class TypeMapper:
    """Maps IR types and callable metadata using MappingConfig."""

    def __init__(self, config: MappingConfig):
        self.config = config
        self.namer = NameTransformer(config.naming)

        options = config.options or {}
        self.symbol_overrides: Dict[str, str] = dict(options.get("symbol_overrides", {}) or {})
        self.singleton_classes = set(options.get("singleton_classes", []) or [])
        self.force_instance_methods = set(options.get("force_instance_methods", []) or [])
        self.cstring_free_symbols: Dict[str, str] = dict(options.get("cstring_free_symbols", {}) or {})
        self.bridge_types: Dict[str, str] = dict(options.get("bridge_types", {}) or {})
        self.api_type_exact: Dict[str, str] = dict(
            ((options.get("api_type_rules", {}) or {}).get("exact", {}) or {})
        )
        self.api_type_contains: List[tuple[str, str]] = [
            (str(item.get("match", "")), str(item.get("type", "")))
            for item in ((options.get("api_type_rules", {}) or {}).get("contains", []) or [])
            if item.get("match") and item.get("type")
        ]
        self.return_bridge_map: Dict[str, str] = dict(
            (options.get("return_bridge_map", {}) or {})
        )
        self.param_bridges: List[Dict[str, Any]] = list(options.get("param_bridges", []) or [])
        self.struct_free_symbols: Dict[str, str] = dict(
            (options.get("struct_free_symbols", {}) or {})
        )
        self.class_create_symbols: Dict[str, str] = dict(
            (options.get("class_create_symbols", {}) or {})
        )
        self.class_destroy_symbols: Dict[str, str] = dict(
            (options.get("class_destroy_symbols", {}) or {})
        )

    def map_type(self, ir_type: Optional[IRType]) -> MappedType:
        if ir_type is None:
            return MappedType(
                mapped="void",
                raw=IRType(kind="void"),
                kind="void",
                is_void=True,
            )

        kind = ir_type.kind
        is_const = "const" in ir_type.qualifiers

        if kind == "void":
            mapped_str = self.config.types.get("void", "void")
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_void=True,
                is_const=is_const,
            )

        if kind in ("cstring", "string"):
            mapped_str = self._lookup_type(kind, is_const)
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_const=is_const,
                native_type="ffi.Pointer<ffi.Char>" if kind == "cstring" else "",
                bridge_kind="string",
            )

        if kind == "pointer":
            inner_ir = ir_type.to
            inner_mapped = self.map_type(inner_ir) if inner_ir else None
            inner_str = inner_mapped.mapped if inner_mapped else "void"

            if inner_ir:
                if inner_ir.kind == "void":
                    ptr_key = "void*"
                elif inner_ir.kind == "primitive" and inner_ir.name:
                    ptr_key = f"{inner_ir.name}*"
                elif inner_ir.name:
                    ptr_key = f"{inner_ir.name}*"
                else:
                    ptr_key = None
            else:
                ptr_key = "void*"

            if is_const and ptr_key:
                const_ptr_key = f"const {ptr_key}"
                if const_ptr_key in self.config.types:
                    mapped_str = self.config.types[const_ptr_key]
                elif ptr_key in self.config.types:
                    mapped_str = self.config.types[ptr_key]
                elif self.config.void_pointer_type and ptr_key == "void*":
                    mapped_str = self.config.void_pointer_type
                elif self.config.const_char_pointer_type and ptr_key == "char*":
                    mapped_str = self.config.const_char_pointer_type
                else:
                    mapped_str = self.config.const_pointer_format.format(inner=inner_str)
            elif ptr_key and ptr_key in self.config.types:
                mapped_str = self.config.types[ptr_key]
            elif self.config.void_pointer_type and ptr_key == "void*":
                mapped_str = self.config.void_pointer_type
            else:
                mapped_str = self.config.pointer_format.format(inner=inner_str)

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_pointer=True,
                is_const=is_const,
                inner=inner_mapped,
            )

        if kind == "array":
            element_ir = ir_type.element
            element_mapped = self.map_type(element_ir) if element_ir else None
            element_str = element_mapped.mapped if element_mapped else "void"
            length = ir_type.length if ir_type.length is not None else 0
            mapped_str = self.config.array_format.format(element=element_str, length=length)

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_array=True,
                is_const=is_const,
                element=element_mapped,
                length=ir_type.length,
            )

        if kind == "reference":
            inner_ir = ir_type.to
            inner_mapped = self.map_type(inner_ir) if inner_ir else None
            inner_str = inner_mapped.mapped if inner_mapped else "void"
            mapped_str = self.config.reference_format.format(inner=inner_str)

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_const=is_const,
                inner=inner_mapped,
            )

        if kind == "primitive":
            type_name = ir_type.name or "int"
            mapped_str = self._lookup_type(type_name, is_const)
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                name=type_name,
                is_const=is_const,
            )

        if kind in ("named", "struct", "enum", "class", "typedef", "elaborated"):
            type_name = ir_type.name or "unknown"
            mapped_str = self._lookup_type(type_name, is_const)
            bridge_kind = "plain"
            native_type = ""
            if kind == "struct":
                bridge_kind = "struct"
                native_type = self._native_struct_type(type_name)
            elif kind == "enum":
                bridge_kind = "enum"
                native_type = self._native_enum_type(type_name)
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                name=type_name,
                is_const=is_const,
                native_type=native_type,
                bridge_kind=bridge_kind,
            )

        if kind == "function_pointer":
            mapped_str = self.config.types.get("function_pointer", "FunctionPointer")
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_const=is_const,
            )

        type_name = ir_type.name or kind
        mapped_str = self._lookup_type(type_name, is_const)
        return MappedType(
            mapped=mapped_str,
            raw=ir_type,
            kind=kind,
            name=ir_type.name,
            is_const=is_const,
        )

    def _lookup_type(self, type_name: str, is_const: bool = False) -> str:
        if is_const:
            const_key = f"const {type_name}"
            if const_key in self.config.types:
                return self.config.types[const_key]

        if type_name in self.config.types:
            mapped = self.config.types[type_name]
            return f"{self.config.type_prefix}{mapped}{self.config.type_suffix}"

        # Preserve named declarations such as structs/enums/classes in the
        # generated API surface even when passthrough_unknown is disabled.
        if type_name and type_name[:1].isupper():
            return f"{self.config.type_prefix}{type_name}{self.config.type_suffix}"

        if self.config.passthrough_unknown:
            return f"{self.config.type_prefix}{type_name}{self.config.type_suffix}"
        if self.config.default_type:
            return self.config.default_type
        return type_name

    def _api_void_type(self) -> str:
        return self.config.types.get("void", "void")

    def _native_struct_type(self, type_name: str) -> str:
        return f"native_{to_snake_case(type_name)}_t"

    def _native_enum_type(self, type_name: str) -> str:
        return f"native_{to_snake_case(type_name)}_t"

    def _default_struct_free_symbol(self, type_name: str) -> str:
        return f"native_{to_snake_case(type_name)}_free"

    def _api_type(self, mapped_type: Optional[MappedType]) -> str:
        if mapped_type is None:
            return self._api_void_type()

        mapped = mapped_type.mapped
        if mapped in self.api_type_exact:
            return self.api_type_exact[mapped]
        for match, target in self.api_type_contains:
            if match in mapped:
                return target

        return mapped

    def _return_bridge(self, api_type: str, return_type: Optional[MappedType]) -> str:
        if return_type is None or return_type.kind == "void":
            return "void"
        if return_type.kind == "enum":
            return "enum"
        if return_type.kind == "struct":
            return "struct"
        if return_type.kind in ("cstring", "string"):
            return "string"
        return self.return_bridge_map.get(api_type, "plain")

    def _symbol_for_function(self, ir_func: IRFunction) -> str:
        key = ir_func.qualified_name or ir_func.name
        return self.symbol_overrides.get(key, f"native_{to_snake_case(ir_func.name)}")

    def _symbol_for_method(self, ir_class: IRClass, ir_method: IRMethod) -> str:
        class_key = ir_class.qualified_name or ir_class.name
        key = f"{class_key}::{ir_method.name}"
        fallback = f"native_{to_snake_case(ir_class.name)}_{to_snake_case(ir_method.name)}"
        return self.symbol_overrides.get(key, fallback)

    def _symbol_for_singleton(self, ir_class: IRClass) -> str:
        class_key = ir_class.qualified_name or ir_class.name
        key = f"{class_key}::GetInstance"
        fallback = f"native_{to_snake_case(ir_class.name)}_get_instance"
        return self.symbol_overrides.get(key, fallback)

    def _symbol_for_class_create(self, ir_class: IRClass) -> str:
        class_key = ir_class.qualified_name or ir_class.name
        return self.class_create_symbols.get(
            class_key,
            f"native_{to_snake_case(ir_class.name)}_create",
        )

    def _symbol_for_class_destroy(self, ir_class: IRClass) -> str:
        class_key = ir_class.qualified_name or ir_class.name
        return self.class_destroy_symbols.get(
            class_key,
            f"native_{to_snake_case(ir_class.name)}_destroy",
        )

    def _default_bridge_type(self, key: str) -> str:
        return self.bridge_types.get(key, "")

    def _param_call_args(self, param: MappedParam, symbol: str) -> List[str]:
        name = param.name
        if param.type.kind in ("cstring", "string"):
            return [f"{name}Native"]
        for bridge in self.param_bridges:
            type_key = bridge.get("type_key")
            api_type = self._default_bridge_type(type_key) if type_key else bridge.get("api_type", "")
            if api_type and param.api_type != api_type:
                continue
            suffixes = tuple(bridge.get("symbol_suffixes", []) or [])
            if suffixes and not symbol.endswith(suffixes):
                continue
            templates = list(bridge.get("args", []) or [])
            if templates:
                return [str(t).replace("{name}", name) for t in templates]

        return [name]

    def _param_setup_lines(self, param: MappedParam) -> List[str]:
        if param.type.kind in ("cstring", "string"):
            native_name = f"{param.name}Native"
            return [
                f"final {native_name} = {param.name}.toNativeUtf8().cast<ffi.Char>();",
            ]
        return []

    def _param_cleanup_lines(self, param: MappedParam) -> List[str]:
        if param.type.kind in ("cstring", "string"):
            native_name = f"{param.name}Native"
            return [f"pkgffi.malloc.free({native_name}.cast());"]
        return []

    def _string_free_symbol(self, symbol: str) -> str:
        if not self.cstring_free_symbols:
            return "free_c_str"

        for key, free_symbol in self.cstring_free_symbols.items():
            if key == "default":
                continue
            if key and key in symbol:
                return free_symbol

        return self.cstring_free_symbols.get("default", "free_c_str")

    def _getter_property_name(self, raw_name: str) -> str:
        if not raw_name.startswith("Get") or len(raw_name) <= 3:
            return raw_name
        rest = raw_name[3:]
        return rest[:1].lower() + rest[1:]

    def map_field(self, ir_field: IRField) -> MappedField:
        return MappedField(
            name=self.namer.field_name(ir_field.name),
            type=self.map_type(ir_field.type),
            raw=ir_field,
        )

    def map_param(self, ir_param: IRParam) -> MappedParam:
        mapped_type = self.map_type(ir_param.type)
        mapped = MappedParam(
            name=self.namer.param_name(ir_param.name),
            type=mapped_type,
            api_type=self._api_type(mapped_type),
            call_args=[self.namer.param_name(ir_param.name)],
            nullable=ir_param.nullable,
            direction=ir_param.direction,
            raw=ir_param,
        )
        if mapped_type.kind in ("cstring", "string"):
            mapped.call_args = [f"{mapped.name}Native"]
        return mapped

    def map_struct(self, ir_struct: IRStruct) -> MappedStruct:
        fields = [self.map_field(f) for f in ir_struct.fields]
        has_string_fields = any(field.type.kind in ("cstring", "string") for field in fields)
        qualified_name = ir_struct.qualified_name or ir_struct.name
        return MappedStruct(
            kind="struct",
            name=self.namer.type_name(ir_struct.name),
            fields=fields,
            qualified_name=ir_struct.qualified_name,
            native_type=self._native_struct_type(ir_struct.name),
            free_symbol=self.struct_free_symbols.get(
                qualified_name,
                self.struct_free_symbols.get(
                    ir_struct.name,
                    self._default_struct_free_symbol(ir_struct.name) if has_string_fields else "",
                ),
            ),
            has_string_fields=has_string_fields,
            raw=ir_struct,
        )

    def map_enum(self, ir_enum: IREnum) -> MappedEnum:
        return MappedEnum(
            kind="enum",
            name=self.namer.enum_name(ir_enum.name),
            values=[
                MappedEnumValue(
                    name=self.namer.enum_value_name(v.name),
                    value=v.value,
                )
                for v in ir_enum.values
            ],
            scoped=ir_enum.scoped,
            qualified_name=ir_enum.qualified_name,
            native_type=self._native_enum_type(ir_enum.name),
            raw=ir_enum,
        )

    def map_alias(self, ir_alias: IRAlias) -> MappedAlias:
        return MappedAlias(
            kind="alias",
            name=self.namer.alias_name(ir_alias.name),
            target=self.map_type(ir_alias.target),
            raw=ir_alias,
        )

    def map_function(self, ir_func: IRFunction) -> MappedFunction:
        return_type = self.map_type(ir_func.return_type)
        api_return_type = self._api_type(return_type)
        call_symbol = self._symbol_for_function(ir_func)
        params = [self.map_param(p) for p in ir_func.params]

        call_args: List[str] = []
        pre_call_lines: List[str] = []
        post_call_lines: List[str] = []
        for param in params:
            pre_call_lines.extend(self._param_setup_lines(param))
            param.call_args = self._param_call_args(param, call_symbol)
            call_args.extend(param.call_args)
            post_call_lines[0:0] = self._param_cleanup_lines(param)

        return_bridge = self._return_bridge(api_return_type, return_type)
        string_free_symbol = self._string_free_symbol(call_symbol) if return_bridge == "string" else ""
        struct_free_symbol = ""
        native_return_type = return_type.native_type if return_type else ""
        if return_bridge == "struct" and return_type is not None:
            qualified = ir_func.return_type.name if ir_func.return_type else ""
            struct_free_symbol = self.struct_free_symbols.get(
                qualified,
                self._default_struct_free_symbol(qualified) if qualified else "",
            )

        api_name = self.namer.function_name(ir_func.name)
        return MappedFunction(
            kind="function",
            name=api_name,
            api_name=api_name,
            return_type=return_type,
            api_return_type=api_return_type,
            return_bridge=return_bridge,
            params=params,
            call_symbol=call_symbol,
            call_args=call_args,
            pre_call_lines=pre_call_lines,
            post_call_lines=post_call_lines,
            string_free_symbol=string_free_symbol,
            struct_free_symbol=struct_free_symbol,
            native_return_type=native_return_type,
            callconv=ir_func.callconv,
            variadic=ir_func.variadic,
            qualified_name=ir_func.qualified_name,
            raw=ir_func,
        )

    def map_method(self, ir_method: IRMethod, ir_class: IRClass) -> MappedMethod:
        return_type = self.map_type(ir_method.return_type)
        api_return_type = self._api_type(return_type)
        call_symbol = self._symbol_for_method(ir_class, ir_method)
        params = [self.map_param(p) for p in ir_method.params]
        class_key = ir_class.qualified_name or ir_class.name
        method_key = f"{class_key}::{ir_method.name}"
        force_instance = method_key in self.force_instance_methods

        call_args: List[str] = []
        pre_call_lines: List[str] = []
        post_call_lines: List[str] = []
        for param in params:
            pre_call_lines.extend(self._param_setup_lines(param))
            param.call_args = self._param_call_args(param, call_symbol)
            call_args.extend(param.call_args)
            post_call_lines[0:0] = self._param_cleanup_lines(param)

        raw_name = ir_method.name
        skip = raw_name == ir_class.name or raw_name.startswith("~") or raw_name == "GetInstance"
        is_getter = (
            raw_name.startswith("Get")
            and len(params) == 0
            and return_type is not None
            and return_type.kind != "void"
        )
        is_bool_getter = raw_name.startswith("Is") and len(params) == 0 and api_return_type.lower() == "bool"
        is_property = is_getter or is_bool_getter

        property_name = ""
        if is_getter:
            property_name = self._getter_property_name(raw_name)
        elif is_bool_getter:
            property_name = self.namer.method_name(raw_name)

        return_bridge = self._return_bridge(api_return_type, return_type)
        string_free_symbol = self._string_free_symbol(call_symbol) if return_bridge == "string" else ""
        struct_free_symbol = ""
        native_return_type = return_type.native_type if return_type else ""
        if return_bridge == "struct" and return_type is not None:
            qualified = ir_method.return_type.name if ir_method.return_type else ""
            struct_free_symbol = self.struct_free_symbols.get(
                qualified,
                self._default_struct_free_symbol(qualified) if qualified else "",
            )

        api_name = self.namer.method_name(raw_name)
        return MappedMethod(
            name=api_name,
            raw_name=raw_name,
            api_name=api_name,
            return_type=return_type,
            api_return_type=api_return_type,
            return_bridge=return_bridge,
            params=params,
            call_symbol=call_symbol,
            call_args=call_args,
            pre_call_lines=pre_call_lines,
            post_call_lines=post_call_lines,
            string_free_symbol=string_free_symbol,
            struct_free_symbol=struct_free_symbol,
            native_return_type=native_return_type,
            skip=skip,
            is_property=is_property,
            property_name=property_name,
            method_kind=ir_method.kind,
            static=ir_method.static and not force_instance,
            needs_instance_handle=(not ir_method.static) or force_instance,
            const=ir_method.const,
            access=ir_method.access,
            variadic=ir_method.variadic,
            raw=ir_method,
        )

    def map_class(self, ir_class: IRClass) -> MappedClass:
        class_name = self.namer.class_name(ir_class.name)
        is_singleton = class_name in self.singleton_classes
        singleton_symbol = self._symbol_for_singleton(ir_class) if is_singleton else ""

        return MappedClass(
            kind="class",
            name=class_name,
            fields=[self.map_field(f) for f in ir_class.fields],
            methods=[self.map_method(m, ir_class) for m in ir_class.methods],
            bases=ir_class.bases,
            handle_alias=f"native_{to_snake_case(ir_class.name)}_t",
            create_symbol=self._symbol_for_class_create(ir_class),
            destroy_symbol=self._symbol_for_class_destroy(ir_class),
            is_singleton=is_singleton,
            singleton_symbol=singleton_symbol,
            qualified_name=ir_class.qualified_name,
            raw=ir_class,
        )

    def map_constant(self, ir_const: IRConstant) -> MappedConstant:
        return MappedConstant(
            kind="constant",
            name=self.namer.constant_name(ir_const.name),
            type=self.map_type(ir_const.type),
            value=ir_const.value,
            raw=ir_const,
        )

    def map_item(self, ir_item: IRItem) -> MappedItem:
        if isinstance(ir_item, IRStruct):
            return self.map_struct(ir_item)
        if isinstance(ir_item, IREnum):
            return self.map_enum(ir_item)
        if isinstance(ir_item, IRAlias):
            return self.map_alias(ir_item)
        if isinstance(ir_item, IRFunction):
            return self.map_function(ir_item)
        if isinstance(ir_item, IRClass):
            return self.map_class(ir_item)
        if isinstance(ir_item, IRConstant):
            return self.map_constant(ir_item)
        raise ValueError(f"Unknown IR item type: {type(ir_item)}")

    def map_file(self, ir_file: IRFile) -> MappedFile:
        return MappedFile(
            items=[self.map_item(item) for item in ir_file.items],
            raw=ir_file,
        )


def build_context(module: IRModule, mapping: MappingConfig) -> Dict[str, Any]:
    mapper = TypeMapper(mapping)
    namer = NameTransformer(mapping.naming)

    files = module.files
    sorted_paths = sorted(files.keys())

    mapped_files: Dict[str, MappedFile] = {}
    for path in sorted_paths:
        mapped_files[path] = mapper.map_file(files[path])

    all_items: List[MappedItem] = []
    types: List[MappedStruct] = []
    enums: List[MappedEnum] = []
    functions: List[MappedFunction] = []
    classes: List[MappedClass] = []
    constants: List[MappedConstant] = []
    aliases: List[MappedAlias] = []

    for path in sorted_paths:
        mf = mapped_files[path]
        all_items.extend(mf.items)
        types.extend(mf.types)
        enums.extend(mf.enums)
        functions.extend(mf.functions)
        classes.extend(mf.classes)
        constants.extend(mf.constants)
        aliases.extend(mf.aliases)

    return {
        "module": module,
        "files": mapped_files,
        "file_paths": sorted_paths,
        "items": all_items,
        "types": types,
        "enums": enums,
        "functions": functions,
        "classes": classes,
        "constants": constants,
        "aliases": aliases,
        "mapping": mapping,
        "namer": namer,
        "raw": module,
    }


def _jsonify(value: Any) -> Any:
    if is_dataclass(value):
        payload: Dict[str, Any] = {}
        for item in fields(value):
            if item.name == "raw":
                continue
            payload[item.name] = _jsonify(getattr(value, item.name))
        return payload
    if isinstance(value, dict):
        return {str(key): _jsonify(val) for key, val in value.items()}
    if isinstance(value, list):
        return [_jsonify(item) for item in value]
    if isinstance(value, tuple):
        return [_jsonify(item) for item in value]
    return value


def dump_context_json(module: IRModule, mapping: MappingConfig, path: Path) -> None:
    context = build_context(module, mapping)
    files_payload = {}
    for file_path, mapped_file in context["files"].items():
        files_payload[file_path] = {
            "items": _jsonify(mapped_file.items),
            "types": _jsonify(mapped_file.types),
            "enums": _jsonify(mapped_file.enums),
            "aliases": _jsonify(mapped_file.aliases),
            "functions": _jsonify(mapped_file.functions),
            "classes": _jsonify(mapped_file.classes),
            "constants": _jsonify(mapped_file.constants),
        }
    payload = {
        "file_paths": context["file_paths"],
        "files": files_payload,
        "items": _jsonify(context["items"]),
        "types": _jsonify(context["types"]),
        "enums": _jsonify(context["enums"]),
        "functions": _jsonify(context["functions"]),
        "classes": _jsonify(context["classes"]),
        "constants": _jsonify(context["constants"]),
        "aliases": _jsonify(context["aliases"]),
        "mapping": _jsonify(mapping),
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
