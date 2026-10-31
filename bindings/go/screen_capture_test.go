package nativeapi

import "testing"

// No desktop or permission: exercise the real cgo nested-value conversion.
func TestRecordingRejectionKeepsIdle(t *testing.T) {
	recorder, err := NewScreenRecorder()
	if err != nil {
		t.Fatal(err)
	}
	defer recorder.Release()
	options := ScreenRecordingOptions{
		Capture: ScreenCaptureOptions{IncludeCursor: false, CropEnabled: true,
			Crop: Rectangle{X: 1, Y: 2, Width: 32, Height: 48}},
		FrameRate: 30, VideoBitrate: 1000000,
	}
	if recorder.Start(nil, "relative.mp4", options) == nil {
		t.Fatal("invalid request accepted")
	}
	if recorder.State() != ScreenRecordingStateIdle || recorder.IsActive() || recorder.Duration() != 0 {
		t.Fatal("rejected request changed recorder state")
	}
	if recorder.Stop() == nil || recorder.Cancel() == nil {
		t.Fatal("idle recorder accepted a termination action")
	}
}
