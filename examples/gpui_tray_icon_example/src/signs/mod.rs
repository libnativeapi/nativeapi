//! Custom tray content: the sign styles. A macOS tray icon can carry a
//! content view in place of its icon — these build one out of nativeapi
//! views and labels, and draw the same sign inside the window.

pub mod native_sign;
pub mod sign_art;
pub mod sign_controller;
pub mod sign_style;

/// Custom tray signs are available on macOS only, as in the Flutter example.
pub const SIGNS_SUPPORTED: bool = cfg!(target_os = "macos");
