# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from foundation/event_request.h."""

from __future__ import annotations

from . import _capi as _C
from . import _runtime as _rt


class EventDecision(_rt.NativeObject):
    """Owned reference to a native EventDecision.

    `dispose()` (or `with`) releases it; otherwise it is released when the
    wrapper is garbage collected.
    """

    __slots__ = ()
    _free = staticmethod(_C.native_event_decision_free)

    def accept(self) -> bool:
        raw = _C.native_event_decision_accept(self._handle)
        return raw

    def cancel(self) -> bool:
        raw = _C.native_event_decision_cancel(self._handle)
        return raw

    @property
    def is_pending(self) -> bool:
        raw = _C.native_event_decision_is_pending(self._handle)
        return raw


class EventRequest(_rt.NativeObject):
    """Owned reference to a native EventRequest.

    `dispose()` (or `with`) releases it; otherwise it is released when the
    wrapper is garbage collected.
    """

    __slots__ = ()
    _free = staticmethod(_C.native_event_request_free)

    @property
    def is_cancelable(self) -> bool:
        raw = _C.native_event_request_is_cancelable(self._handle)
        return raw

    @property
    def is_cancelled(self) -> bool:
        raw = _C.native_event_request_is_cancelled(self._handle)
        return raw

    @property
    def is_pending(self) -> bool:
        raw = _C.native_event_request_is_pending(self._handle)
        return raw

    def cancel(self) -> bool:
        raw = _C.native_event_request_cancel(self._handle)
        return raw

    def defer(self) -> EventDecision | None:
        raw = _C.native_event_request_defer(self._handle)
        return EventDecision._owned(raw)
