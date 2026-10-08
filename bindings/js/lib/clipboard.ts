// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

import { native, deliverEvent } from "./runtime.ts";
import { Image } from "./image.ts";

function validClipboardText(value: string | null | undefined): boolean {
  for (const character of value ?? "") {
    const point = character.codePointAt(0)!;
    if (point === 0 || (point >= 0xd800 && point <= 0xdfff)) return false;
  }
  return true;
}

export interface ClipboardData {
  text?: string | null;
  html?: string | null;
  image?: Image | null;
  filePaths?: string[];
}

export type ClipboardEvent =
  | { type: "changed" };

export class Clipboard {
  private constructor() {}

  static isSupported(): boolean {
    return native.native_clipboard_is_supported();
  }

  static isChangeMonitoringSupported(): boolean {
    return native.native_clipboard_is_change_monitoring_supported();
  }

  static read(callback: (arg0: boolean, arg1: ClipboardData) => void): void {
    native.native_clipboard_read((arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, ({ ...arg1, image: (arg1.image ? new Image(native.retainHandle(arg1.image)) : null) })] as const, (values) => callback(...values)));
  }

  static readAsync(): Promise<ClipboardData> {
    return new Promise((resolve, reject) => this.read((success, value) => {
      if (success) resolve(value);
      else reject(new Error("nativeapi: operation failed"));
    }));
  }

  static readText(callback: (arg0: boolean, arg1: string | null) => void): void {
    native.native_clipboard_read_text((arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, arg1] as const, (values) => callback(...values)));
  }

  static readTextAsync(): Promise<string | null> {
    return new Promise((resolve, reject) => this.readText((success, value) => {
      if (success) resolve(value);
      else reject(new Error("nativeapi: operation failed"));
    }));
  }

  static readHtml(callback: (arg0: boolean, arg1: string | null) => void): void {
    native.native_clipboard_read_html((arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, arg1] as const, (values) => callback(...values)));
  }

  static readHtmlAsync(): Promise<string | null> {
    return new Promise((resolve, reject) => this.readHtml((success, value) => {
      if (success) resolve(value);
      else reject(new Error("nativeapi: operation failed"));
    }));
  }

  static readImage(callback: (arg0: boolean, arg1: Image | null) => void): void {
    native.native_clipboard_read_image((arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, (arg1 ? new Image(native.retainHandle(arg1)) : null)] as const, (values) => callback(...values)));
  }

  static readImageAsync(): Promise<Image | null> {
    return new Promise((resolve, reject) => this.readImage((success, value) => {
      if (success) resolve(value);
      else reject(new Error("nativeapi: operation failed"));
    }));
  }

  static readFilePaths(callback: (arg0: boolean, arg1: string[]) => void): void {
    native.native_clipboard_read_file_paths((arg0: any, arg1: any, delivery: bigint) => deliverEvent(delivery, () => [arg0, arg1] as const, (values) => callback(...values)));
  }

  static readFilePathsAsync(): Promise<string[]> {
    return new Promise((resolve, reject) => this.readFilePaths((success, value) => {
      if (success) resolve(value);
      else reject(new Error("nativeapi: operation failed"));
    }));
  }

  static write(data: ClipboardData): boolean {
    if (!validClipboardText(data.text) || !validClipboardText(data.html) || (data.filePaths ?? []).some(path => !validClipboardText(path))) return false;
    return native.native_clipboard_write(({ ...data, image: data.image?.nativeHandle ?? 0n }));
  }

  static writeText(text: string): boolean {
    if (!validClipboardText(text)) return false;
    return native.native_clipboard_write_text(text);
  }

  static writeHtml(html: string): boolean {
    if (!validClipboardText(html)) return false;
    return native.native_clipboard_write_html(html);
  }

  static writeImage(image: Image | null): boolean {
    return native.native_clipboard_write_image(image?.nativeHandle ?? 0n);
  }

  static writeFilePaths(filePaths: string[]): boolean {
    if (filePaths.some(path => !validClipboardText(path))) return false;
    return native.native_clipboard_write_file_paths(filePaths);
  }

  static clear(): boolean {
    return native.native_clipboard_clear();
  }

  static isMonitoring(): boolean {
    return native.native_clipboard_is_monitoring();
  }

  /** Receives events on the JS thread. Borrowed objects stay valid until the returned Promise settles. */
  static addListener(listener: (event: ClipboardEvent) => void | Promise<void>): number {
    return native.native_clipboard_add_listener((event: Record<string, unknown>, delivery: bigint) =>
      deliverEvent(delivery, () => {
        return event as unknown as ClipboardEvent;
      }, listener));
  }

  /** Unregisters a listener; returns false if the id is unknown. */
  static removeListener(listenerId: number): boolean {
    return native.native_clipboard_remove_listener(listenerId);
  }
}
