"""Clipboard marshalling and opt-in desktop round trips.

Run desktop cases through core/build/tests/clipboard_macos_test to restore native data.
"""

import asyncio
import os

import pytest

from nativeapi import Application, Clipboard, ClipboardData, Image

PNG = (
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAC0lEQVR42mNg"
    "AAIAAAUAAen63NgAAAAASUVORK5CYII="
)


def test_clipboard_optional_strings_preserve_empty():
    for text in (None, "", "剪贴板 😀"):
        data = ClipboardData(text=text, file_paths=["/tmp/example", "/tmp/目录"])
        raw = data._to_c()
        assert ClipboardData._from_c(raw) == data


def test_clipboard_rejects_invalid_text_before_native_submission():
    assert not Clipboard.write_text("a\0b")
    assert not Clipboard.write_html("\ud800")
    assert not Clipboard.write(ClipboardData(text="\0"))
    assert not Clipboard.write_file_paths(["/tmp/a\0b"])


@pytest.mark.skipif(
    not os.getenv("NATIVEAPI_CLIPBOARD_TEST"), reason="requires desktop clipboard"
)
def test_clipboard_async_roundtrip_and_owned_images():
    async def run():
        loop = asyncio.create_task(Application.run_async())
        await asyncio.sleep(0)
        try:
            image = Image.from_base64(PNG)
            assert image is not None
            assert Clipboard.write(
                ClipboardData(
                    text="剪贴板 😀",
                    html="<b>中文</b>",
                    image=image,
                    file_paths=["/tmp/file", "/tmp/directory"],
                )
            )
            image.dispose()
            data, text, html, paths, result_image = await asyncio.gather(
                Clipboard.read_async(),
                Clipboard.read_text_async(),
                Clipboard.read_html_async(),
                Clipboard.read_file_paths_async(),
                Clipboard.read_image_async(),
            )
            assert data.text == text == "剪贴板 😀"
            assert data.html == html == "<b>中文</b>"
            assert data.file_paths == paths == ["/tmp/file", "/tmp/directory"]
            assert Clipboard.clear()
            assert data.image.size.width == result_image.size.width == 1
            data.image.dispose()
            result_image.dispose()
            assert await Clipboard.read_text_async() is None
            assert Clipboard.write_text("")
            assert await Clipboard.read_text_async() == ""
            assert await Clipboard.read_html_async() is None
        finally:
            Application.quit()
            await loop

    asyncio.run(run())
