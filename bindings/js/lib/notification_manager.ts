// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, deliverEvent } from "./runtime.ts";

export type NotificationEvent =
  | { type: "activated"; argument: string | null };

export class NotificationManager {
  private constructor() {}

  static isSupported(): boolean {
    return native.native_notification_manager_is_supported();
  }

  static initialize(): boolean {
    return native.native_notification_manager_initialize();
  }

  static shutdown(): void {
    native.native_notification_manager_shutdown();
  }

  static show(title: string, message: string, tag: string, buttonLabel: string): boolean {
    return native.native_notification_manager_show(title, message, tag, buttonLabel);
  }

  static remove(tag: string): boolean {
    return native.native_notification_manager_remove(tag);
  }

  static getLastError(): string {
    return native.native_notification_manager_get_last_error();
  }

  /** Receives events on the JS thread. Borrowed objects stay valid until the returned Promise settles. */
  static addListener(listener: (event: NotificationEvent) => void | Promise<void>): number {
    return native.native_notification_manager_add_listener((event: Record<string, unknown>, delivery: bigint) =>
      deliverEvent(delivery, () => {
        return event as unknown as NotificationEvent;
      }, listener));
  }

  /** Unregisters a listener; returns false if the id is unknown. */
  static removeListener(listenerId: number): boolean {
    return native.native_notification_manager_remove_listener(listenerId);
  }
}
