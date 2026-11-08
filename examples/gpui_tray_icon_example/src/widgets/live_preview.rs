//! Magnified view of the selected tray icon — the counterpart of the
//! Flutter example's `widgets/live_preview.dart`.
//!
//! It paints the animator's `last_frame` — the very image that was just
//! handed to the tray — so the window and the tray can be compared frame by
//! frame.

use dazzui::theme::ActiveTheme;
use dazzui::{Card, CardVariant, StyledExt};
use gpui::prelude::*;
use gpui::{div, px, App, Entity, IntoElement, ParentElement, SharedString, Styled};

use crate::example::TrayExample;
use crate::host;
use crate::tray_controller::{StillIcon, TrayEntry};

pub fn live_preview(entry: &TrayEntry, this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let animator = &entry.animator;
    let name = animator
        .animation
        .map(|animation| animation.label().to_string())
        .unwrap_or_else(|| format!("{} icon", still_label(entry)));
    let state = if !animator.is_playing() {
        "still"
    } else if animator.paused {
        "paused"
    } else {
        "playing"
    };
    let playing = animator.is_playing();
    let frames = animator.frames;
    let measured = animator.measured_fps;
    let render_ms = animator.render_ms;
    let dropped = animator.dropped;
    let pixel_size = animator
        .last_frame
        .as_ref()
        .map(|frame| frame.size)
        .unwrap_or(animator.pixel_size());
    let paused = animator.paused;
    let in_scene = entry.scene.is_some();
    let number = entry.number;

    // The shell picks a template's tint; the preview uses this window's ink
    // so its alpha mask remains visible in either application theme.
    let frame = animator.last_frame.as_ref().map(|frame| {
        if entry.tray_icon.is_icon_template() {
            frame.tinted(host::to_rgb(t.color_content))
        } else {
            frame.bgra.clone()
        }
    });

    let mono = |text: String| {
        div()
            .type_style(host::mono_style(cx))
            .text_color(host::muted_color(cx))
            .child(SharedString::from(text))
    };

    div()
        .flex()
        .w_full()
        .items_center()
        .gap(px(t.spacing_2_5))
        .px(px(t.spacing_2_5))
        .child(
            div()
                .flex()
                .flex_none()
                .items_center()
                .justify_center()
                .size(px(84.))
                .child(
                    Card::new()
                        .size(dazzui::Size::Small)
                        .variant(CardVariant::Sunken)
                        .child(
                            div()
                                .flex()
                                .items_center()
                                .justify_center()
                                .size(px(56.))
                                .children(frame.map(|frame| {
                                    gpui::img(frame)
                                        .size(px(56.))
                                        // Show the real pixels; smoothing
                                        // would hide the resolution.
                                        .into_any_element()
                                })),
                        ),
                ),
        )
        .child(
            div()
                .flex()
                .flex_1()
                .flex_col()
                .gap(px(t.spacing_0_5))
                .child(
                    div()
                        .type_style(t.title_small)
                        .child(SharedString::from(format!("{name} · {state}"))),
                )
                .children(if playing {
                    vec![
                        mono(format!("frame {frames} · {measured:.1} fps")),
                        mono(format!(
                            "render {render_ms:.1} ms · {pixel_size}×{pixel_size} px"
                        )),
                        mono(format!("dropped {dropped} · same frame as tray")),
                    ]
                } else {
                    vec![
                        mono(format!("{pixel_size}×{pixel_size} px · same image as tray")),
                        mono("pick an animation below to play it".to_string()),
                    ]
                })
                .child(
                    div()
                        .flex()
                        .flex_wrap()
                        .gap(px(t.spacing_1))
                        .mt(px(t.spacing_1_5))
                        .child(
                            host::ActionChip::new("pause", if paused { "Resume" } else { "Pause" })
                                .when(playing, |chip| {
                                    let this = this.clone();
                                    chip.on_tap(Some(Box::new(move |_, cx| {
                                        this.update(cx, |this, cx| this.toggle_paused(cx));
                                    })))
                                }),
                        )
                        .child(host::ActionChip::new("step", "Step").when(playing, |chip| {
                            let this = this.clone();
                            chip.on_tap(Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.step_animation(cx));
                            })))
                        }))
                        .child(
                            // Back to the asset icon; a scene also gives back
                            // the title and tooltip it took.
                            host::ActionChip::new("stop", "Stop").when(playing, |chip| {
                                let this = this.clone();
                                chip.on_tap(Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| {
                                        if in_scene {
                                            this.reset_scene(cx);
                                        } else {
                                            this.set_still(
                                                StillIcon::Asset,
                                                Some(number),
                                                false,
                                                cx,
                                            );
                                        }
                                    });
                                })))
                            }),
                        ),
                ),
        )
}

fn still_label(entry: &TrayEntry) -> &'static str {
    match entry.still {
        Some(StillIcon::Asset) => "Asset",
        Some(StillIcon::Drawn) => "Drawn",
        Some(StillIcon::Base64) => "Base64",
        None => "Last frame",
    }
}
