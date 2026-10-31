// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, deliverEvent } from "./runtime.ts";
import { CaptureSource, CaptureSourceType } from "./capture_source.ts";
import { type Rectangle } from "./geometry.ts";
import { Image } from "./image.ts";
import { Window } from "./window.ts";

export interface CaptureSourceSelectionOptions {
  allowDisplays: boolean;
  allowWindows: boolean;
  parentWindow?: Window | null;
}

export interface ScreenCaptureOptions {
  includeCursor: boolean;
  cropEnabled: boolean;
  crop: Rectangle;
}

export class ScreenCapturer {
  private constructor() {}

  static isSupported(): boolean {
    return native.native_screen_capturer_is_supported();
  }

  static isSourceSelectionSupported(): boolean {
    return native.native_screen_capturer_is_source_selection_supported();
  }

  static isDirectCaptureSupported(type: CaptureSourceType): boolean {
    return native.native_screen_capturer_is_direct_capture_supported(type);
  }

  static selectSource(options: CaptureSourceSelectionOptions, callback: (arg0: boolean, arg1: CaptureSource | null) => void): void {
    native.native_screen_capturer_select_source(({ ...options, parentWindow: options.parentWindow?.nativeHandle ?? 0n }), (arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, (arg1 ? new CaptureSource(native.retainHandle(arg1)) : null)] as const, (values) => callback(...values)));
  }

  static capture(source: CaptureSource | null, options: ScreenCaptureOptions, callback: (arg0: boolean, arg1: Image | null) => void): void {
    native.native_screen_capturer_capture(source?.nativeHandle ?? 0n, ({ ...options, crop: options.crop }), (arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, (arg1 ? new Image(native.retainHandle(arg1)) : null)] as const, (values) => callback(...values)));
  }
}
