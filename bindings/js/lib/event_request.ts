// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, NativeObject, wrapHandle } from "./runtime.ts";

/** A native EventDecision, held through an owned handle. */
export class EventDecision extends NativeObject {
  /** Wraps a raw handle; an owned one is released on `dispose()` or collection. */
  constructor(handle: bigint, owned = true) {
    super(handle, owned ? native.native_event_decision_free : undefined);
  }

  accept(): boolean {
    return native.native_event_decision_accept(this.nativeHandle);
  }

  cancel(): boolean {
    return native.native_event_decision_cancel(this.nativeHandle);
  }

  get isPending(): boolean {
    return native.native_event_decision_is_pending(this.nativeHandle);
  }
}

/** A native EventRequest, held through an owned handle. */
export class EventRequest extends NativeObject {
  /** Wraps a raw handle; an owned one is released on `dispose()` or collection. */
  constructor(handle: bigint, owned = true) {
    super(handle, owned ? native.native_event_request_free : undefined);
  }

  get isCancelable(): boolean {
    return native.native_event_request_is_cancelable(this.nativeHandle);
  }

  get isCancelled(): boolean {
    return native.native_event_request_is_cancelled(this.nativeHandle);
  }

  get isPending(): boolean {
    return native.native_event_request_is_pending(this.nativeHandle);
  }

  cancel(): boolean {
    return native.native_event_request_cancel(this.nativeHandle);
  }

  defer(): EventDecision | null {
    return wrapHandle(EventDecision, native.native_event_request_defer(this.nativeHandle));
  }
}
