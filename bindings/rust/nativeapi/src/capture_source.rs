// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

use crate::display::Display;
use crate::window::Window;

#[repr(i32)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum CaptureSourceType {
    None = 0,
    Display = 1,
    Window = 2,
    Unknown = 3,
}

impl CaptureSourceType {
    pub(crate) fn from_raw(raw: cnativeapi::native_capture_source_type_t) -> Self {
        match raw {
            cnativeapi::NATIVE_CAPTURE_SOURCE_TYPE_NONE => Self::None,
            cnativeapi::NATIVE_CAPTURE_SOURCE_TYPE_DISPLAY => Self::Display,
            cnativeapi::NATIVE_CAPTURE_SOURCE_TYPE_WINDOW => Self::Window,
            cnativeapi::NATIVE_CAPTURE_SOURCE_TYPE_UNKNOWN => Self::Unknown,
            _ => Self::None,
        }
    }

    pub(crate) fn to_raw(self) -> cnativeapi::native_capture_source_type_t {
        self as cnativeapi::native_capture_source_type_t
    }
}

/// Owned handle to a native `CaptureSource`.
#[derive(Debug)]
#[repr(transparent)]
pub struct CaptureSource {
    handle: cnativeapi::native_capture_source_t,
}

impl CaptureSource {
    /// Adopts a handle returned by the C API.
    ///
    /// # Safety
    /// `handle` must be a live handle that is not owned elsewhere.
    pub unsafe fn from_raw(handle: cnativeapi::native_capture_source_t) -> Option<Self> {
        if handle == 0 {
            None
        } else {
            Some(Self { handle })
        }
    }

    /// Borrows the underlying C handle.
    pub fn as_raw(&self) -> cnativeapi::native_capture_source_t {
        self.handle
    }

    pub fn from_display(display: Option<&Display>) -> Option<CaptureSource> {
        unsafe {
            CaptureSource::from_raw(cnativeapi::native_capture_source_from_display(display.map_or(0, |value| value.as_raw())))
        }
    }

    pub fn from_window(window: Option<&Window>) -> Option<CaptureSource> {
        unsafe {
            CaptureSource::from_raw(cnativeapi::native_capture_source_from_window(window.map_or(0, |value| value.as_raw())))
        }
    }

    pub fn r#type(&self) -> CaptureSourceType {
        unsafe {
            CaptureSourceType::from_raw(cnativeapi::native_capture_source_get_type(self.handle))
        }
    }

    pub fn name(&self) -> Option<String> {
        unsafe {
            let ptr = cnativeapi::native_capture_source_get_name(self.handle);
            if ptr.is_null() {
                return None;
            }
            let value = CStr::from_ptr(ptr).to_string_lossy().into_owned();
            cnativeapi::free_c_str(ptr);
            Some(value)
        }
    }

}

impl Drop for CaptureSource {
    fn drop(&mut self) {
        if self.handle != 0 {
            unsafe { cnativeapi::native_capture_source_free(self.handle) };
        }
    }
}

/// A `CaptureSource` owned elsewhere; using it does not release it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CaptureSourceRef(cnativeapi::native_capture_source_t);

impl CaptureSourceRef {
    pub(crate) fn from_raw(handle: cnativeapi::native_capture_source_t) -> Self {
        Self(handle)
    }

    pub fn as_raw(&self) -> cnativeapi::native_capture_source_t {
        self.0
    }

    /// Runs `f` against the borrowed handle. The `CaptureSource` it receives
    /// must not outlive the call.
    pub fn with<R>(&self, f: impl FnOnce(&CaptureSource) -> R) -> R {
        let borrowed = std::mem::ManuallyDrop::new(CaptureSource { handle: self.0 });
        f(&borrowed)
    }
}

