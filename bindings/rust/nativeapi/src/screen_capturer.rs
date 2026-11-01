// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.
#![allow(dead_code)]
#![allow(unused_imports)]

use cnativeapi;
use std::ffi::{CStr, CString};

use crate::capture_source::{CaptureSource, CaptureSourceType};
use crate::geometry::{RawOfRectangle, Rectangle};
use crate::image::Image;
use crate::window::Window;

#[derive(Debug)]
pub struct CaptureSourceSelectionOptions {
    pub allow_displays: bool,
    pub allow_windows: bool,
    pub parent_window: Option<Window>,
}

impl CaptureSourceSelectionOptions {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_capture_source_selection_options_t) -> Self {
        Self {
            allow_displays: raw.allow_displays,
            allow_windows: raw.allow_windows,
            parent_window: Window::from_raw(cnativeapi::native_handle_retain(raw.parent_window)),
        }
    }

    pub(crate) fn to_raw(&self) -> RawOfCaptureSourceSelectionOptions {
        let mut raw = cnativeapi::native_capture_source_selection_options_t::default();
        raw.allow_displays = self.allow_displays;
        raw.allow_windows = self.allow_windows;
        raw.parent_window = self.parent_window.as_ref().map_or(0, |v| v.as_raw());
        RawOfCaptureSourceSelectionOptions { raw, _owned: () }
    }
}

/// `CaptureSourceSelectionOptions` in its C form, keeping any borrowed buffers alive.
pub(crate) struct RawOfCaptureSourceSelectionOptions {
    pub(crate) raw: cnativeapi::native_capture_source_selection_options_t,
    _owned: (),
}

#[derive(Debug, Clone, PartialEq)]
pub struct ScreenCaptureOptions {
    pub include_cursor: bool,
    pub crop_enabled: bool,
    pub crop: Rectangle,
}

impl ScreenCaptureOptions {
    pub(crate) unsafe fn from_raw(raw: &cnativeapi::native_screen_capture_options_t) -> Self {
        Self {
            include_cursor: raw.include_cursor,
            crop_enabled: raw.crop_enabled,
            crop: Rectangle::from_raw(&raw.crop),
        }
    }

    pub(crate) fn to_raw(&self) -> RawOfScreenCaptureOptions {
        let mut raw = cnativeapi::native_screen_capture_options_t::default();
        raw.include_cursor = self.include_cursor;
        raw.crop_enabled = self.crop_enabled;
        let crop_nested = self.crop.to_raw();
        raw.crop = crop_nested.raw;
        RawOfScreenCaptureOptions { raw, _owned: (crop_nested, ()) }
    }
}

/// `ScreenCaptureOptions` in its C form, keeping any borrowed buffers alive.
pub(crate) struct RawOfScreenCaptureOptions {
    pub(crate) raw: cnativeapi::native_screen_capture_options_t,
    _owned: (RawOfRectangle, ()),
}

pub struct ScreenCapturer;

impl ScreenCapturer {
    pub fn is_supported() -> bool {
        unsafe {
            cnativeapi::native_screen_capturer_is_supported()
        }
    }

    pub fn is_source_selection_supported() -> bool {
        unsafe {
            cnativeapi::native_screen_capturer_is_source_selection_supported()
        }
    }

    pub fn is_direct_capture_supported(r#type: CaptureSourceType) -> bool {
        unsafe {
            cnativeapi::native_screen_capturer_is_direct_capture_supported(r#type.to_raw())
        }
    }

    pub fn select_source(options: &CaptureSourceSelectionOptions, callback: impl Fn(bool, Option<CaptureSource>) + 'static) {
        let options_raw = options.to_raw();
        unsafe extern "C" fn trampoline(arg0: bool, arg1: u64, delivery: cnativeapi::native_event_delivery_t, user_data: *mut std::ffi::c_void) {
            struct Delivery(cnativeapi::native_event_delivery_t);
            impl Drop for Delivery { fn drop(&mut self) { unsafe { cnativeapi::native_event_delivery_complete(self.0, true); } } }
            let _delivery = Delivery(delivery);
            if !cnativeapi::native_event_delivery_is_active(delivery) { return; }
            if user_data.is_null() {
                return;
            }
            let callback = &*(user_data as *const std::sync::Arc<dyn Fn(bool, Option<CaptureSource>)>);
            let _ = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| callback(arg0, CaptureSource::from_raw(cnativeapi::native_handle_retain(arg1)))));
        }
        fn into_user_data(callback: std::sync::Arc<dyn Fn(bool, Option<CaptureSource>)>) -> *mut std::ffi::c_void {
            Box::into_raw(Box::new(callback)) as *mut std::ffi::c_void
        }
        unsafe extern "C" fn release(user_data: *mut std::ffi::c_void) {
            if !user_data.is_null() {
                drop(Box::from_raw(user_data as *mut std::sync::Arc<dyn Fn(bool, Option<CaptureSource>)>));
            }
        }
        let callback_user_data = into_user_data(std::sync::Arc::new(callback));
        unsafe {
            cnativeapi::native_screen_capturer_select_source(options_raw.raw, Some(trampoline), callback_user_data, Some(release));
        }
    }

    pub fn capture(source: Option<&CaptureSource>, options: &ScreenCaptureOptions, callback: impl Fn(bool, Option<Image>) + 'static) {
        let options_raw = options.to_raw();
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
            cnativeapi::native_screen_capturer_capture(source.map_or(0, |value| value.as_raw()), options_raw.raw, Some(trampoline), callback_user_data, Some(release));
        }
    }

}

