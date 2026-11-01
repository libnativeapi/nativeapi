// No UI or permission request: real addon conversions and rejected lifecycle.
import assert from "node:assert/strict";
import { test } from "node:test";
import { CaptureSourceType, ScreenCapturer, ScreenRecorder, ScreenRecordingState } from "../lib/index.ts";

test("static capture entry rejects invalid direct types", () => {
  assert.equal(ScreenCapturer.isDirectCaptureSupported(CaptureSourceType.None), false);
  assert.equal(ScreenCapturer.isDirectCaptureSupported(CaptureSourceType.Unknown), false);
});

test("nested recording options cross the ABI without creating an output", () => {
  const recorder = ScreenRecorder.create()!;
  try {
    assert.equal(recorder.state, ScreenRecordingState.Idle);
    assert.equal(recorder.start(null, "relative.mp4", {
      capture: { includeCursor: false, cropEnabled: true, crop: { x: 1, y: 2, width: 32, height: 48 } },
      frameRate: 30,
      videoBitrate: 1_000_000,
    }), false);
    assert.equal(recorder.state, ScreenRecordingState.Idle);
    assert.equal(recorder.isActive, false);
    assert.equal(recorder.duration, 0);
    assert.equal(recorder.stop(), false);
    assert.equal(recorder.cancel(), false);
  } finally {
    recorder.dispose();
  }
});
