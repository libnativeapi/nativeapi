// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, wrapHandle, deliverEvent } from "./runtime.ts";
import { Display, type DisplayEvent } from "./display.ts";
import { type Point } from "./geometry.ts";

export class DisplayManager {
  private constructor() {}

  static getAll(): Display[] {
    return (native.native_display_manager_get_all() as bigint[]).map((handle) => new Display(handle));
  }

  static getPrimary(): Display | null {
    return wrapHandle(Display, native.native_display_manager_get_primary());
  }

  static getCursorPosition(): Point {
    return native.native_display_manager_get_cursor_position();
  }

  /** Receives events on the JS thread. Borrowed objects stay valid until the returned Promise settles. */
  static addListener(listener: (event: DisplayEvent) => void | Promise<void>): number {
    return native.native_display_manager_add_listener((event: Record<string, unknown>, delivery: bigint) =>
      deliverEvent(delivery, () => {
        if (typeof event.display === "bigint") {
          event.display = event.display ? new Display(event.display, false) : null;
        }
        return event as unknown as DisplayEvent;
      }, listener));
  }

  /** Unregisters a listener; returns false if the id is unknown. */
  static removeListener(listenerId: number): boolean {
    return native.native_display_manager_remove_listener(listenerId);
  }
}
