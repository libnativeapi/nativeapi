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


# =============================================================================
# IR Items - Each item has a 'kind' field to identify its type
# =============================================================================


@dataclass
class IRStruct:
    """Represents a struct type definition."""

    kind: str = field(default="struct", init=False)
    name: str = ""
    fields: List[IRField] = field(default_factory=list)
    qualified_name: Optional[str] = None


@dataclass
class IREnum:
    """Represents an enum type definition."""

    kind: str = field(default="enum", init=False)
    name: str = ""
    values: List[IREnumValue] = field(default_factory=list)
    scoped: bool = False
    qualified_name: Optional[str] = None


@dataclass
class IRAlias:
    """Represents a type alias (typedef/using)."""

    kind: str = field(default="alias", init=False)
    name: str = ""
    target: Optional[IRType] = None


@dataclass
class IRFunction:
    """Represents a function declaration."""

    kind: str = field(default="function", init=False)
    name: str = ""
    return_type: Optional[IRType] = None
    params: List[IRParam] = field(default_factory=list)
    callconv: Optional[str] = None
    variadic: bool = False
    qualified_name: Optional[str] = None


@dataclass
class IRClass:
    """Represents a class/struct with methods."""

    kind: str = field(default="class", init=False)
    name: str = ""
    fields: List[IRField] = field(default_factory=list)
    methods: List[IRMethod] = field(default_factory=list)
    bases: List[str] = field(default_factory=list)
    qualified_name: Optional[str] = None


@dataclass
class IRConstant:
    """Represents a constant definition."""

    kind: str = field(default="constant", init=False)
    name: str = ""
    type: Optional[IRType] = None
    value: Union[int, float, str] = 0


# Union type for all IR items
IRItem = Union[IRStruct, IREnum, IRAlias, IRFunction, IRClass, IRConstant]


@dataclass
class IRFile:
    """
    Represents a single source file's IR.

    Items are stored in declaration order, each with a 'kind' field:
    - "struct": IRStruct
    - "enum": IREnum
    - "alias": IRAlias
    - "function": IRFunction
    - "class": IRClass
    - "constant": IRConstant
    """

    items: List[IRItem] = field(default_factory=list)

    # Convenience accessors for filtered items
    @property
    def types(self) -> List[IRStruct]:
        """Get all struct types in declaration order."""
        return [item for item in self.items if isinstance(item, IRStruct)]

    @property
    def enums(self) -> List[IREnum]:
        """Get all enums in declaration order."""
        return [item for item in self.items if isinstance(item, IREnum)]

    @property
    def aliases(self) -> List[IRAlias]:
        """Get all type aliases in declaration order."""
        return [item for item in self.items if isinstance(item, IRAlias)]

    @property
    def functions(self) -> List[IRFunction]:
        """Get all functions in declaration order."""
        return [item for item in self.items if isinstance(item, IRFunction)]

    @property
    def classes(self) -> List[IRClass]:
        """Get all classes in declaration order."""
        return [item for item in self.items if isinstance(item, IRClass)]

    @property
    def constants(self) -> List[IRConstant]:
        """Get all constants in declaration order."""
        return [item for item in self.items if isinstance(item, IRConstant)]


@dataclass
class IRModule:
    """
    Represents the entire IR for a project.

    Files are stored as a dict mapping file path to IRFile.
    Each IRFile contains items in declaration order.
    """

    files: Dict[str, IRFile] = field(default_factory=dict)
