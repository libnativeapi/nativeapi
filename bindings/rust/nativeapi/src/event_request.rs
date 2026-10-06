// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

/// Owned handle to a native `EventDecision`.
#[derive(Debug)]
#[repr(transparent)]
pub struct EventDecision {
    handle: cnativeapi::native_event_decision_t,
}

impl EventDecision {
    /// Adopts a handle returned by the C API.
    ///
    /// # Safety
    /// `handle` must be a live handle that is not owned elsewhere.
    pub unsafe fn from_raw(handle: cnativeapi::native_event_decision_t) -> Option<Self> {
        if handle == 0 {
            None
        } else {
            Some(Self { handle })
        }
    }

    /// Borrows the underlying C handle.
    pub fn as_raw(&self) -> cnativeapi::native_event_decision_t {
        self.handle
    }

    pub fn accept(&self) -> bool {
        unsafe {
            cnativeapi::native_event_decision_accept(self.handle)
        }
    }

    pub fn cancel(&self) -> bool {
        unsafe {
            cnativeapi::native_event_decision_cancel(self.handle)
        }
    }

    pub fn is_pending(&self) -> bool {
        unsafe {
            cnativeapi::native_event_decision_is_pending(self.handle)
        }
    }

}

impl Drop for EventDecision {
    fn drop(&mut self) {
        if self.handle != 0 {
            unsafe { cnativeapi::native_event_decision_free(self.handle) };
        }
    }
}

/// A `EventDecision` owned elsewhere; using it does not release it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct EventDecisionRef(cnativeapi::native_event_decision_t);

impl EventDecisionRef {
    pub(crate) fn from_raw(handle: cnativeapi::native_event_decision_t) -> Self {
        Self(handle)
    }

    pub fn as_raw(&self) -> cnativeapi::native_event_decision_t {
        self.0
    }

    /// Runs `f` against the borrowed handle. The `EventDecision` it receives
    /// must not outlive the call.
    pub fn with<R>(&self, f: impl FnOnce(&EventDecision) -> R) -> R {
        let borrowed = std::mem::ManuallyDrop::new(EventDecision { handle: self.0 });
        f(&borrowed)
    }
}

/// Owned handle to a native `EventRequest`.
#[derive(Debug)]
#[repr(transparent)]
pub struct EventRequest {
    handle: cnativeapi::native_event_request_t,
}

impl EventRequest {
    /// Adopts a handle returned by the C API.
    ///
    /// # Safety
    /// `handle` must be a live handle that is not owned elsewhere.
    pub unsafe fn from_raw(handle: cnativeapi::native_event_request_t) -> Option<Self> {
        if handle == 0 {
            None
        } else {
            Some(Self { handle })
        }
    }

    /// Borrows the underlying C handle.
    pub fn as_raw(&self) -> cnativeapi::native_event_request_t {
        self.handle
    }

    pub fn is_cancelable(&self) -> bool {
        unsafe {
            cnativeapi::native_event_request_is_cancelable(self.handle)
        }
    }

    pub fn is_cancelled(&self) -> bool {
        unsafe {
            cnativeapi::native_event_request_is_cancelled(self.handle)
        }
    }

    pub fn is_pending(&self) -> bool {
        unsafe {
            cnativeapi::native_event_request_is_pending(self.handle)
        }
    }

    pub fn cancel(&self) -> bool {
        unsafe {
            cnativeapi::native_event_request_cancel(self.handle)
        }
    }

    pub fn defer(&self) -> Option<EventDecision> {
        unsafe {
            EventDecision::from_raw(cnativeapi::native_event_request_defer(self.handle))
        }
    }

}

impl Drop for EventRequest {
    fn drop(&mut self) {
        if self.handle != 0 {
            unsafe { cnativeapi::native_event_request_free(self.handle) };
        }
    }
}

/// A `EventRequest` owned elsewhere; using it does not release it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct EventRequestRef(cnativeapi::native_event_request_t);

impl EventRequestRef {
    pub(crate) fn from_raw(handle: cnativeapi::native_event_request_t) -> Self {
        Self(handle)
    }

    pub fn as_raw(&self) -> cnativeapi::native_event_request_t {
        self.0
    }

    /// Runs `f` against the borrowed handle. The `EventRequest` it receives
    /// must not outlive the call.
    pub fn with<R>(&self, f: impl FnOnce(&EventRequest) -> R) -> R {
        let borrowed = std::mem::ManuallyDrop::new(EventRequest { handle: self.0 });
        f(&borrowed)
    }
}

