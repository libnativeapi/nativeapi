from __future__ import annotations

from dataclasses import dataclass, field
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
from .naming import NameTransformer


@dataclass
class MappedType:
    """A type that has been mapped to the target language."""

    # The mapped type string (e.g., "Pointer<Void>", "int", "double")
    mapped: str

    # Original IR type for reference
    raw: IRType

    # Convenience properties from raw type
    kind: str = ""
    name: Optional[str] = None
    is_pointer: bool = False
    is_array: bool = False
    is_void: bool = False
    is_const: bool = False

    # For pointer/reference types
    inner: Optional["MappedType"] = None

    # For array types
    element: Optional["MappedType"] = None
    length: Optional[int] = None

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
    """A function with mapped types."""

    kind: str = "function"
    name: str = ""
    return_type: Optional[MappedType] = None
    params: List[MappedParam] = field(default_factory=list)
    callconv: Optional[str] = None
    variadic: bool = False
    qualified_name: Optional[str] = None
    raw: Optional[IRFunction] = None


@dataclass
class MappedMethod:
    """A method with mapped types."""

    name: str = ""
    return_type: Optional[MappedType] = None
    params: List[MappedParam] = field(default_factory=list)
    method_kind: str = "method"
    static: bool = False
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


# Union type for all mapped items
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
        """Get all struct types in declaration order."""
        return [item for item in self.items if isinstance(item, MappedStruct)]

    @property
    def enums(self) -> List[MappedEnum]:
        """Get all enums in declaration order."""
        return [item for item in self.items if isinstance(item, MappedEnum)]

    @property
    def aliases(self) -> List[MappedAlias]:
        """Get all type aliases in declaration order."""
        return [item for item in self.items if isinstance(item, MappedAlias)]

    @property
    def functions(self) -> List[MappedFunction]:
        """Get all functions in declaration order."""
        return [item for item in self.items if isinstance(item, MappedFunction)]

    @property
    def classes(self) -> List[MappedClass]:
        """Get all classes in declaration order."""
        return [item for item in self.items if isinstance(item, MappedClass)]

    @property
    def constants(self) -> List[MappedConstant]:
        """Get all constants in declaration order."""
        return [item for item in self.items if isinstance(item, MappedConstant)]


class TypeMapper:
    """Maps IR types to target language types using MappingConfig."""

    def __init__(self, config: MappingConfig):
        self.config = config
        self.namer = NameTransformer(config.naming)

    def map_type(self, ir_type: Optional[IRType]) -> MappedType:
        """Map an IRType to a MappedType."""
        if ir_type is None:
            return MappedType(
                mapped="void",
                raw=IRType(kind="void"),
                kind="void",
                is_void=True,
            )

        kind = ir_type.kind
        is_const = "const" in ir_type.qualifiers

        # Handle void type
        if kind == "void":
            mapped_str = self.config.types.get("void", "void")
            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_void=True,
                is_const=is_const,
            )

        # Handle pointer types
        if kind == "pointer":
            inner_ir = ir_type.to
            inner_mapped = self.map_type(inner_ir) if inner_ir else None
            inner_str = inner_mapped.mapped if inner_mapped else "void"

            # Build pointer key for lookup (e.g., "void*", "char*", "int*")
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

            # Check for const pointer mapping first (e.g., "const char*")
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
                    mapped_str = self.config.const_pointer_format.format(
                        inner=inner_str
                    )
            # Check for direct pointer type mapping (e.g., "void*", "int*")
            elif ptr_key and ptr_key in self.config.types:
                mapped_str = self.config.types[ptr_key]
            # Check for special void* mapping config
            elif self.config.void_pointer_type and ptr_key == "void*":
                mapped_str = self.config.void_pointer_type
            # Fallback to pointer format
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

        # Handle array types
        if kind == "array":
            element_ir = ir_type.element
            element_mapped = self.map_type(element_ir) if element_ir else None
            element_str = element_mapped.mapped if element_mapped else "void"
            length = ir_type.length if ir_type.length is not None else 0
            mapped_str = self.config.array_format.format(
                element=element_str, length=length
            )

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_array=True,
                is_const=is_const,
                element=element_mapped,
                length=ir_type.length,
            )

        # Handle reference types
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

        # Handle primitive types
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

        # Handle named types (struct, enum, class, typedef, etc.)
        if kind in ("named", "struct", "enum", "class", "typedef", "elaborated"):
            type_name = ir_type.name or "unknown"
            mapped_str = self._lookup_type(type_name, is_const)

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                name=type_name,
                is_const=is_const,
            )

        # Handle function pointer types
        if kind == "function_pointer":
            if "function_pointer" in self.config.types:
                mapped_str = self.config.types["function_pointer"]
            else:
                mapped_str = "FunctionPointer"

            return MappedType(
                mapped=mapped_str,
                raw=ir_type,
                kind=kind,
                is_const=is_const,
            )

        # Fallback
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
        """Look up a type name in the mapping config."""
        # Check for const variant first
        if is_const:
            const_key = f"const {type_name}"
            if const_key in self.config.types:
                return self.config.types[const_key]

        # Look up in type mappings
        if type_name in self.config.types:
            mapped = self.config.types[type_name]
            return f"{self.config.type_prefix}{mapped}{self.config.type_suffix}"

        # Passthrough or default
        if self.config.passthrough_unknown:
            return f"{self.config.type_prefix}{type_name}{self.config.type_suffix}"
        elif self.config.default_type:
            return self.config.default_type
        else:
            return type_name

    def map_field(self, ir_field: IRField) -> MappedField:
        """Map an IRField to a MappedField."""
        return MappedField(
            name=self.namer.field_name(ir_field.name),
            type=self.map_type(ir_field.type),
            raw=ir_field,
        )

    def map_param(self, ir_param: IRParam) -> MappedParam:
        """Map an IRParam to a MappedParam."""
        return MappedParam(
            name=self.namer.param_name(ir_param.name),
            type=self.map_type(ir_param.type),
            nullable=ir_param.nullable,
            direction=ir_param.direction,
            raw=ir_param,
        )

    def map_struct(self, ir_struct: IRStruct) -> MappedStruct:
        """Map an IRStruct to a MappedStruct."""
        return MappedStruct(
            kind="struct",
            name=self.namer.type_name(ir_struct.name),
            fields=[self.map_field(f) for f in ir_struct.fields],
            qualified_name=ir_struct.qualified_name,
            raw=ir_struct,
        )

    def map_enum(self, ir_enum: IREnum) -> MappedEnum:
        """Map an IREnum to a MappedEnum."""
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
            raw=ir_enum,
        )

    def map_alias(self, ir_alias: IRAlias) -> MappedAlias:
        """Map an IRAlias to a MappedAlias."""
        return MappedAlias(
            kind="alias",
            name=self.namer.alias_name(ir_alias.name),
            target=self.map_type(ir_alias.target),
            raw=ir_alias,
        )

    def map_function(self, ir_func: IRFunction) -> MappedFunction:
        """Map an IRFunction to a MappedFunction."""
        return MappedFunction(
            kind="function",
            name=self.namer.function_name(ir_func.name),
            return_type=self.map_type(ir_func.return_type),
            params=[self.map_param(p) for p in ir_func.params],
            callconv=ir_func.callconv,
            variadic=ir_func.variadic,
            qualified_name=ir_func.qualified_name,
            raw=ir_func,
        )

    def map_method(self, ir_method: IRMethod) -> MappedMethod:
        """Map an IRMethod to a MappedMethod."""
        return MappedMethod(
            name=self.namer.method_name(ir_method.name),
            return_type=self.map_type(ir_method.return_type),
            params=[self.map_param(p) for p in ir_method.params],
            method_kind=ir_method.kind,
            static=ir_method.static,
            const=ir_method.const,
            access=ir_method.access,
            variadic=ir_method.variadic,
            raw=ir_method,
        )

    def map_class(self, ir_class: IRClass) -> MappedClass:
        """Map an IRClass to a MappedClass."""
        return MappedClass(
            kind="class",
            name=self.namer.class_name(ir_class.name),
            fields=[self.map_field(f) for f in ir_class.fields],
            methods=[self.map_method(m) for m in ir_class.methods],
            bases=ir_class.bases,
            qualified_name=ir_class.qualified_name,
            raw=ir_class,
        )

    def map_constant(self, ir_const: IRConstant) -> MappedConstant:
        """Map an IRConstant to a MappedConstant."""
        return MappedConstant(
            kind="constant",
            name=self.namer.constant_name(ir_const.name),
            type=self.map_type(ir_const.type),
            value=ir_const.value,
            raw=ir_const,
        )

    def map_item(self, ir_item: IRItem) -> MappedItem:
        """Map any IR item to its mapped equivalent."""
        if isinstance(ir_item, IRStruct):
            return self.map_struct(ir_item)
        elif isinstance(ir_item, IREnum):
            return self.map_enum(ir_item)
        elif isinstance(ir_item, IRAlias):
            return self.map_alias(ir_item)
        elif isinstance(ir_item, IRFunction):
            return self.map_function(ir_item)
        elif isinstance(ir_item, IRClass):
            return self.map_class(ir_item)
        elif isinstance(ir_item, IRConstant):
            return self.map_constant(ir_item)
        else:
            raise ValueError(f"Unknown IR item type: {type(ir_item)}")

    def map_file(self, ir_file: IRFile) -> MappedFile:
        """Map an IRFile to a MappedFile, preserving item order."""
        return MappedFile(
            items=[self.map_item(item) for item in ir_file.items],
            raw=ir_file,
        )


def build_context(module: IRModule, mapping: MappingConfig) -> Dict[str, Any]:
    """
    Build the template context with preprocessed (mapped) data.

    The context provides:
    - Mapped data with types already converted to target language
    - Names transformed according to naming config
    - Items in declaration order (via 'items' list)
    - Filtered accessors (types, enums, functions, etc.)
    - Raw IR data accessible via .raw attribute on each item
    - Global raw module accessible via 'raw' key
    """
    mapper = TypeMapper(mapping)
    namer = NameTransformer(mapping.naming)

    files = module.files
    sorted_paths = sorted(files.keys())

    # Map all files
    mapped_files: Dict[str, MappedFile] = {}
    for path in sorted_paths:
        mapped_files[path] = mapper.map_file(files[path])

    # Flatten all items (preserving order within each file)
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
        # All items in order
        "items": all_items,
        # Filtered by type
        "types": types,
        "enums": enums,
        "functions": functions,
        "classes": classes,
        "constants": constants,
        "aliases": aliases,
        "mapping": mapping,
        # Name transformer for custom transformations in templates
        "namer": namer,
        # Raw IR data for direct access
        "raw": module,
    }
