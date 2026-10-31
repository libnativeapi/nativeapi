# AUTO-GENERATED. DO NOT EDIT.
# Any manual changes WILL BE LOST when this file is regenerated.
"""Generated from screen_recorder.h."""

from __future__ import annotations

import enum
from collections.abc import Callable
from dataclasses import dataclass, field

from . import _capi as _C
from . import _runtime as _rt
from . import capture_source as _capture_source
from . import screen_capturer as _screen_capturer


class ScreenRecordingState(enum.IntEnum):
    IDLE = 0
    STARTING = 1
    RECORDING = 2
    STOPPING = 3
    COMPLETED = 4
    CANCELLED = 5
    FAILED = 6


@dataclass
class ScreenRecordingOptions:
    capture: _screen_capturer.ScreenCaptureOptions = (
        field(default_factory=lambda: _screen_capturer.ScreenCaptureOptions())
    )
    frame_rate: float = 0.0
    video_bitrate: int = 0

    @classmethod
    def _from_c(
        cls,
        raw: _C.native_screen_recording_options_t,
    ) -> ScreenRecordingOptions:
        return cls(
            _screen_capturer.ScreenCaptureOptions._from_c(raw.capture),
            raw.frame_rate,
            raw.video_bitrate,
        )

    def _to_c(self) -> _C.native_screen_recording_options_t:
        raw = _C.native_screen_recording_options_t()
        raw.capture = self.capture._to_c()
        raw.frame_rate = self.frame_rate
        raw.video_bitrate = self.video_bitrate
        return raw


@dataclass(frozen=True)
class ScreenRecordingEvent:
    """Base of every ScreenRecordingEvent; listeners receive one of its subclasses."""

    @staticmethod
    def _from_c(raw: _C.native_screen_recording_event_t) -> ScreenRecordingEvent | None:
        if raw.type == 0:
            return ScreenRecordingStartedEvent()
        if raw.type == 1:
            return ScreenRecordingCompletedEvent(
                _rt.decode(raw.data.completed.file_path),
                raw.data.completed.duration,
            )
        if raw.type == 2:
            return ScreenRecordingCancelledEvent()
        if raw.type == 3:
            return ScreenRecordingFailedEvent(_rt.decode(raw.data.failed.message))
        return None


@dataclass(frozen=True)
class ScreenRecordingStartedEvent(ScreenRecordingEvent):
    pass


@dataclass(frozen=True)
class ScreenRecordingCompletedEvent(ScreenRecordingEvent):
    file_path: str
    duration: float


@dataclass(frozen=True)
class ScreenRecordingCancelledEvent(ScreenRecordingEvent):
    pass


@dataclass(frozen=True)
class ScreenRecordingFailedEvent(ScreenRecordingEvent):
    message: str


class ScreenRecorder(_rt.NativeObject):
    """Owned reference to a native ScreenRecorder.

    `dispose()` (or `with`) releases it; otherwise it is released when the
    wrapper is garbage collected.
    """

    __slots__ = ()
    _free = staticmethod(_C.native_screen_recorder_free)

    def __init__(self) -> None:
        handle = _C.native_screen_recorder_create()
        if not handle:
            raise _rt.NativeApiError("failed to create a ScreenRecorder")
        self._adopt(handle)

    @staticmethod
    def is_supported() -> bool:
        raw = _C.native_screen_recorder_is_supported()
        return raw

    def start(
        self,
        source: _capture_source.CaptureSource | None,
        file_path: str,
        options: ScreenRecordingOptions,
    ) -> bool:
        raw = _C.native_screen_recorder_start(
            self._handle,
            _rt.handle_of(source),
            _rt.encode(file_path),
            options._to_c(),
        )
        return raw

    def stop(self) -> bool:
        raw = _C.native_screen_recorder_stop(self._handle)
        return raw

    def cancel(self) -> bool:
        raw = _C.native_screen_recorder_cancel(self._handle)
        return raw

    @property
    def state(self) -> ScreenRecordingState:
        raw = _C.native_screen_recorder_get_state(self._handle)
        return _rt.to_enum(ScreenRecordingState, raw)

    @property
    def is_active(self) -> bool:
        raw = _C.native_screen_recorder_is_active(self._handle)
        return raw

    @property
    def duration(self) -> float:
        raw = _C.native_screen_recorder_get_duration(self._handle)
        return raw

    @property
    def file_path(self) -> str:
        raw = _C.native_screen_recorder_get_file_path(self._handle)
        return _rt.take_str(raw)

    def add_listener(self, callback: Callable[[ScreenRecordingEvent], None]) -> int:
        """Calls `callback` with every ScreenRecordingEvent this ScreenRecorder emits.

        Callbacks run synchronously; coroutine callbacks are not accepted.
        Use an owned EventDecision for asynchronous request confirmation.
        Returns the listener id for `remove_listener()`.
        """

        def trampoline(raw, _user_data):
            event = ScreenRecordingEvent._from_c(raw.contents)
            if event is not None:
                _rt.deliver_event(callback, event)

        return _rt.add_listener(
            _C.native_screen_recorder_add_listener,
            _C.native_screen_recording_event_callback_t,
            trampoline,
            self._handle,
        )

    def remove_listener(self, listener_id: int) -> bool:
        """Unregisters a listener. Returns False if unknown."""
        return _rt.remove_listener(
            _C.native_screen_recorder_remove_listener,
            listener_id,
            self._handle,
        )
