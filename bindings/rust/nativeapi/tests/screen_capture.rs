use nativeapi::{
    CaptureSourceType, Rectangle, ScreenCaptureOptions, ScreenCapturer, ScreenRecorder,
    ScreenRecordingOptions, ScreenRecordingState,
};

// No desktop access: links the real capture backend and exercises nested FFI
// values, source validation and lifecycle rejection without requesting capture.
#[test]
fn invalid_request_keeps_recorder_idle() {
    assert!(!ScreenCapturer::is_direct_capture_supported(
        CaptureSourceType::None
    ));
    assert!(!ScreenCapturer::is_direct_capture_supported(
        CaptureSourceType::Unknown
    ));
    let options = ScreenRecordingOptions {
        capture: ScreenCaptureOptions {
            include_cursor: false,
            crop_enabled: true,
            crop: Rectangle {
                x: 1.0,
                y: 2.0,
                width: 32.0,
                height: 48.0,
            },
        },
        frame_rate: 30.0,
        video_bitrate: 1_000_000,
    };
    let recorder = ScreenRecorder::new().expect("recorder handle");
    assert!(!recorder.start(None, "relative.mp4", &options));
    assert_eq!(recorder.state(), ScreenRecordingState::Idle);
    assert_eq!(recorder.duration(), 0.0);
    assert!(!recorder.is_active());
    assert!(!recorder.stop());
    assert!(!recorder.cancel());
}
