# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from capture_source.h."""

from __future__ import annotations

import enum

from . import _capi as _C
from . import _runtime as _rt
from . import display as _display
from . import window as _window


class CaptureSourceType(enum.IntEnum):
    NONE = 0
    DISPLAY = 1
    WINDOW = 2
    UNKNOWN = 3


class CaptureSource(_rt.NativeObject):
    """Owned reference to a native CaptureSource.

    `dispose()` (or `with`) releases it; otherwise it is released when the
    wrapper is garbage collected.
    """

    __slots__ = ()
    _free = staticmethod(_C.native_capture_source_free)

    @staticmethod
    def from_display(display: _display.Display | None) -> CaptureSource | None:
        raw = _C.native_capture_source_from_display(_rt.handle_of(display))
        return CaptureSource._owned(raw)

    @staticmethod
    def from_window(window: _window.Window | None) -> CaptureSource | None:
        raw = _C.native_capture_source_from_window(_rt.handle_of(window))
        return CaptureSource._owned(raw)

    @property
    def type(self) -> CaptureSourceType:
        raw = _C.native_capture_source_get_type(self._handle)
        return _rt.to_enum(CaptureSourceType, raw)

    @property
    def name(self) -> str:
        raw = _C.native_capture_source_get_name(self._handle)
        return _rt.take_str(raw)
