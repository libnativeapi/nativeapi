// Headless checks of the addon and the generated layer: value conversions,
// owned strings, handle lifetime. Nothing here needs a window server.
import assert from "node:assert/strict";
import { test } from "node:test";

import { Color, NativeObject, Preferences, Shortcut, ShortcutManager, ShortcutScope } from "../lib/index.ts";

test("structs round-trip through the C ABI", () => {
  assert.deepEqual(Color.fromHex("#ff8000"), { r: 255, g: 128, b: 0, a: 255 });
  assert.equal(Color.toRgba(Color.fromRgba(1, 2, 3, 4)), 0x01020304);
  assert.deepEqual(Color.Red, { r: 255, g: 0, b: 0, a: 255 });
  assert.ok(Object.isFrozen(Color.Red));
});

test("argument type errors throw instead of crashing", () => {
  assert.throws(() => Color.fromHex(42 as unknown as string), TypeError);
});

test("shortcut request and metadata ABI calls reserve no system keys", () => {
  const options = { accelerator: "", description: "ABI probe", scope: ShortcutScope.Global, enabled: true };
  assert.equal(ShortcutManager.requestRegistration(options, "abi-probe"), 0);
  assert.equal(ShortcutManager.isRegistrationPending(0), false);
  assert.equal(ShortcutManager.cancelRegistration(0), false);
  assert.equal(ShortcutManager.cancelPendingRegistrations(""), 0);
  const shortcut = Shortcut.createWithIdAndOptions(42, { ...options, accelerator: "Ctrl+F19" })!;
  try {
    assert.equal(shortcut.registrationKey, "");
    assert.equal(shortcut.triggerDescription, "Ctrl+F19");
  } finally {
    shortcut.dispose();
  }
});

test("owned handles are released exactly once", () => {
  const preferences = Preferences.createWithScope("nativeapi-js-smoke-test")!;
  assert.ok(preferences instanceof NativeObject);
  assert.ok(preferences.nativeHandle > 0n);
  preferences.set("greeting", "héllo");
  assert.equal(preferences.get("greeting", ""), "héllo");
  preferences.remove("greeting");
  preferences.dispose();
  assert.equal(preferences.nativeHandle, 0n);
  preferences.dispose();
});
