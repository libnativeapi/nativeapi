// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

/// Identifies one registered event listener.
pub type ListenerId = cnativeapi::native_listener_id_t;

/// One `ProtocolHandlerEvent`, in its concrete form.
#[derive(Debug, Clone, PartialEq)]
pub enum ProtocolHandlerEvent {
    ProtocolUrlReceived { url: Option<String> },
}

impl ProtocolHandlerEvent {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_protocol_handler_event_t) -> Option<Self> {
        Some(match raw.type_ {
            cnativeapi::NATIVE_PROTOCOL_HANDLER_EVENT_TYPE_PROTOCOL_URL_RECEIVED => Self::ProtocolUrlReceived { url: if raw.data.protocol_url_received.url.is_null() { None } else { Some(CStr::from_ptr(raw.data.protocol_url_received.url).to_string_lossy().into_owned()) } },
            _ => return None,
        })
    }
}

pub struct ProtocolHandler;

impl ProtocolHandler {
    pub fn is_supported() -> bool {
        unsafe {
            cnativeapi::native_protocol_handler_is_supported()
        }
    }

    pub fn register(scheme: &str) -> bool {
        let scheme_native = CString::new(scheme).expect("string argument contains interior nul byte");
        unsafe {
            cnativeapi::native_protocol_handler_register(scheme_native.as_ptr())
        }
    }

    pub fn get_initial_url() -> Option<String> {
        unsafe {
            let ptr = cnativeapi::native_protocol_handler_get_initial_url();
            if ptr.is_null() {
                return None;
            }
            let value = CStr::from_ptr(ptr).to_string_lossy().into_owned();
            cnativeapi::free_c_str(ptr);
            Some(value)
        }
    }

    /// Registers `callback` for every `ProtocolHandlerEvent` this `ProtocolHandler` emits.
    ///
    /// The closure is dropped on the main thread once the listener is removed
    /// or its emitter destroyed.
    pub fn add_listener(callback: impl Fn(&ProtocolHandlerEvent) + 'static) -> ListenerId {
        unsafe extern "C" fn trampoline(event: *const cnativeapi::native_protocol_handler_event_t, user_data: *mut std::ffi::c_void) {
            if event.is_null() || user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const Box<dyn Fn(&ProtocolHandlerEvent)>);
            if let Some(event) = ProtocolHandlerEvent::from_raw(&*event) {
                callback(&event);
            }
        }
        let boxed: Box<Box<dyn Fn(&ProtocolHandlerEvent)>> = Box::new(Box::new(callback));
        let user_data = Box::into_raw(boxed) as *mut std::ffi::c_void;
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            drop(Box::from_raw(user_data as *mut Box<dyn Fn(&ProtocolHandlerEvent)>));
        }
        unsafe { cnativeapi::native_protocol_handler_add_listener(Some(trampoline), user_data, Some(release)) }
    }

    /// Unregisters a listener. Returns false if unknown.
    pub fn remove_listener(listener_id: ListenerId) -> bool {
        unsafe { cnativeapi::native_protocol_handler_remove_listener(listener_id) }
    }

}

