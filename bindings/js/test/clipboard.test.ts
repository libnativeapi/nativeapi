import assert from "node:assert/strict";
import { test } from "node:test";
import { Application, Clipboard, Image } from "../lib/index.ts";

const png = "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAC0lEQVR42mNgAAIAAAUAAen63NgAAAAASUVORK5CYII=";

test("clipboard rejects NUL and malformed Unicode before submission", () => {
  assert.equal(Clipboard.writeText("a\0b"), false);
  assert.equal(Clipboard.writeHtml("\ud800"), false);
  assert.equal(Clipboard.write({ text: "\0" }), false);
  assert.equal(Clipboard.writeFilePaths(["/tmp/a\0b"]), false);
});

test("clipboard promises copy values and retain images", { skip: !process.env.NATIVEAPI_CLIPBOARD_TEST }, async () => {
  const loop = Application.run();
  try {
    const image = Image.fromBase64(png)!;
    assert.ok(Clipboard.write({ text: "剪贴板 😀", html: "<b>中文</b>", image, filePaths: ["/tmp/file", "/tmp/directory"] }));
    image.dispose();
    const [data, text, html, paths, resultImage] = await Promise.all([
      Clipboard.readAsync(), Clipboard.readTextAsync(), Clipboard.readHtmlAsync(),
      Clipboard.readFilePathsAsync(), Clipboard.readImageAsync(),
    ]);
    assert.equal(data.text, text);
    assert.equal(text, "剪贴板 😀");
    assert.equal(data.html, html);
    assert.equal(html, "<b>中文</b>");
    assert.deepEqual(data.filePaths, paths);
    assert.deepEqual(paths, ["/tmp/file", "/tmp/directory"]);
    assert.ok(Clipboard.clear());
    assert.equal(data.image!.size.width, 1);
    assert.equal(resultImage!.size.width, 1);
    data.image!.dispose(); resultImage!.dispose();
    assert.equal(await Clipboard.readTextAsync(), null);
    assert.ok(Clipboard.writeText(""));
    assert.equal(await Clipboard.readTextAsync(), "");
    assert.equal(await Clipboard.readHtmlAsync(), null);
  } finally {
    Application.quit();
    await loop;
  }
});
