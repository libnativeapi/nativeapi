//! Tray icon example — a GPUI control panel for the nativeapi `TrayIcon`
//! API, the counterpart of `flutter_tray_icon_example`, drawn with the DazzUI
//! design system for GPUI.
//!
//! DazzUI draws the panel; nativeapi does everything it shows: it creates and
//! disposes tray icons, sets their image (a bundled PNG, a CPU-drawn star, a
//! base64 literal, or one of eight animations rendered frame by frame),
//! template mode and tint, title and tooltip, visibility, the context-menu
//! trigger and the menu itself — items, a checkbox, a submenu — and moves the
//! window next to the icon the way a tray popup would, while the footer logs
//! every tray and menu event and a checklist settles itself from real return
//! values. On macOS a tray icon can also carry a custom content view: the
//! sign styles, laid out with nativeapi views and labels.
//!
//! Usage:
//!   cargo run   # in examples/gpui_tray_icon_example

mod checklist;
mod context_menu;
mod example;
mod host;
mod icon_animations;
mod icon_animator;
mod icon_tint;
mod signs;
mod tabs;
mod tray_controller;
mod widgets;

use gpui::{
    px, size, App, AppContext, Application, Bounds, TitlebarOptions, WindowBounds, WindowOptions,
};

use crate::example::TrayExample;
use nativeapi::geometry::Size;
use nativeapi_gpui::{defer_native, WindowExt};

fn main() {
    Application::new()
        .with_assets(dazzui::Assets)
        .run(|cx: &mut App| {
            dazzui::init(cx);
            let options = WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(Bounds::centered(
                    None,
                    size(px(800.), px(600.)),
                    cx,
                ))),
                titlebar: Some(TitlebarOptions {
                    title: Some("Tray Icon".into()),
                    ..Default::default()
                }),
                ..Default::default()
            };
            cx.open_window(options, |window, cx| {
                host::follow_appearance(window, cx);
                let native = window.native_window();
                window.on_window_should_close(cx, |_, cx| {
                    cx.quit();
                    true
                });
                // The example's window geometry is the Flutter example's: a
                // content area of 800×600 with a 720×560 minimum. Sizing the
                // platform window makes it call back into GPUI, which drops
                // callbacks during the open closure — hence deferred.
                // The example's window geometry is set through the native
                // side by id, so the handle stays owned by the entity.
                if let Some(id) = native.as_ref().map(|window| window.id()) {
                    defer_native(cx, move || {
                        if let Some(window) = nativeapi::window_manager::WindowManager::get(id) {
                            window.set_content_size(&Size {
                                width: 800.,
                                height: 600.,
                            });
                            window.set_minimum_size(&Size {
                                width: 720.,
                                height: 560.,
                            });
                        }
                    });
                }
                cx.new(|cx| TrayExample::new(native, window, cx))
            })
            .expect("failed to open the main window");
            cx.activate(true);
        });
}
