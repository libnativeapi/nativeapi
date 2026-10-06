"""Actual C ABI confirmation on unshown AppKit windows, no input or app exit."""

import sys
import threading
from ctypes import c_int

import pytest

from nativeapi import Window, WindowCloseRequestedEvent, _library

pytestmark = pytest.mark.skipif(
    sys.platform != "darwin" or not hasattr(_library.lib, "nativeapi_py_test_init"),
    reason="unshown AppKit integration requires the optional native test fixture",
)


def test_native_window_aliases_veto_and_worker_approval():
    initialize = _library.function("nativeapi_py_test_init", None, [])
    pump = _library.function("nativeapi_py_test_pump", c_int, [])
    reset = _library.function("nativeapi_py_test_reset", None, [])
    initialize()
    first = Window()
    alias = Window.with_native_window(first.native_object)
    mode = "cancel"
    votes, events = [], []
    ui = threading.get_ident()

    def listener(event):
        if isinstance(event, WindowCloseRequestedEvent):
            assert threading.get_ident() == ui
            assert event.request is not None and event.request.is_cancelable
            events.append(event.window_id)
            if mode == "cancel":
                event.request.cancel()
            elif mode == "defer":
                votes.append(event.request.defer())

    a = first.add_listener(listener)
    b = alias.add_listener(listener)
    try:
        assert first.id == alias.id and not first.is_visible
        assert first.close() and len(events) == 2
        mode = "defer"
        assert first.close() and alias.close() and len(events) == 4
        assert len(votes) == 2 and all(vote is not None for vote in votes)
        assert votes[0].accept()
        replies = []
        worker = threading.Thread(target=lambda: replies.append(votes[1].accept()))
        worker.start()
        worker.join(timeout=3)
        assert not worker.is_alive() and replies == [True]
        assert first.close() and len(events) == 4
        pump()
        assert not first.close()
    finally:
        first.remove_listener(a)
        alias.remove_listener(b)
        for vote in votes:
            if vote is not None:
                vote.dispose()
        alias.dispose()
        first.dispose()
        pump()
        reset()
