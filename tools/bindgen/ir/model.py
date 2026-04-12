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

    name: str = ""
    source_path: Optional[str] = None
    qualified_name: Optional[str] = None
    decl_index: int = 0


@dataclass
class IRStruct(IRDecl):
    """Represents a struct type definition."""

    kind: str = field(default="struct", init=False)
    fields: List[IRField] = field(default_factory=list)


@dataclass
class IREnum(IRDecl):
    """Represents an enum type definition."""

    kind: str = field(default="enum", init=False)
    values: List[IREnumValue] = field(default_factory=list)
    scoped: bool = False


@dataclass
class IRAlias(IRDecl):
    """Represents a type alias (typedef/using)."""

    kind: str = field(default="alias", init=False)
    target: Optional[IRType] = None


@dataclass
class IRFunction(IRDecl):
    """Represents a function declaration."""

    kind: str = field(default="function", init=False)
    return_type: Optional[IRType] = None
    params: List[IRParam] = field(default_factory=list)
    callconv: Optional[str] = None
    variadic: bool = False


@dataclass
class IRClass(IRDecl):
    """Represents a class/struct with methods."""

    kind: str = field(default="class", init=False)
    fields: List[IRField] = field(default_factory=list)
    methods: List[IRMethod] = field(default_factory=list)
    bases: List[str] = field(default_factory=list)


@dataclass
class IRConstant(IRDecl):
    """Represents a constant definition."""

    kind: str = field(default="constant", init=False)
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

    types: List[IRStruct] = field(default_factory=list)
    enums: List[IREnum] = field(default_factory=list)
    aliases: List[IRAlias] = field(default_factory=list)
    functions: List[IRFunction] = field(default_factory=list)
    classes: List[IRClass] = field(default_factory=list)
    constants: List[IRConstant] = field(default_factory=list)

    @property
    def items(self) -> List[IRItem]:
        """Get all items in declaration order."""
        items: List[IRItem] = [
            *self.types,
            *self.enums,
            *self.aliases,
            *self.functions,
            *self.classes,
            *self.constants,
        ]
        return sorted(items, key=lambda item: (item.decl_index, item.name))

    def append(self, item: IRItem) -> None:
        """Append an item to its typed bucket."""
        if isinstance(item, IRStruct):
            self.types.append(item)
            return
        if isinstance(item, IREnum):
            self.enums.append(item)
            return
        if isinstance(item, IRAlias):
            self.aliases.append(item)
            return
        if isinstance(item, IRFunction):
            self.functions.append(item)
            return
        if isinstance(item, IRClass):
            self.classes.append(item)
            return
        if isinstance(item, IRConstant):
            self.constants.append(item)
            return
        raise ValueError(f"Unknown IR item type: {type(item)}")


@dataclass
class IRModule:
    """
    Represents the entire IR for a project.

    Files are stored as a dict mapping file path to IRFile.
    Each IRFile contains items in declaration order.
    """

    files: Dict[str, IRFile] = field(default_factory=dict)
