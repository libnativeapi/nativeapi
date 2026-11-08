//! One tray sign: its content, its native views and its preview window —
//! the counterpart of the Flutter example's `signs/sign_controller.dart`.
//!
//! The Flutter example's `EmbeddedSignPreview` hosts the sign's native view
//! inside its window through a macOS PlatformView; GPUI cannot host platform
//! views, so the in-window preview is the drawn replica in `sign_art.rs`,
//! while this controller keeps everything native: the tray's content view
//! and the enlarged preview window, which is a plain nativeapi window with
//! the sign's own views in it.

use std::collections::HashMap;

use nativeapi as na;
use nativeapi::tray_icon::TrayIcon;
use nativeapi::view::View;
use nativeapi::window::Window as NativeWindow;

use super::native_sign::NativeSign;
use super::sign_style::{SignContent, SignStyle, ALL as ALL_STYLES};

const TRAY_SIDE_PADDING: f64 = 4.0;

pub struct SignController {
    pub style: SignStyle,
    contents: HashMap<SignStyle, SignContent>,
    pub green: bool,
    pub english: bool,
    pub right: bool,
    tray_height: f64,
    pub error: Option<String>,
    tray_root: Option<View>,
    tray_sign: Option<NativeSign>,
    preview_window: Option<NativeWindow>,
    preview_root: Option<View>,
    preview_sign: Option<NativeSign>,
}

impl SignController {
    /// Creates the sign and attaches it to `tray` as its content view.
    /// `tray` stays owned by the entry; every later call borrows it.
    pub fn new(initial_style: SignStyle, tray: &TrayIcon, tray_supported: bool) -> Self {
        let mut sign = SignController {
            style: initial_style,
            contents: ALL_STYLES.iter().map(|&s| (s, s.defaults())).collect(),
            green: false,
            english: true,
            right: true,
            tray_height: 32.,
            error: None,
            tray_root: None,
            tray_sign: None,
            preview_window: None,
            preview_root: None,
            preview_sign: None,
        };
        if tray_supported {
            match (View::new(), NativeSign::new(1., None)) {
                (Some(root), Some(tray_sign)) => {
                    root.set_background_color(&na::color::Color {
                        r: 0,
                        g: 0,
                        b: 0,
                        a: 0,
                    });
                    root.add_subview(Some(&tray_sign.view));
                    tray.set_content_view(Some(&root));
                    sign.tray_root = Some(root);
                    sign.tray_sign = Some(tray_sign);
                }
                _ => {
                    sign.error = Some("Could not create the tray sign".to_string());
                }
            }
        }
        sign.update(tray);
        sign
    }

    pub fn content(&self) -> &SignContent {
        self.contents
            .get(&self.style)
            .unwrap_or_else(|| self.contents.get(&SignStyle::Missing).unwrap())
    }

    #[allow(dead_code)]
    pub fn fields(&self) -> &'static [&'static str] {
        self.style.fields()
    }

    pub fn display_title(&self) -> String {
        self.content().heading(self.style)
    }

    pub fn tray_width(&self) -> f64 {
        self.tray_root
            .as_ref()
            .map(|root| root.preferred_size().width)
            .unwrap_or(0.)
    }

    pub fn tray_height(&self) -> f64 {
        self.tray_height
    }

    /// Invalid input leaves the current sign intact: the error message, or
    /// `None` when the content was applied.
    pub fn set_content(
        &mut self,
        primary: &str,
        secondary: &str,
        value: &str,
        detail: &str,
        tray: &TrayIcon,
    ) -> Option<String> {
        let next = SignContent {
            primary: primary.trim().to_string(),
            secondary: secondary.trim().to_string(),
            value: value.trim().to_string(),
            detail: detail.trim().to_string(),
        };
        let problem = next.validate(self.style);
        if problem.is_some() {
            return problem;
        }
        self.contents.insert(self.style, next);
        self.update(tray);
        None
    }

    pub fn select_style(&mut self, index: usize, tray: &TrayIcon) {
        let Some(&style) = ALL_STYLES.get(index) else {
            return;
        };
        self.style = style;
        self.english = true;
        self.right = true;
        self.update(tray);
    }

    pub fn restore_defaults(&mut self, tray: &TrayIcon) {
        self.contents.insert(self.style, self.style.defaults());
        self.english = true;
        self.right = true;
        if self.style == SignStyle::Missing {
            self.green = false;
        }
        self.update(tray);
    }

    pub fn set_green(&mut self, value: bool, tray: &TrayIcon) {
        self.green = value;
        self.update(tray);
    }

    pub fn set_english(&mut self, value: bool, tray: &TrayIcon) {
        self.english = self.style == SignStyle::Missing || value;
        self.update(tray);
    }

    pub fn set_right(&mut self, value: bool, tray: &TrayIcon) {
        self.right = value;
        self.update(tray);
    }

    pub fn show_preview(&mut self, tray: &TrayIcon) {
        if self.preview_window.is_none() {
            let Some(window) = NativeWindow::new() else {
                self.error = Some("Could not create preview window".to_string());
                return;
            };
            let Some(root) = window.content_view() else {
                self.error = Some("Could not create preview window".to_string());
                return;
            };
            let Some(sign) = NativeSign::new(4., None) else {
                self.error = Some("Could not create the preview sign".to_string());
                return;
            };
            root.set_background_color(&na::color::Color {
                r: 238,
                g: 241,
                b: 237,
                a: 255,
            });
            root.add_subview(Some(&sign.view));
            window.set_title("Enlarged sign preview");
            window.set_resizable(false);
            self.preview_window = Some(window);
            self.preview_root = Some(root);
            self.preview_sign = Some(sign);
            self.update(tray);
            if let Some(window) = self.preview_window.as_ref() {
                window.center();
            }
        }
        if let Some(window) = self.preview_window.as_ref() {
            window.show();
            window.focus();
        }
    }

    /// The sign grew or shrank: fit the preview window around it again.
    fn layout_preview(&mut self) {
        let Some(sign) = self.preview_sign.as_ref() else {
            return;
        };
        let Some(window) = self.preview_window.as_ref() else {
            return;
        };
        window.set_content_size(&na::geometry::Size {
            width: sign.width() + 48.,
            height: sign.height() + 48.,
        });
        sign.view.set_frame(&na::geometry::Rectangle {
            x: 24.,
            y: 24.,
            width: sign.width(),
            height: sign.height(),
        });
    }

    /// The tray's menu-bar height may only settle after placement: adopt a
    /// new one and rebuild the sign. Returns whether it changed.
    pub fn settle_tray_height(&mut self, height: f64) -> bool {
        if height > 0. && (height - self.tray_height).abs() > f64::EPSILON {
            self.tray_height = height;
            true
        } else {
            false
        }
    }

    /// Reads the tray's height, rebuilds both signs and syncs the tooltip.
    pub fn update(&mut self, tray: &TrayIcon) {
        let available = tray.get_bounds().height;
        if available > 0. {
            self.tray_height = available;
        }
        let content = self.content().clone();
        let (style, green, english, right, bar_height) = (
            self.style,
            self.green,
            self.english || matches!(self.style, SignStyle::Missing | SignStyle::Travel),
            self.right,
            self.tray_height,
        );
        for sign in [&mut self.tray_sign, &mut self.preview_sign]
            .into_iter()
            .flatten()
        {
            sign.update(style, &content, green, english, right, bar_height);
        }
        let action = "Click to edit, right-click for menu";
        tray.set_tooltip(Some(&format!(
            "{} · {} · {}",
            style.label(),
            self.display_title(),
            action
        )));
        self.layout_tray();
        self.layout_preview();
    }

    /// Keeps the sign's spacing outside its painted face, on the transparent
    /// host view the tray centers.
    fn layout_tray(&mut self) {
        let Some(sign) = self.tray_sign.as_ref() else {
            return;
        };
        let Some(root) = self.tray_root.as_ref() else {
            return;
        };
        sign.view.set_frame(&na::geometry::Rectangle {
            x: TRAY_SIDE_PADDING,
            y: 0.,
            width: sign.width(),
            height: sign.height(),
        });
        root.set_preferred_size(&na::geometry::Size {
            width: sign.width() + TRAY_SIDE_PADDING * 2.,
            height: sign.height(),
        });
    }

    /// A window event arrived: if it closed the preview window, let it go.
    /// Returns whether it did.
    pub fn on_window_closed(&mut self, window_id: u32, close: bool) -> bool {
        let Some(window) = self.preview_window.as_ref() else {
            return false;
        };
        if window.id() != window_id {
            return false;
        }
        self.release_preview(close);
        true
    }

    pub fn release_preview(&mut self, close: bool) {
        let root = self.preview_root.take();
        let sign = self.preview_sign.take();
        let window = self.preview_window.take();
        let Some(window) = window else {
            return;
        };
        if let Some(root) = root.as_ref() {
            root.clear_subviews();
        }
        drop(sign);
        drop(root);
        if close {
            window.close();
        }
    }

    pub fn release_tray(&mut self, tray: &TrayIcon) {
        tray.set_content_view(None);
        if let Some(root) = self.tray_root.as_ref() {
            root.clear_subviews();
        }
        self.tray_sign = None;
        self.tray_root = None;
    }
}
