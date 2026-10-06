"""Actual ctypes quit/asyncio regressions with no native windows or input.

Configure CMake with -DNATIVEAPI_PY_BUILD_TESTS=ON. The optional fixture lives
in the real binding DSO, sharing its application and handle table. Only the
OS launcher/pump is substituted, so these tests cannot activate the desktop.
"""

import asyncio
import os
import subprocess
import sys
import textwrap
import threading
from ctypes import c_int, c_uint64
from pathlib import Path

import pytest

from nativeapi import (
    Application,
    ApplicationExitingEvent,
    ApplicationQuitRequestedEvent,
    _library,
    _runtime,
)

pytestmark = pytest.mark.skipif(
    not hasattr(_library.lib, "nativeapi_py_test_init"),
    reason="requires CMake -DNATIVEAPI_PY_BUILD_TESTS=ON",
)

_init = _library.function("nativeapi_py_test_init", None, [])
_pump = _library.function("nativeapi_py_test_pump", c_int, [])
_reset = _library.function("nativeapi_py_test_reset", None, [])
_live = _library.function("nativeapi_py_test_live_handles", c_uint64, [])


@pytest.fixture(autouse=True)
def native_loop(monkeypatch):
    _init()
    monkeypatch.setattr(_runtime, "_start_event_loop", lambda _window: None)
    monkeypatch.setattr(_runtime, "_pump_event_loop", _pump)
    baseline = _live(), len(_runtime._callbacks)
    try:
        yield
        assert not _runtime.is_event_loop_running()
        _pump()
        assert (_live(), len(_runtime._callbacks)) == baseline
    finally:
        _reset()


async def start_loop():
    task = asyncio.create_task(Application.run_async())
    await asyncio.sleep(0)
    assert _runtime.is_event_loop_running()
    return task


def test_veto_keeps_asyncio_running_and_can_retry():
    async def scenario():
        requests, exits = [], []
        veto = True

        def listener(event):
            if isinstance(event, ApplicationQuitRequestedEvent):
                requests.append(event)
                assert event.request.is_cancelable
                assert event.request.is_pending
                if veto:
                    assert event.request.cancel()
            elif isinstance(event, ApplicationExitingEvent):
                exits.append(event.exit_code)
                assert not task.done()

        token = Application.add_listener(listener)
        try:
            task = await start_loop()
            Application.quit(7)
            await asyncio.sleep(0.02)
            assert not task.done() and len(requests) == 1 and not exits
            veto = False
            Application.quit(11)
            assert await asyncio.wait_for(task, 1) == 11
            assert len(requests) == 2 and exits == [11]
        finally:
            Application.remove_listener(token)

    asyncio.run(scenario())


def test_owned_votes_await_all_workers_and_coalesce_exit_code():
    async def scenario():
        votes, exits = [], []
        requests = 0
        ui = threading.get_ident()

        def listener(event):
            nonlocal requests
            assert threading.get_ident() == ui
            if isinstance(event, ApplicationQuitRequestedEvent):
                requests += 1
                votes.extend([event.request.defer(), event.request.defer()])
            elif isinstance(event, ApplicationExitingEvent):
                exits.append(event.exit_code)

        token = Application.add_listener(listener)
        try:
            task = await start_loop()
            await asyncio.to_thread(Application.quit, 13)
            _pump()
            Application.quit(17)
            assert requests == 1 and len(votes) == 2 and not task.done()
            assert await asyncio.to_thread(votes[0].accept)
            _pump()
            assert not task.done() and not exits
            # Do not yield to asyncio between the final worker vote and the
            # repeated quit: otherwise the pump may legitimately finish first.
            accepted = []
            worker = threading.Thread(target=lambda: accepted.append(votes[1].accept()))
            worker.start()
            worker.join()
            assert accepted == [True]
            Application.quit(19)
            assert requests == 1
            _pump()
            assert await asyncio.wait_for(task, 1) == 19 and exits == [19]
        finally:
            for vote in votes:
                vote.dispose()
            Application.remove_listener(token)

    asyncio.run(scenario())


def test_cancelled_async_task_invalidates_votes_before_next_run():
    async def scenario():
        votes, exits = [], []

        def listener(event):
            if isinstance(event, ApplicationQuitRequestedEvent):
                votes.append(event.request.defer())
            elif isinstance(event, ApplicationExitingEvent):
                exits.append(event.exit_code)

        token = Application.add_listener(listener)
        try:
            old = await start_loop()
            Application.quit(23)
            assert votes[0].is_pending
            old.cancel()
            with pytest.raises(asyncio.CancelledError):
                await old
            assert not votes[0].is_pending and not votes[0].accept()
            new = await start_loop()
            Application.quit(29)
            assert len(votes) == 2 and not new.done()
            assert votes[1].accept()
            assert await asyncio.wait_for(new, 1) == 29 and exits == [29]
        finally:
            for vote in votes:
                vote.dispose()
            Application.remove_listener(token)

    asyncio.run(scenario())


def test_queued_worker_quit_is_ignored_after_task_cancellation():
    async def scenario():
        requests = []
        token = Application.add_listener(requests.append)
        try:
            old = await start_loop()
            # Join before yielding, ensuring the worker's queued native request
            # cannot reach the UI before the old asyncio task is cancelled.
            worker = threading.Thread(target=Application.quit, args=(31,))
            worker.start()
            worker.join()
            old.cancel()
            with pytest.raises(asyncio.CancelledError):
                await old
            _pump()
            assert not requests
            new = await start_loop()
            Application.quit(37)
            assert await asyncio.wait_for(new, 1) == 37
            assert (
                sum(isinstance(e, ApplicationQuitRequestedEvent) for e in requests) == 1
            )
        finally:
            Application.remove_listener(token)

    asyncio.run(scenario())


def test_platform_exit_does_not_ask_for_another_confirmation(monkeypatch):
    async def scenario():
        requests = []
        token = Application.add_listener(requests.append)
        try:
            monkeypatch.setattr(_runtime, "_pump_event_loop", lambda: 41)
            task = await start_loop()
            assert await asyncio.wait_for(task, 1) == 41
            assert not requests
        finally:
            Application.remove_listener(token)

    asyncio.run(scenario())


@pytest.mark.parametrize("exit_code", [0, -7, 43])
def test_quit_without_listeners_completes_the_async_loop(exit_code):
    async def scenario():
        task = await start_loop()
        baseline = len(_runtime._callbacks)
        Application.quit(exit_code)
        assert len(_runtime._callbacks) == baseline
        assert await asyncio.wait_for(task, 1) == exit_code

    asyncio.run(scenario())


def test_callback_exception_vetoes_and_reports_to_asyncio():
    async def scenario():
        errors = []
        aio = asyncio.get_running_loop()
        aio.set_exception_handler(lambda _loop, context: errors.append(context))

        def listener(event):
            if isinstance(event, ApplicationQuitRequestedEvent):
                raise RuntimeError("confirmation failed")

        token = Application.add_listener(listener)
        try:
            task = await start_loop()
            Application.quit(47)
            await asyncio.sleep(0.02)
            assert not task.done()
            assert len(errors) == 1
            assert isinstance(errors[0]["exception"], RuntimeError)
        finally:
            Application.remove_listener(token)
        Application.quit(49)
        assert await asyncio.wait_for(task, 1) == 49

    asyncio.run(scenario())


def test_coroutine_callback_is_rejected_without_approving_exit():
    async def scenario():
        errors = []
        asyncio.get_running_loop().set_exception_handler(
            lambda _loop, context: errors.append(context)
        )

        async def listener(_event):
            raise AssertionError("coroutine body must not run")

        token = Application.add_listener(listener)
        try:
            task = await start_loop()
            Application.quit(51)
            await asyncio.sleep(0.02)
            assert not task.done()
            assert len(errors) == 1
            assert isinstance(errors[0]["exception"], TypeError)
        finally:
            Application.remove_listener(token)
        Application.quit(53)
        assert await asyncio.wait_for(task, 1) == 53

    asyncio.run(scenario())


def test_owned_decision_can_veto_after_awaiting_asyncio_work():
    async def scenario():
        confirmed = asyncio.Event()
        votes, tasks = [], []

        async def confirm(vote):
            await asyncio.sleep(0.01)
            assert vote.cancel()
            confirmed.set()

        def listener(event):
            if isinstance(event, ApplicationQuitRequestedEvent):
                vote = event.request.defer()
                votes.append(vote)
                tasks.append(asyncio.create_task(confirm(vote)))

        token = Application.add_listener(listener)
        try:
            task = await start_loop()
            Application.quit(55)
            await asyncio.wait_for(confirmed.wait(), 1)
            await asyncio.gather(*tasks)
            assert not task.done()
        finally:
            for vote in votes:
                vote.dispose()
            Application.remove_listener(token)
        Application.quit(57)
        assert await asyncio.wait_for(task, 1) == 57

    asyncio.run(scenario())


@pytest.mark.parametrize("mode", ["pending", "queued"])
def test_interpreter_shutdown_releases_pending_ctypes_callbacks(mode):
    code = textwrap.dedent(
        """\
        import atexit
        import asyncio
        import sys
        import threading

        def checked_shutdown():
            assert _runtime._async_loop is None
            assert len(_runtime._callbacks) == baseline
            print('shutdown-fenced', flush=True)

        atexit.register(checked_shutdown)
        from nativeapi import Application, ApplicationQuitRequestedEvent
        from nativeapi import _runtime, _library
        from ctypes import c_int
        _library.function('nativeapi_py_test_init', None, [])()
        _runtime._start_event_loop = lambda _window: None
        _runtime._pump_event_loop = _library.function(
            'nativeapi_py_test_pump', c_int, [])
        votes = []
        def listener(event):
            if isinstance(event, ApplicationQuitRequestedEvent):
                votes.append(event.request.defer())
        token = Application.add_listener(listener)
        loop = asyncio.new_event_loop()
        pending = loop.create_task(Application.run_async())
        loop.run_until_complete(asyncio.sleep(0))
        baseline = len(_runtime._callbacks)
        if sys.argv[1] == 'pending':
            Application.quit(61)
            assert len(votes) == 1
        else:
            worker = threading.Thread(target=Application.quit, args=(63,))
            worker.start()
            worker.join()
            assert not votes
        assert len(_runtime._callbacks) == baseline + 1
        loop.close()
        """
    )
    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join(
        [str(Path(__file__).resolve().parents[1]), env.get("PYTHONPATH", "")]
    )
    result = subprocess.run(
        [sys.executable, "-c", code, mode],
        env=env,
        text=True,
        capture_output=True,
        timeout=15,
    )
    assert result.returncode == 0, result.stderr
    assert "shutdown-fenced" in result.stdout
    assert "Exception ignored in atexit callback" not in result.stderr
