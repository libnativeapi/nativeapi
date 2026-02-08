from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Optional


@dataclass
class IRType:
    kind: str
    name: Optional[str] = None
    to: Optional["IRType"] = None
    element: Optional["IRType"] = None
    length: Optional[int] = None
    qualifiers: List[str] = field(default_factory=list)
    scoped: Optional[bool] = None


@dataclass
class IRField:
    name: str
    type: IRType


@dataclass
class IRStruct:
    name: str
    fields: List[IRField]
    qualified_name: Optional[str] = None


@dataclass
class IREnumValue:
    name: str
    value: int


@dataclass
class IREnum:
    name: str
    values: List[IREnumValue]
    scoped: bool = False
    qualified_name: Optional[str] = None


@dataclass
class IRAlias:
    name: str
    target: IRType


@dataclass
class IRParam:
    name: str
    type: IRType
    nullable: bool = False
    direction: Optional[str] = None


@dataclass
class IRFunction:
    name: str
    return_type: IRType
    params: List[IRParam]
    callconv: Optional[str] = None
    variadic: bool = False
    qualified_name: Optional[str] = None


@dataclass
class IRMethod:
    name: str
    return_type: IRType
    params: List[IRParam]
    static: bool = False
    const: bool = False
    access: Optional[str] = None
    variadic: bool = False


@dataclass
class IRClass:
    name: str
    methods: List[IRMethod] = field(default_factory=list)
    qualified_name: Optional[str] = None


@dataclass
class IRConstant:
    name: str
    type: IRType
    value: int | float | str


@dataclass
class IRModule:
    headers: List[str]
    types: List[IRStruct] = field(default_factory=list)
    enums: List[IREnum] = field(default_factory=list)
    functions: List[IRFunction] = field(default_factory=list)
    classes: List[IRClass] = field(default_factory=list)
    constants: List[IRConstant] = field(default_factory=list)
    aliases: List[IRAlias] = field(default_factory=list)
