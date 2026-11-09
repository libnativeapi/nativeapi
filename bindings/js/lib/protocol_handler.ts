// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, deliverEvent } from "./runtime.ts";

export type ProtocolHandlerEvent =
  | { type: "protocolUrlReceived"; url: string | null };

export class ProtocolHandler {
  private constructor() {}

  static isSupported(): boolean {
    return native.native_protocol_handler_is_supported();
  }

  static register(scheme: string): boolean {
    return native.native_protocol_handler_register(scheme);
  }

  static getInitialUrl(): string | null {
    return native.native_protocol_handler_get_initial_url();
  }

  /** Receives events on the JS thread. Borrowed objects stay valid until the returned Promise settles. */
  static addListener(listener: (event: ProtocolHandlerEvent) => void | Promise<void>): number {
    return native.native_protocol_handler_add_listener((event: Record<string, unknown>, delivery: bigint) =>
      deliverEvent(delivery, () => {
        return event as unknown as ProtocolHandlerEvent;
      }, listener));
  }

  /** Unregisters a listener; returns false if the id is unknown. */
  static removeListener(listenerId: number): boolean {
    return native.native_protocol_handler_remove_listener(listenerId);
  }
}
