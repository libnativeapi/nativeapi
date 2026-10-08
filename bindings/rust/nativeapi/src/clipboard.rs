// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

use crate::image::Image;

/// Identifies one registered event listener.
pub type ListenerId = cnativeapi::native_listener_id_t;

#[derive(Debug)]
pub struct ClipboardData {
    pub text: Option<String>,
    pub html: Option<String>,
    pub image: Option<Image>,
    pub file_paths: Vec<String>,
}

impl ClipboardData {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_clipboard_data_t) -> Self {
        Self {
            text: if raw.text.is_null() { None } else { Some(CStr::from_ptr(raw.text).to_string_lossy().into_owned()) },
            html: if raw.html.is_null() { None } else { Some(CStr::from_ptr(raw.html).to_string_lossy().into_owned()) },
            image: Image::from_raw(cnativeapi::native_handle_retain(raw.image)),
            file_paths: if raw.file_paths.items.is_null() { Vec::new() } else { (0..raw.file_paths.count as usize).filter_map(|index| { let ptr = *raw.file_paths.items.add(index); if ptr.is_null() { None } else { Some(CStr::from_ptr(ptr).to_string_lossy().into_owned()) } }).collect() },
        }
    }

    pub(crate) fn to_raw(&self) -> RawOfClipboardData {
        let mut raw = cnativeapi::native_clipboard_data_t::default();
        let text_owned = self.text.as_deref().map(|v| CString::new(v).expect("interior NUL"));
        raw.text = text_owned.as_ref().map_or(std::ptr::null_mut(), |v| v.as_ptr() as *mut _);
        let html_owned = self.html.as_deref().map(|v| CString::new(v).expect("interior NUL"));
        raw.html = html_owned.as_ref().map_or(std::ptr::null_mut(), |v| v.as_ptr() as *mut _);
        raw.image = self.image.as_ref().map_or(0, |v| v.as_raw());
        let file_paths_owned: Vec<CString> = self.file_paths.iter().map(|v| CString::new(v.as_str()).expect("interior NUL")).collect();
        let mut file_paths_items: Vec<*mut std::os::raw::c_char> = file_paths_owned.iter().map(|v| v.as_ptr() as *mut _).collect();
        raw.file_paths = cnativeapi::native_string_list_t { items: file_paths_items.as_mut_ptr(), count: file_paths_items.len() as _ };
        RawOfClipboardData { raw, _owned: (text_owned, html_owned, file_paths_owned, file_paths_items) }
    }
}

/// `ClipboardData` in its C form, keeping any borrowed buffers alive.
pub(crate) struct RawOfClipboardData {
    pub(crate) raw: cnativeapi::native_clipboard_data_t,
    _owned: (Option<CString>, Option<CString>, Vec<CString>, Vec<*mut std::os::raw::c_char>),
}

/// One `ClipboardEvent`, in its concrete form.
#[derive(Debug, Clone, PartialEq)]
pub enum ClipboardEvent {
    Changed,
}

impl ClipboardEvent {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_clipboard_event_t) -> Option<Self> {
        Some(match raw.type_ {
            cnativeapi::NATIVE_CLIPBOARD_EVENT_TYPE_CHANGED => Self::Changed,
            _ => return None,
        })
    }
}

pub struct Clipboard;

impl Clipboard {
    pub fn is_supported() -> bool {
        unsafe {
            cnativeapi::native_clipboard_is_supported()
        }
    }

    pub fn is_change_monitoring_supported() -> bool {
        unsafe {
            cnativeapi::native_clipboard_is_change_monitoring_supported()
        }
    }

    pub fn read(callback: impl Fn(bool, ClipboardData) + 'static) {
        unsafe extern "C" fn trampoline(arg0: bool, arg1: *const cnativeapi::native_clipboard_data_t, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, ClipboardData)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, ClipboardData::from_raw(&(*arg1)))));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, ClipboardData)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, ClipboardData)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_clipboard_read(Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn read_text(callback: impl Fn(bool, Option<String>) + 'static) {
        unsafe extern "C" fn trampoline(arg0: bool, arg1: *const std::os::raw::c_char, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, Option<String>)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, if arg1.is_null() { None } else { Some(CStr::from_ptr(arg1).to_string_lossy().into_owned()) })));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, Option<String>)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, Option<String>)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_clipboard_read_text(Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn read_html(callback: impl Fn(bool, Option<String>) + 'static) {
        unsafe extern "C" fn trampoline(arg0: bool, arg1: *const std::os::raw::c_char, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, Option<String>)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, if arg1.is_null() { None } else { Some(CStr::from_ptr(arg1).to_string_lossy().into_owned()) })));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, Option<String>)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, Option<String>)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_clipboard_read_html(Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn read_image(callback: impl Fn(bool, Option<Image>) + 'static) {
        unsafe extern "C" fn trampoline(arg0: bool, arg1: u64, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, Option<Image>)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, Image::from_raw(cnativeapi::native_handle_retain(arg1)))));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, Option<Image>)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, Option<Image>)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_clipboard_read_image(Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn read_file_paths(callback: impl Fn(bool, Vec<String>) + 'static) {
        unsafe extern "C" fn trampoline(arg0: bool, arg1: *const cnativeapi::native_string_list_t, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, Vec<String>)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, if (*arg1).items.is_null() { Vec::new() } else { (0..(*arg1).count as usize).filter_map(|index| { let ptr = *(*arg1).items.add(index); if ptr.is_null() { None } else { Some(CStr::from_ptr(ptr).to_string_lossy().into_owned()) } }).collect() })));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, Vec<String>)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, Vec<String>)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_clipboard_read_file_paths(Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn write(data: &ClipboardData) -> bool {
        if data.text.as_ref().is_some_and(|s| s.contains('\0')) || data.html.as_ref().is_some_and(|s| s.contains('\0')) || data.file_paths.iter().any(|s| s.contains('\0')) { return false; }
        let data_raw = data.to_raw();
        unsafe {
            cnativeapi::native_clipboard_write(data_raw.raw)
        }
    }

    pub fn write_text(text: &str) -> bool {
        if text.contains('\0') { return false; }
        let text_native = CString::new(text).expect("string argument contains interior nul byte");
        unsafe {
            cnativeapi::native_clipboard_write_text(text_native.as_ptr())
        }
    }

    pub fn write_html(html: &str) -> bool {
        if html.contains('\0') { return false; }
        let html_native = CString::new(html).expect("string argument contains interior nul byte");
        unsafe {
            cnativeapi::native_clipboard_write_html(html_native.as_ptr())
        }
    }

    pub fn write_image(image: Option<&Image>) -> bool {
        unsafe {
            cnativeapi::native_clipboard_write_image(image.map_or(0, |value| value.as_raw()))
        }
    }

    pub fn write_file_paths(file_paths: &[String]) -> bool {
        if file_paths.iter().any(|s| s.contains('\0')) { return false; }
        let file_paths_owned: Vec<CString> = file_paths
            .iter()
            .map(|value| CString::new(value.as_str()).unwrap_or_default())
            .collect();
        let mut file_paths_ptrs: Vec<*mut std::os::raw::c_char> =
            file_paths_owned.iter().map(|value| value.as_ptr() as *mut _).collect();
        let file_paths_native = cnativeapi::native_string_list_t {
            items: file_paths_ptrs.as_mut_ptr(),
            count: file_paths_ptrs.len() as std::os::raw::c_long,
        };
        unsafe {
            cnativeapi::native_clipboard_write_file_paths(file_paths_native)
        }
    }

    pub fn clear() -> bool {
        unsafe {
            cnativeapi::native_clipboard_clear()
        }
    }

    pub fn is_monitoring() -> bool {
        unsafe {
            cnativeapi::native_clipboard_is_monitoring()
        }
    }

    /// Registers `callback` for every `ClipboardEvent` this `Clipboard` emits.
    ///
    /// The closure is dropped on the main thread once the listener is removed
    /// or its emitter destroyed.
    pub fn add_listener(callback: impl Fn(&ClipboardEvent) + 'static) -> ListenerId {
        unsafe extern "C" fn trampoline(event: *const cnativeapi::native_clipboard_event_t, user_data: *mut std::ffi::c_void) {
            if event.is_null() || user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const Box<dyn Fn(&ClipboardEvent)>);
            if let Some(event) = ClipboardEvent::from_raw(&*event) {
                callback(&event);
            }
        }
        let boxed: Box<Box<dyn Fn(&ClipboardEvent)>> = Box::new(Box::new(callback));
        let user_data = Box::into_raw(boxed) as *mut std::ffi::c_void;
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            drop(Box::from_raw(user_data as *mut Box<dyn Fn(&ClipboardEvent)>));
        }
        unsafe { cnativeapi::native_clipboard_add_listener(Some(trampoline), user_data, Some(release)) }
    }

    /// Unregisters a listener. Returns false if unknown.
    pub fn remove_listener(listener_id: ListenerId) -> bool {
        unsafe { cnativeapi::native_clipboard_remove_listener(listener_id) }
    }

}

