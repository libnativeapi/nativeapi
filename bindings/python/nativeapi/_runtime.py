"""Hand-written runtime for the generated modules: handle ownership, string
and container marshalling, callbacks, and the platform event loop."""

from __future__ import annotations

import asyncio
import atexit
import ctypes
import inspect
import itertools
import signal
import sys
import threading
import traceback
import weakref
from collections.abc import Callable
from ctypes import byref, c_bool, c_char_p, c_int, c_uint64
from typing import Any, TypeVar

from . import _capi as _C
from ._library import function

__all__ = ["NativeApiError", "NativeObject", "is_event_loop_running", "byref"]


class NativeApiError(RuntimeError):
    """A native call failed, e.g. an object could not be created."""


# ---------------------------------------------------------------------------
# Handles
# ---------------------------------------------------------------------------

_T = TypeVar("_T", bound="NativeObject")


class NativeObject:
    """Base of every class wrapping a native object.

    An owned handle is released by `dispose()` (or leaving a `with` block), and
    otherwise when the wrapper is garbage collected. Handles are
    generation-checked, so a call on a released one fails safely instead of
    touching freed memory.
    """

    __slots__ = ("_handle", "_release", "__weakref__")

    _free: Callable[[int], None]

    def __init__(self) -> None:
        raise TypeError(
            f"{type(self).__name__} has no default constructor; use one of its "
            "class methods"
        )

    def _adopt(self, handle: int, owned: bool = True) -> None:
        self._handle = handle
        # The finalizer holds the handle, not the wrapper, so it can run after
        # the wrapper is gone.
        self._release = None
        if owned:
            self._release = weakref.finalize(self, type(self)._free, handle)
            # Releasing at interpreter exit would race the library's own
            # teardown; the process is going away anyway.
            self._release.atexit = False

    @classmethod
    def _owned(cls: type[_T], handle: int) -> _T | None:
        """Wraps a handle the caller owns; the invalid handle becomes None."""
        if not handle:
            return None
        obj = cls.__new__(cls)
        obj._adopt(handle, owned=True)
        return obj

    @classmethod
    def _borrowed(cls: type[_T], handle: int) -> _T | None:
        """Wraps a handle someone else releases (event payloads)."""
        if not handle:
            return None
        obj = cls.__new__(cls)
        obj._adopt(handle, owned=False)
        return obj

    @property
    def native_handle(self) -> int:
        """The raw C ABI handle; 0 once disposed."""
        return self._handle

    def dispose(self) -> None:
        """Releases this reference now. The object itself lives on while
        others hold it."""
        release = self._release
        self._release = None
        self._handle = 0
        if release is not None:
            release()

    def __enter__(self: _T) -> _T:
        return self

    def __exit__(self, *_exc: object) -> None:
        self.dispose()

    def __repr__(self) -> str:
        return f"<{type(self).__name__} handle={self._handle:#x}>"


def handle_of(obj: NativeObject | None) -> int:
    return 0 if obj is None else obj._handle


def read_handles(items, count: int, cls: type[_T]) -> list[_T]:
    """Borrowed wrappers for a C array of handles."""
    if not items:
        return []
    wrappers = (cls._borrowed(items[i]) for i in range(count))
    return [obj for obj in wrappers if obj is not None]


def take_handles(raw, items, cls: type[_T], release) -> list[_T]:
    """Owned wrappers for a returned handle list; frees just the array."""
    try:
        if not items:
            return []
        wrappers = (cls._owned(items[i]) for i in range(raw.count))
        return [obj for obj in wrappers if obj is not None]
    finally:
        release(byref(raw))


# ---------------------------------------------------------------------------
# Strings and containers
# ---------------------------------------------------------------------------


def encode(value: str) -> bytes:
    if not isinstance(value, str):
        raise TypeError(f"expected str, got {type(value).__name__}")
    return value.encode("utf-8")


def encode_optional(value: str | None) -> bytes | None:
    return None if value is None else encode(value)


def decode(value: bytes | None) -> str:
    """A borrowed C string; the C ABI uses NULL for the empty string."""
    return "" if value is None else value.decode("utf-8", errors="replace")


def take_str(ptr: int | None) -> str:
    """An owned C string: copied, then freed."""
    if not ptr:
        return ""
    try:
        return ctypes.string_at(ptr).decode("utf-8", errors="replace")
    finally:
        _C.free_c_str(ptr)


def take_optional_str(ptr: int | None) -> str | None:
    return None if not ptr else take_str(ptr)


def _pointer_array(values: list[bytes]):
    array = (c_char_p * max(len(values), 1))(*values)
    return array, ctypes.cast(array, ctypes.POINTER(c_char_p))


def str_list(values: list[str]) -> _C.native_string_list_t:
    encoded = [encode(value) for value in values]
    array, items = _pointer_array(encoded)
    raw = _C.native_string_list_t(items, len(encoded))
    raw._keep = (array, encoded)  # alive as long as the C value
    return raw


def str_map(values: dict[str, str]) -> _C.native_string_map_t:
    keys = [encode(key) for key in values]
    vals = [encode(value) for value in values.values()]
    key_array, key_items = _pointer_array(keys)
    value_array, value_items = _pointer_array(vals)
    raw = _C.native_string_map_t(key_items, value_items, len(keys))
    raw._keep = (key_array, value_array, keys, vals)
    return raw


def read_str_list(raw) -> list[str]:
    if not raw.items:
        return []
    return [decode(raw.items[i]) for i in range(raw.count)]


def take_str_list(raw) -> list[str]:
    try:
        return read_str_list(raw)
    finally:
        _C.native_string_list_free(byref(raw))


def read_str_map(raw) -> dict[str, str]:
    if not raw.keys or not raw.values:
        return {}
    return {
        decode(raw.keys[i]): decode(raw.values[i])
        for i in range(raw.count)
        if raw.keys[i] is not None
    }


def take_str_map(raw) -> dict[str, str]:
    try:
        return read_str_map(raw)
    finally:
        _C.native_string_map_free(byref(raw))


_E = TypeVar("_E")


def to_enum(cls: Callable[[int], _E], value: int) -> _E | int:
    """`cls(value)`, or the bare int for a value this binding does not know."""
    try:
        return cls(value)
    except ValueError:
        return value


# ---------------------------------------------------------------------------
# Callbacks
# ---------------------------------------------------------------------------

# Each ctypes callback handed to the C ABI stays referenced here for as long
# as the core may call it: the user_data passed with it is its key, and the
# core hands that key to `release_user_data` (on the main thread) once it lets
# the callback go -- listener removed, callback replaced, registration ended,
# owner destroyed, or the call failed.
# The asyncio shim's internal completion is released inline on its last owning
# thread; ctypes reacquires the GIL, and no further platform pump is needed.
_callbacks: dict[int, Any] = {}
_next_key = itertools.count(1)


def _guard(fn: Callable[..., None]) -> Callable[..., None]:
    """An exception must not unwind into C; report it and carry on."""

    def call(*args):
        try:
            fn(*args)
        except BaseException as exc:  # noqa: BLE001 - nothing may escape to C
            _report(exc)

    return call


def _release(key: int | None) -> None:
    if key is not None:
        _callbacks.pop(key, None)


# Passed with every callback; referenced for the life of the process.
release_user_data = _C.native_release_user_data_t(_release)


def make_callback(callback_type, fn: Callable[..., None]):
    return callback_type(_guard(fn))


def user_data(native) -> int | None:
    """The key to pass as user_data with `native`, which keeps it alive until
    the core releases it. None for no callback (None, or a NULL pointer)."""
    if not native:
        return None
    key = next(_next_key)
    _callbacks[key] = native
    return key


def add_listener(add, callback_type, trampoline, *receiver: int) -> int:
    native = make_callback(callback_type, trampoline)
    return add(*receiver, native, user_data(native), release_user_data)


def deliver_event(callback: Callable[[Any], None], event: Any) -> None:
    """Keep callback failure from silently approving a cancellable request."""
    try:
        result = callback(event)
        if inspect.isawaitable(result):
            if inspect.iscoroutine(result):
                result.close()
            raise TypeError(
                "nativeapi: event callbacks must be synchronous; use request.defer() "
                "and resolve the owned decision from an asyncio task"
            )
    except BaseException:
        request = getattr(event, "request", None)
        if request is not None:
            request.cancel()
        raise


def remove_listener(remove, listener_id: int, *receiver: int) -> bool:
    return bool(remove(*receiver, listener_id))


def _report(exc: BaseException) -> None:
    if isinstance(exc, KeyboardInterrupt):
        quit_event_loop(130)
        return
    loop = _async_loop.aio if _async_loop is not None else None
    if loop is not None:
        loop.call_exception_handler(
            {"message": "Exception in a nativeapi callback", "exception": exc}
        )
    else:
        traceback.print_exception(exc, file=sys.stderr)


# ---------------------------------------------------------------------------
# Event loop
# ---------------------------------------------------------------------------

_start_event_loop = function("nativeapi_py_start_event_loop", None, [c_uint64])
_pump_event_loop = function("nativeapi_py_pump_event_loop", c_int, [])
_is_platform_main_thread = function("nativeapi_py_is_main_thread", c_bool, [])
_begin_event_loop = function("nativeapi_py_begin_event_loop", c_uint64, [])
_end_event_loop = function("nativeapi_py_end_event_loop", None, [c_uint64])
_quit_callback_type = ctypes.CFUNCTYPE(None, c_int, ctypes.c_void_p)
_request_event_loop_quit = function(
    "nativeapi_py_request_event_loop_quit",
    c_bool,
    [
        c_uint64,
        c_int,
        _quit_callback_type,
        ctypes.c_void_p,
        _C.native_release_user_data_t,
    ],
)

# How often the platform queue is drained while nothing is happening, in s.
_IDLE_INTERVAL = 0.008


class _AsyncLoop:
    def __init__(
        self, aio: asyncio.AbstractEventLoop, future: asyncio.Future[int]
    ) -> None:
        self.aio = aio
        self.future = future
        self.timer: asyncio.Handle | None = None
        self.session = 0
        self.thread = threading.get_ident()

    def complete(self, exit_code: int) -> None:
        def finish() -> None:
            if _async_loop is self and not self.future.done():
                self.future.set_result(exit_code)

        if threading.get_ident() == self.thread:
            finish()
        else:
            self.aio.call_soon_threadsafe(finish)


_async_loop: _AsyncLoop | None = None
_blocking = False


def _shutdown_async_loop() -> None:
    global _async_loop
    state = _async_loop
    _async_loop = None
    if state is not None and state.session:
        _end_event_loop(state.session)


# End before ctypes trampolines disappear, even when a manually driven asyncio
# loop is closed with an outstanding run_async task or queued worker quit.
atexit.register(_shutdown_async_loop)


def is_event_loop_running() -> bool:
    """Whether `Application.run()` or `Application.run_async()` is active."""
    return _blocking or _async_loop is not None


def _check_can_run() -> None:
    if is_event_loop_running():
        raise RuntimeError("nativeapi: the event loop is already running")
    if threading.current_thread() is not threading.main_thread():
        raise RuntimeError("nativeapi: the event loop must run on the main thread")
    if not _is_platform_main_thread():
        raise RuntimeError(
            "nativeapi: the event loop must run on the platform UI thread"
        )


def run_event_loop(window: NativeObject | None) -> int:
    global _blocking
    _check_can_run()
    # Python only runs signal handlers between bytecodes, and the platform
    # loop blocks inside C; let Ctrl+C terminate the process as usual.
    previous = signal.signal(signal.SIGINT, signal.SIG_DFL)
    _blocking = True
    try:
        if window is None:
            return _C.native_application_run()
        return _C.native_application_run_with_window(window._handle)
    finally:
        _blocking = False
        signal.signal(signal.SIGINT, previous)


async def run_event_loop_async(window: NativeObject | None) -> int:
    global _async_loop
    _check_can_run()
    aio = asyncio.get_running_loop()
    state = _AsyncLoop(aio, aio.create_future())
    _async_loop = state
    try:
        state.session = _begin_event_loop()
        if not state.session:
            raise NativeApiError("nativeapi: could not start the asyncio loop session")
        _start_event_loop(handle_of(window))

        def tick() -> None:
            if state.future.done():
                return
            exit_code = _pump_event_loop()
            if exit_code >= 0:
                # WM_QUIT has already been approved by the native producer,
                # or is a mandatory platform exit. Do not ask a second time.
                state.complete(exit_code)
            if not state.future.done():
                state.timer = aio.call_later(_IDLE_INTERVAL, tick)

        state.timer = aio.call_soon(tick)
        return await state.future
    finally:
        if state.timer is not None:
            state.timer.cancel()
        if state.session:
            _end_event_loop(state.session)
        _async_loop = None


def quit_event_loop(exit_code: int = 0) -> None:
    state = _async_loop
    if state is not None:
        if not state.future.done():
            native = make_callback(
                _quit_callback_type, lambda code, _data: state.complete(code)
            )
            if not _request_event_loop_quit(
                state.session, exit_code, native, user_data(native), release_user_data
            ):
                raise NativeApiError("nativeapi: could not schedule the quit request")
        return
    _C.native_application_quit(exit_code)


_retain_handle = function("native_handle_retain", c_uint64, [c_uint64])
_delivery_active = function("native_event_delivery_is_active", c_bool, [c_uint64])
_delivery_complete = function(
    "native_event_delivery_complete", c_bool, [c_uint64, c_bool]
)


def retain_handle(handle: int) -> int:
    return int(_retain_handle(handle)) if handle else 0


def deliver_callback(delivery: int, invoke: Callable[[], None]) -> None:
    try:
        if _delivery_active(delivery):
            invoke()
    finally:
        _delivery_complete(delivery, True)


async def read_async(read: Callable) -> Any:
    future = asyncio.get_running_loop().create_future()

    def complete(success: bool, value: Any) -> None:
        if future.done():
            return
        if success:
            future.set_result(value)
        else:
            future.set_exception(NativeApiError("nativeapi: operation failed"))

    read(complete)
    return await future


def valid_clipboard_text(value: str | None) -> bool:
    if value is None:
        return True
    try:
        return "\0" not in value and bool(value.encode("utf-8") or value == "")
    except UnicodeEncodeError:
        return False
