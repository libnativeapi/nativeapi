from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Union


@dataclass
class IRType:
    """Represents a type reference in the IR."""

    kind: str
    name: Optional[str] = None
    to: Optional["IRType"] = None
    element: Optional["IRType"] = None
    length: Optional[int] = None
    qualifiers: List[str] = field(default_factory=list)
    scoped: Optional[bool] = None


@dataclass
class IRField:
    """Represents a field in a struct or class."""

    name: str
    type: IRType


@dataclass
class IREnumValue:
    """Represents a single enum value."""

    name: str
    value: int


@dataclass
class IRParam:
    """Represents a function/method parameter."""

    name: str
    type: IRType
    nullable: bool = False
    direction: Optional[str] = None


@dataclass
class IRMethod:
    """Represents a method in a class."""

    name: str
    return_type: IRType
    params: List[IRParam] = field(default_factory=list)
    kind: str = "method"
    static: bool = False
    const: bool = False
    access: Optional[str] = None
    variadic: bool = False


@dataclass
class IRDecl:
    """Common metadata for top-level declarations."""

    kind: str = "decl"
    name: str = ""
    source_path: Optional[str] = None
    qualified_name: Optional[str] = None
    decl_index: int = 0


@dataclass
class IRStruct(IRDecl):
    """Represents a struct type definition."""

    kind: str = "struct"
    fields: List[IRField] = field(default_factory=list)


@dataclass
class IREnum(IRDecl):
    """Represents an enum type definition."""

    kind: str = "enum"
    values: List[IREnumValue] = field(default_factory=list)
    scoped: bool = False


@dataclass
class IRAlias(IRDecl):
    """Represents a type alias (typedef/using)."""

    kind: str = "alias"
    target: Optional[IRType] = None


@dataclass
class IRFunction(IRDecl):
    """Represents a function declaration."""

    kind: str = "function"
    return_type: Optional[IRType] = None
    params: List[IRParam] = field(default_factory=list)
    callconv: Optional[str] = None
    variadic: bool = False


@dataclass
class IRClass(IRDecl):
    """Represents a class/struct with methods."""

    kind: str = "class"
    fields: List[IRField] = field(default_factory=list)
    methods: List[IRMethod] = field(default_factory=list)
    bases: List[str] = field(default_factory=list)


@dataclass
class IRConstant(IRDecl):
    """Represents a constant definition."""

    kind: str = "constant"
    type: Optional[IRType] = None
    value: Union[int, float, str] = 0


# Union type for all IR items
IRItem = Union[IRStruct, IREnum, IRAlias, IRFunction, IRClass, IRConstant]


@dataclass
class IRFile:
    """
    Represents a single source file's IR.

    Items are stored in declaration order as a single list.
    Typed convenience properties (``types``, ``enums``, etc.) filter on demand.
    """

    items: List[IRItem] = field(default_factory=list)

    @property
    def types(self) -> List[IRStruct]:
        return [i for i in self.items if isinstance(i, IRStruct)]

    @property
    def enums(self) -> List[IREnum]:
        return [i for i in self.items if isinstance(i, IREnum)]

    @property
    def aliases(self) -> List[IRAlias]:
        return [i for i in self.items if isinstance(i, IRAlias)]

    @property
    def functions(self) -> List[IRFunction]:
        return [i for i in self.items if isinstance(i, IRFunction)]

    @property
    def classes(self) -> List[IRClass]:
        return [i for i in self.items if isinstance(i, IRClass)]

    @property
    def constants(self) -> List[IRConstant]:
        return [i for i in self.items if isinstance(i, IRConstant)]


@dataclass
class IRModule:
    """
    Represents the entire IR for a project.

    Files are stored as a dict mapping file path to IRFile.
    Each IRFile contains items in declaration order.
    """

    files: Dict[str, IRFile] = field(default_factory=dict)
