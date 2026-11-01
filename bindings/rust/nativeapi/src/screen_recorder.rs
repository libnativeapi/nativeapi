// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

use crate::capture_source::CaptureSource;
use crate::screen_capturer::{RawOfScreenCaptureOptions, ScreenCaptureOptions};

/// Identifies one registered event listener.
pub type ListenerId = cnativeapi::native_listener_id_t;

#[repr(i32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum ScreenRecordingState {
    Idle = 0,
    Starting = 1,
    Recording = 2,
    Stopping = 3,
    Completed = 4,
    Cancelled = 5,
    Failed = 6,
}

impl ScreenRecordingState {
    pub(crate) fn from_raw(raw: cnativeapi::native_screen_recording_state_t) -> Self {
        match raw {
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_IDLE => Self::Idle,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_STARTING => Self::Starting,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_RECORDING => Self::Recording,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_STOPPING => Self::Stopping,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_COMPLETED => Self::Completed,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_CANCELLED => Self::Cancelled,
            cnativeapi::NATIVE_SCREEN_RECORDING_STATE_FAILED => Self::Failed,
            _ => Self::Idle,
        }
    }

    pub(crate) fn to_raw(self) -> cnativeapi::native_screen_recording_state_t {
        self as cnativeapi::native_screen_recording_state_t
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ScreenRecordingOptions {
    pub capture: ScreenCaptureOptions,
    pub frame_rate: f64,
    pub video_bitrate: std::os::raw::c_ulong,
}

impl ScreenRecordingOptions {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_screen_recording_options_t) -> Self {
        Self {
            capture: ScreenCaptureOptions::from_raw(&raw.capture),
            frame_rate: raw.frame_rate,
            video_bitrate: raw.video_bitrate,
        }
    }

    pub(crate) fn to_raw(&self) -> RawOfScreenRecordingOptions {
        let mut raw = cnativeapi::native_screen_recording_options_t::default();
        let capture_nested = self.capture.to_raw();
        raw.capture = capture_nested.raw;
        raw.frame_rate = self.frame_rate;
        raw.video_bitrate = self.video_bitrate;
        RawOfScreenRecordingOptions { raw, _owned: (capture_nested, ()) }
    }
}

/// `ScreenRecordingOptions` in its C form, keeping any borrowed buffers alive.
pub(crate) struct RawOfScreenRecordingOptions {
    pub(crate) raw: cnativeapi::native_screen_recording_options_t,
    _owned: (RawOfScreenCaptureOptions, ()),
}

/// One `ScreenRecordingEvent`, in its concrete form.
#[derive(Debug, Clone, PartialEq)]
pub enum ScreenRecordingEvent {
    Started,
    Completed { file_path: Option<String>, duration: f64 },
    Cancelled,
    Failed { message: Option<String> },
}

impl ScreenRecordingEvent {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_screen_recording_event_t) -> Option<Self> {
        Some(match raw.type_ {
            cnativeapi::NATIVE_SCREEN_RECORDING_EVENT_TYPE_STARTED => Self::Started,
            cnativeapi::NATIVE_SCREEN_RECORDING_EVENT_TYPE_COMPLETED => Self::Completed { file_path: if raw.data.completed.file_path.is_null() { None } else { Some(CStr::from_ptr(raw.data.completed.file_path).to_string_lossy().into_owned()) }, duration: raw.data.completed.duration },
            cnativeapi::NATIVE_SCREEN_RECORDING_EVENT_TYPE_CANCELLED => Self::Cancelled,
            cnativeapi::NATIVE_SCREEN_RECORDING_EVENT_TYPE_FAILED => Self::Failed { message: if raw.data.failed.message.is_null() { None } else { Some(CStr::from_ptr(raw.data.failed.message).to_string_lossy().into_owned()) } },
            _ => return None,
        })
    }
}

/// Owned handle to a native `ScreenRecorder`.
#[derive(Debug)]
#[repr(transparent)]
pub struct ScreenRecorder {
    handle: cnativeapi::native_screen_recorder_t,
}

impl ScreenRecorder {
    /// Adopts a handle returned by the C API.
    ///
    /// # Safety
    /// `handle` must be a live handle that is not owned elsewhere.
    pub unsafe fn from_raw(handle: cnativeapi::native_screen_recorder_t) -> Option<Self> {
        if handle == 0 {
            None
        } else {
            Some(Self { handle })
        }
    }

    /// Borrows the underlying C handle.
    pub fn as_raw(&self) -> cnativeapi::native_screen_recorder_t {
        self.handle
    }

    /// Creates a new `ScreenRecorder`; returns `None` if the native side failed.
    pub fn new() -> Option<Self> {
        unsafe {
            Self::from_raw(cnativeapi::native_screen_recorder_create())
        }
    }

    pub fn is_supported() -> bool {
        unsafe {
            cnativeapi::native_screen_recorder_is_supported()
        }
    }

    pub fn start(&self, source: Option<&CaptureSource>, file_path: &str, options: &ScreenRecordingOptions) -> bool {
        let file_path_native = CString::new(file_path).expect("string argument contains interior nul byte");
        let options_raw = options.to_raw();
        unsafe {
            cnativeapi::native_screen_recorder_start(self.handle, source.map_or(0, |value| value.as_raw()), file_path_native.as_ptr(), options_raw.raw)
        }
    }

    pub fn stop(&self) -> bool {
        unsafe {
            cnativeapi::native_screen_recorder_stop(self.handle)
        }
    }

    pub fn cancel(&self) -> bool {
        unsafe {
            cnativeapi::native_screen_recorder_cancel(self.handle)
        }
    }

    pub fn state(&self) -> ScreenRecordingState {
        unsafe {
            ScreenRecordingState::from_raw(cnativeapi::native_screen_recorder_get_state(self.handle))
        }
    }

    pub fn is_active(&self) -> bool {
        unsafe {
            cnativeapi::native_screen_recorder_is_active(self.handle)
        }
    }

    pub fn duration(&self) -> f64 {
        unsafe {
            cnativeapi::native_screen_recorder_get_duration(self.handle)
        }
    }

    pub fn file_path(&self) -> Option<String> {
        unsafe {
            let ptr = cnativeapi::native_screen_recorder_get_file_path(self.handle);
            if ptr.is_null() {
                return None;
            }
            let value = CStr::from_ptr(ptr).to_string_lossy().into_owned();
            cnativeapi::free_c_str(ptr);
            Some(value)
        }
    }

    /// Registers `callback` for every `ScreenRecordingEvent` this `ScreenRecorder` emits.
    ///
    /// The closure is dropped on the main thread once the listener is removed
    /// or its emitter destroyed.
    pub fn add_listener(&self, callback: impl Fn(&ScreenRecordingEvent) + 'static) -> ListenerId {
        unsafe extern "C" fn trampoline(event: *const cnativeapi::native_screen_recording_event_t, user_data: *mut std::ffi::c_void) {
            if event.is_null() || user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const Box<dyn Fn(&ScreenRecordingEvent)>);
            if let Some(event) = ScreenRecordingEvent::from_raw(&*event) {
                callback(&event);
            }
        }
        let boxed: Box<Box<dyn Fn(&ScreenRecordingEvent)>> = Box::new(Box::new(callback));
        let user_data = Box::into_raw(boxed) as *mut std::ffi::c_void;
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            drop(Box::from_raw(user_data as *mut Box<dyn Fn(&ScreenRecordingEvent)>));
        }
        unsafe { cnativeapi::native_screen_recorder_add_listener(self.handle, Some(trampoline), user_data, Some(release)) }
    }

    /// Unregisters a listener. Returns false if unknown.
    pub fn remove_listener(&self, listener_id: ListenerId) -> bool {
        unsafe { cnativeapi::native_screen_recorder_remove_listener(self.handle, listener_id) }
    }

}

impl Drop for ScreenRecorder {
    fn drop(&mut self) {
        if self.handle != 0 {
            unsafe { cnativeapi::native_screen_recorder_free(self.handle) };
        }
    }
}

/// A `ScreenRecorder` owned elsewhere; using it does not release it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ScreenRecorderRef(cnativeapi::native_screen_recorder_t);

impl ScreenRecorderRef {
    pub(crate) fn from_raw(handle: cnativeapi::native_screen_recorder_t) -> Self {
        Self(handle)
    }

    pub fn as_raw(&self) -> cnativeapi::native_screen_recorder_t {
        self.0
    }

    /// Runs `f` against the borrowed handle. The `ScreenRecorder` it receives
    /// must not outlive the call.
    pub fn with<R>(&self, f: impl FnOnce(&ScreenRecorder) -> R) -> R {
        let borrowed = std::mem::ManuallyDrop::new(ScreenRecorder { handle: self.0 });
        f(&borrowed)
    }
}

