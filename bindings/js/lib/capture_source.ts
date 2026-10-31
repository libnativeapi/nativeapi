// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, NativeObject, wrapHandle } from "./runtime.ts";
import { Display } from "./display.ts";
import { Window } from "./window.ts";

export const CaptureSourceType = {
  None: 0,
  Display: 1,
  Window: 2,
  Unknown: 3,
} as const;
export type CaptureSourceType = (typeof CaptureSourceType)[keyof typeof CaptureSourceType];

/** A native CaptureSource, held through an owned handle. */
export class CaptureSource extends NativeObject {
  /** Wraps a raw handle; an owned one is released on `dispose()` or collection. */
  constructor(handle: bigint, owned = true) {
    super(handle, owned ? native.native_capture_source_free : undefined);
  }

  static fromDisplay(display: Display | null): CaptureSource | null {
    return wrapHandle(CaptureSource, native.native_capture_source_from_display(display?.nativeHandle ?? 0n));
  }

  static fromWindow(window: Window | null): CaptureSource | null {
    return wrapHandle(CaptureSource, native.native_capture_source_from_window(window?.nativeHandle ?? 0n));
  }

  get type(): CaptureSourceType {
    return native.native_capture_source_get_type(this.nativeHandle);
  }

  get name(): string {
    return native.native_capture_source_get_name(this.nativeHandle);
  }
}
