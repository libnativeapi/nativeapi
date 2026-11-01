// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, NativeObject, deliverEvent } from "./runtime.ts";
import { CaptureSource } from "./capture_source.ts";
import { type ScreenCaptureOptions } from "./screen_capturer.ts";

export const ScreenRecordingState = {
  Idle: 0,
  Starting: 1,
  Recording: 2,
  Stopping: 3,
  Completed: 4,
  Cancelled: 5,
  Failed: 6,
} as const;
export type ScreenRecordingState = (typeof ScreenRecordingState)[keyof typeof ScreenRecordingState];

export interface ScreenRecordingOptions {
  capture: ScreenCaptureOptions;
  frameRate: number;
  videoBitrate: number;
}

export type ScreenRecordingEvent =
  | { type: "started" }
  | { type: "completed"; filePath: string | null; duration: number }
  | { type: "cancelled" }
  | { type: "failed"; message: string | null };

/** A native ScreenRecorder, held through an owned handle. */
export class ScreenRecorder extends NativeObject {
  /** Wraps a raw handle; an owned one is released on `dispose()` or collection. */
  constructor(handle: bigint, owned = true) {
    super(handle, owned ? native.native_screen_recorder_free : undefined);
  }

  static create(): ScreenRecorder | null {
    const handle: bigint = native.native_screen_recorder_create();
    return handle ? new ScreenRecorder(handle) : null;
  }

  static isSupported(): boolean {
    return native.native_screen_recorder_is_supported();
  }

  start(source: CaptureSource | null, filePath: string, options: ScreenRecordingOptions): boolean {
    return native.native_screen_recorder_start(this.nativeHandle, source?.nativeHandle ?? 0n, filePath, ({ ...options, capture: ({ ...options.capture, crop: options.capture.crop }) }));
  }

  stop(): boolean {
    return native.native_screen_recorder_stop(this.nativeHandle);
  }

  cancel(): boolean {
    return native.native_screen_recorder_cancel(this.nativeHandle);
  }

  get state(): ScreenRecordingState {
    return native.native_screen_recorder_get_state(this.nativeHandle);
  }

  get isActive(): boolean {
    return native.native_screen_recorder_is_active(this.nativeHandle);
  }

  get duration(): number {
    return native.native_screen_recorder_get_duration(this.nativeHandle);
  }

  get filePath(): string {
    return native.native_screen_recorder_get_file_path(this.nativeHandle);
  }

  /** Receives events on the JS thread. Borrowed objects stay valid until the returned Promise settles. */
  addListener(listener: (event: ScreenRecordingEvent) => void | Promise<void>): number {
    return native.native_screen_recorder_add_listener(this.nativeHandle, (event: Record<string, unknown>, delivery: bigint) =>
      deliverEvent(delivery, () => {
        return event as unknown as ScreenRecordingEvent;
      }, listener));
  }

  /** Unregisters a listener; returns false if the id is unknown. */
  removeListener(listenerId: number): boolean {
    return native.native_screen_recorder_remove_listener(this.nativeHandle, listenerId);
  }
}
