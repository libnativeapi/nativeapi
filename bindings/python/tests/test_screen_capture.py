"""No capture/UI: validate nested options and recorder state across the real ABI."""

import pytest

from nativeapi import (
    CaptureSourceType,
    Rectangle,
    ScreenCaptureOptions,
    ScreenCapturer,
    ScreenRecorder,
    ScreenRecordingOptions,
    ScreenRecordingState,
)


def test_static_entry_and_invalid_direct_types():
    with pytest.raises(TypeError):
        ScreenCapturer()
    assert not ScreenCapturer.is_direct_capture_supported(CaptureSourceType.NONE)
    assert not ScreenCapturer.is_direct_capture_supported(CaptureSourceType.UNKNOWN)


def test_nested_options_rejection_keeps_idle_recorder():
    capture = ScreenCaptureOptions(False, True, Rectangle(1, 2, 32, 48))
    options = ScreenRecordingOptions(capture, 30.0, 1_000_000)
    # Exercises the actual nested ctypes value conversion independently of a
    # desktop/permission; a relative destination is rejected before any capture.
    with ScreenRecorder() as recorder:
        assert recorder.state == ScreenRecordingState.IDLE
        assert not recorder.start(None, "relative.mp4", options)
        assert recorder.state == ScreenRecordingState.IDLE
        assert recorder.duration == 0
        assert not recorder.is_active
        assert not recorder.stop()
        assert not recorder.cancel()
