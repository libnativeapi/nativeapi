# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from screen_capturer.h."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

from . import _capi as _C
from . import _runtime as _rt
from . import capture_source as _capture_source
from . import geometry as _geometry
from . import image as _image
from . import window as _window


@dataclass
class CaptureSourceSelectionOptions:
    allow_displays: bool = False
    allow_windows: bool = False
    parent_window: _window.Window | None = None

    @classmethod
    def _from_c(
        cls,
        raw: _C.native_capture_source_selection_options_t,
    ) -> CaptureSourceSelectionOptions:
        return cls(
            bool(raw.allow_displays),
            bool(raw.allow_windows),
            _window.Window._owned(_rt.retain_handle(raw.parent_window)),
        )

    def _to_c(self) -> _C.native_capture_source_selection_options_t:
        raw = _C.native_capture_source_selection_options_t()
        raw.allow_displays = self.allow_displays
        raw.allow_windows = self.allow_windows
        raw.parent_window = _rt.handle_of(self.parent_window)
        return raw


@dataclass
class ScreenCaptureOptions:
    include_cursor: bool = False
    crop_enabled: bool = False
    crop: _geometry.Rectangle = field(default_factory=lambda: _geometry.Rectangle())

    @classmethod
    def _from_c(cls, raw: _C.native_screen_capture_options_t) -> ScreenCaptureOptions:
        return cls(
            bool(raw.include_cursor),
            bool(raw.crop_enabled),
            _geometry.Rectangle._from_c(raw.crop),
        )

    def _to_c(self) -> _C.native_screen_capture_options_t:
        raw = _C.native_screen_capture_options_t()
        raw.include_cursor = self.include_cursor
        raw.crop_enabled = self.crop_enabled
        raw.crop = self.crop._to_c()
        return raw


class ScreenCapturer:
    def __init__(self) -> None:
        raise TypeError("ScreenCapturer contains only static methods")

    @staticmethod
    def is_supported() -> bool:
        raw = _C.native_screen_capturer_is_supported()
        return raw

    @staticmethod
    def is_source_selection_supported() -> bool:
        raw = _C.native_screen_capturer_is_source_selection_supported()
        return raw

    @staticmethod
    def is_direct_capture_supported(type: _capture_source.CaptureSourceType) -> bool:
        raw = _C.native_screen_capturer_is_direct_capture_supported(int(type))
        return raw

    @staticmethod
    def select_source(
        options: CaptureSourceSelectionOptions,
        callback: Callable[[bool, _capture_source.CaptureSource | None], None],
    ) -> None:
        native_callback = _rt.make_callback(
            _C.native_callback_bool_capture_source_payload_t,
            lambda a0, a1, delivery, _user_data: _rt.deliver_callback(
                delivery,
                lambda: callback(
                    bool(a0),
                    _capture_source.CaptureSource._owned(_rt.retain_handle(a1))
                ),
            ),
        )
        _C.native_screen_capturer_select_source(
            options._to_c(),
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )

    @staticmethod
    def capture(
        source: _capture_source.CaptureSource | None,
        options: ScreenCaptureOptions,
        callback: Callable[[bool, _image.Image | None], None],
    ) -> None:
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
        _C.native_screen_capturer_capture(
            _rt.handle_of(source),
            options._to_c(),
            native_callback,
            _rt.user_data(native_callback),
            _rt.release_user_data,
        )
