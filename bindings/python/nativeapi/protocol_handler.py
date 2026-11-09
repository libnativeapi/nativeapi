# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from protocol_handler.h."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from . import _capi as _C
from . import _runtime as _rt


@dataclass(frozen=True)
class ProtocolHandlerEvent:
    """Base of every ProtocolHandlerEvent; listeners receive one of its subclasses."""

    @staticmethod
    def _from_c(raw: _C.native_protocol_handler_event_t) -> ProtocolHandlerEvent | None:
        if raw.type == 0:
            return ProtocolUrlReceivedEvent(
                _rt.decode(raw.data.protocol_url_received.url),
            )
        return None


@dataclass(frozen=True)
class ProtocolUrlReceivedEvent(ProtocolHandlerEvent):
    url: str


class ProtocolHandler:
    """The process-wide ProtocolHandler; every member is static."""

    def __init__(self) -> None:
        raise TypeError("ProtocolHandler is a singleton; call its static methods")

    @staticmethod
    def is_supported() -> bool:
        raw = _C.native_protocol_handler_is_supported()
        return raw

    @staticmethod
    def register(scheme: str) -> bool:
        raw = _C.native_protocol_handler_register(_rt.encode(scheme))
        return raw

    @staticmethod
    def get_initial_url() -> str | None:
        raw = _C.native_protocol_handler_get_initial_url()
        return _rt.take_optional_str(raw)

    @staticmethod
    def add_listener(callback: Callable[[ProtocolHandlerEvent], None]) -> int:
        """Calls `callback` with every ProtocolHandlerEvent this ProtocolHandler emits.

        Callbacks run synchronously; coroutine callbacks are not accepted.
        Use an owned EventDecision for asynchronous request confirmation.
        Returns the listener id for `remove_listener()`.
        """

        def trampoline(raw, _user_data):
            event = ProtocolHandlerEvent._from_c(raw.contents)
            if event is not None:
                _rt.deliver_event(callback, event)

        return _rt.add_listener(
            _C.native_protocol_handler_add_listener,
            _C.native_protocol_handler_event_callback_t,
            trampoline,
        )

    @staticmethod
    def remove_listener(listener_id: int) -> bool:
        """Unregisters a listener. Returns False if unknown."""
        return _rt.remove_listener(
            _C.native_protocol_handler_remove_listener,
            listener_id,
        )
