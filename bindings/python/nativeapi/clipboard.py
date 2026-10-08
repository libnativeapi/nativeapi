# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from clipboard.h."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

from . import _capi as _C
from . import _runtime as _rt
from . import image as _image


@dataclass
class ClipboardData:
    text: str | None = None
    html: str | None = None
    image: _image.Image | None = None
    file_paths: list[str] = field(default_factory=list)

    @classmethod
    def _from_c(cls, raw: _C.native_clipboard_data_t) -> ClipboardData:
        return cls(
            None if raw.text is None else _rt.decode(raw.text),
            None if raw.html is None else _rt.decode(raw.html),
            _image.Image._owned(_rt.retain_handle(raw.image)),
            _rt.read_str_list(raw.file_paths),
        )

    def _to_c(self) -> _C.native_clipboard_data_t:
        raw = _C.native_clipboard_data_t()
        raw.text = _rt.encode_optional(self.text)
        raw.html = _rt.encode_optional(self.html)
        raw.image = _rt.handle_of(self.image)
        raw.file_paths = _rt.str_list(self.file_paths)
        return raw


@dataclass(frozen=True)
class ClipboardEvent:
    """Base of every ClipboardEvent; listeners receive one of its subclasses."""

    @staticmethod
    def _from_c(raw: _C.native_clipboard_event_t) -> ClipboardEvent | None:
        if raw.type == 0:
            return ClipboardChangedEvent()
        return None


@dataclass(frozen=True)
class ClipboardChangedEvent(ClipboardEvent):
    pass


class Clipboard:
    """The process-wide Clipboard; every member is static."""

    def __init__(self) -> None:
        raise TypeError("Clipboard is a singleton; call its static methods")

    @staticmethod
    def is_supported() -> bool:
        raw = _C.native_clipboard_is_supported()
        return raw

    @staticmethod
    def is_change_monitoring_supported() -> bool:
        raw = _C.native_clipboard_is_change_monitoring_supported()
        return raw

    @staticmethod
    def read(callback: Callable[[bool, ClipboardData], None]) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_clipboard_data_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    ClipboardData._from_c(a1.contents)
                ),
            ),
        )
        _C.native_clipboard_read(
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    async def read_async() -> ClipboardData:
        return await _rt.read_async(Clipboard.read)

    @staticmethod
    def read_text(callback: Callable[[bool, str | None], None]) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_std_optional_std_string_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    None if a1 is None else _rt.decode(a1)
                ),
            ),
        )
        _C.native_clipboard_read_text(
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    async def read_text_async() -> str | None:
        return await _rt.read_async(Clipboard.read_text)

    @staticmethod
    def read_html(callback: Callable[[bool, str | None], None]) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_std_optional_std_string_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    None if a1 is None else _rt.decode(a1)
                ),
            ),
        )
        _C.native_clipboard_read_html(
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    async def read_html_async() -> str | None:
        return await _rt.read_async(Clipboard.read_html)

    @staticmethod
    def read_image(callback: Callable[[bool, _image.Image | None], None]) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_image_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    _image.Image._owned(_rt.retain_handle(a1))
                ),
            ),
        )
        _C.native_clipboard_read_image(
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    async def read_image_async() -> _image.Image | None:
        return await _rt.read_async(Clipboard.read_image)

    @staticmethod
    def read_file_paths(callback: Callable[[bool, list[str]], None]) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_std_vector_std_string_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    _rt.read_str_list(a1.contents)
                ),
            ),
        )
        _C.native_clipboard_read_file_paths(
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    async def read_file_paths_async() -> list[str]:
        return await _rt.read_async(Clipboard.read_file_paths)

    @staticmethod
    def write(data: ClipboardData) -> bool:
        if (
            not _rt.valid_clipboard_text(data.text)
            or not _rt.valid_clipboard_text(data.html)
            or any(not _rt.valid_clipboard_text(path) for path in data.file_paths)
        ):
            return False
        raw = _C.native_clipboard_write(data._to_c())
        return raw

    @staticmethod
    def write_text(text: str) -> bool:
        if not _rt.valid_clipboard_text(text):
            return False
        raw = _C.native_clipboard_write_text(_rt.encode(text))
        return raw

    @staticmethod
    def write_html(html: str) -> bool:
        if not _rt.valid_clipboard_text(html):
            return False
        raw = _C.native_clipboard_write_html(_rt.encode(html))
        return raw

    @staticmethod
    def write_image(image: _image.Image | None) -> bool:
        raw = _C.native_clipboard_write_image(_rt.handle_of(image))
        return raw

    @staticmethod
    def write_file_paths(file_paths: list[str]) -> bool:
        if any(not _rt.valid_clipboard_text(path) for path in file_paths):
            return False
        raw = _C.native_clipboard_write_file_paths(_rt.str_list(file_paths))
        return raw

    @staticmethod
    def clear() -> bool:
        raw = _C.native_clipboard_clear()
        return raw

    @staticmethod
    def is_monitoring() -> bool:
        raw = _C.native_clipboard_is_monitoring()
        return raw

    @staticmethod
    def add_listener(callback: Callable[[ClipboardEvent], None]) -> int:
        """Calls `callback` with every ClipboardEvent this Clipboard emits.

        Callbacks run synchronously; coroutine callbacks are not accepted.
        Use an owned EventDecision for asynchronous request confirmation.
        Returns the listener id for `remove_listener()`.
        """

        def trampoline(raw, _user_data):
            event = ClipboardEvent._from_c(raw.contents)
            if event is not None:
                _rt.deliver_event(callback, event)

        return _rt.add_listener(
            _C.native_clipboard_add_listener,
            _C.native_clipboard_event_callback_t,
            trampoline,
        )

    @staticmethod
    def remove_listener(listener_id: int) -> bool:
        """Unregisters a listener. Returns False if unknown."""
        return _rt.remove_listener(_C.native_clipboard_remove_listener, listener_id)
