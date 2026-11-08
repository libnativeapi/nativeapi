//! The gallery: every tile is alive, one click plays it in the tray — the
//! counterpart of the Flutter example's `tabs/animate_tab.dart`.

use std::sync::Arc;

use dazzui::theme::ActiveTheme;
use dazzui::{OptionCard, Tint};
use gpui::{div, px, AnyElement, App, Entity, IntoElement, ParentElement, RenderImage, Styled};

use crate::example::TrayExample;
use crate::host;
use crate::icon_animations::{IconAnimation, Rgb, ALL as ALL_ANIMATIONS};
use crate::tray_controller::{Scene, StillIcon, TrayEntry};

/// Seconds on the wall clock — one clock for all tiles. They only preview
/// the look; the tray frames come from the selected icon's animator.
fn now_secs() -> f64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_secs_f64())
        .unwrap_or(0.)
}

/// The colors the Color row offers: Auto (the shell's tint) or a fixed one.
pub const COLORS: [(&str, Option<Rgb>); 4] = [
    ("Auto", None),
    ("Blue", Some([0x2F, 0x7D, 0xE1])),
    ("Amber", Some([0xF0, 0xA0, 0x20])),
    ("Red", Some([0xE5, 0x48, 0x4D])),
];

pub fn animate_tab(entry: &TrayEntry, this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let animator = &entry.animator;
    let playing = animator.animation;
    let time = now_secs();

    // The gallery: two rows of four, every tile drawn at the shared clock.
    let mut rows = div().flex().flex_col().gap(px(t.spacing_1_5));
    for row in 0..2 {
        let mut cells = div().flex().gap(px(t.spacing_1_5));
        for column in 0..4 {
            let animation = ALL_ANIMATIONS[row * 4 + column];
            let selected = playing == Some(animation);
            let ink = host::to_rgb(t.color_content);
            let image = tile_image(animation, time, ink, selected, cx);
            cells = cells.child(
                div().flex_1().min_w_0().child(
                    OptionCard::new(
                        ("animation-tile", row * 4 + column),
                        div()
                            .flex()
                            .flex_col()
                            .items_center()
                            .justify_center()
                            .gap(px(t.spacing_1))
                            .py(px(t.spacing_1))
                            .child(
                                div()
                                    .size(px(28.))
                                    .flex()
                                    .items_center()
                                    .justify_center()
                                    .child(gpui::img(image).size(px(28.))),
                            )
                            .child(div().child(animation.label())),
                    )
                    .selected(selected)
                    .on_click({
                        let this = this.clone();
                        move |_, _, cx| {
                            this.update(cx, |this, cx| {
                                this.play(animation, None, false, cx);
                            });
                        }
                    }),
                ),
            );
        }
        rows = rows.child(cells);
    }

    div()
        .flex()
        .flex_col()
        // Plain rows rather than a grid: a UI probe reads box offsets.
        .child(section("Animations", cx))
        .child(div().p(px(t.spacing_2_5)).child(rows))
        .child(host::option_row(
            "Icon presets",
            vec![
                scene_chip(entry, Scene::Download, 0, this).into_any_element(),
                scene_chip(entry, Scene::Recording, 1, this).into_any_element(),
                scene_chip(entry, Scene::Syncing, 2, this).into_any_element(),
                host::ActionChip::new("three-icons", "Three icons")
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.play_three_at_once(cx));
                        }))
                    })
                    .into_any_element(),
            ],
            cx,
        ))
        .child(dazzui::Divider::new())
        .child(section("Frames", cx))
        .child(host::option_row(
            "Still icon",
            StillIcon::ALL
                .iter()
                .enumerate()
                .map(|(index, &kind)| {
                    host::OptionChip::new(("still", index), kind.label())
                        .selected(entry.still == Some(kind))
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| {
                                    this.set_still(kind, None, false, cx);
                                });
                            }))
                        })
                        .into_any_element()
                })
                .collect(),
            cx,
        ))
        .child(host::option_row(
            "Rate",
            [10u32, 24, 30, 60]
                .iter()
                .enumerate()
                .map(|(index, &fps)| {
                    host::OptionChip::new(("fps", index), format!("{fps} fps"))
                        .selected(animator.fps == fps)
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.set_fps(fps, cx));
                            }))
                        })
                        .into_any_element()
                })
                .collect(),
            cx,
        ))
        .child(host::option_row(
            "Resolution",
            {
                let mut chips: Vec<AnyElement> = [1u32, 2, 3]
                    .iter()
                    .enumerate()
                    .map(|(index, &scale)| {
                        host::OptionChip::new(("scale", index), format!("{scale}x"))
                            .selected(animator.scale == scale)
                            .on_tap({
                                let this = this.clone();
                                Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| this.set_scale(scale, cx));
                                }))
                            })
                            .into_any_element()
                    })
                    .collect();
                chips.push(
                    host::hint(format!("{} px per side", animator.pixel_size()), cx)
                        .into_any_element(),
                );
                chips
            },
            cx,
        ))
        .child(host::option_row(
            "Color",
            {
                let mut chips: Vec<AnyElement> = COLORS
                    .iter()
                    .enumerate()
                    .map(|(index, (label, color))| {
                        host::OptionChip::new(("color", index), *label)
                            .selected(entry.selected_color == *color)
                            .on_tap({
                                let this = this.clone();
                                let color = *color;
                                Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| this.set_color(color, cx));
                                }))
                            })
                            .into_any_element()
                    })
                    .collect();
                chips.push(
                    host::hint(
                        if cfg!(target_os = "macos") {
                            "Auto: menu bar tint"
                        } else {
                            "Auto: system theme"
                        },
                        cx,
                    )
                    .into_any_element(),
                );
                chips
            },
            cx,
        ))
        .child(div().h(px(t.spacing_2_5)))
}

fn section(text: impl Into<gpui::SharedString>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    div()
        .flex_none()
        .pl(px(t.spacing_2_5))
        .pt(px(t.spacing_2_5))
        .child(host::section_label(text))
}

/// One of the three one-click scenes; pressing the current one resets it.
fn scene_chip(
    entry: &TrayEntry,
    scene: Scene,
    index: usize,
    this: &Entity<TrayExample>,
) -> impl IntoElement {
    host::OptionChip::new(("scene", index), scene.label())
        .selected(entry.scene == Some(scene))
        .on_tap({
            let this = this.clone();
            Some(Box::new(move |_, cx| {
                this.update(cx, |this, cx| {
                    if this.controller.selected_entry().map(|e| e.scene) == Some(Some(scene)) {
                        this.reset_scene(cx);
                    } else {
                        this.play_scene(scene, None, cx);
                    }
                });
            }))
        })
}

/// The tile's drawing: the animation rendered at `time` at the card's ink.
/// A selected tile is drawn the way the system draws any chosen card, so it
/// carries the primary tint's ink.
fn tile_image(
    animation: IconAnimation,
    time: f64,
    ink: Rgb,
    selected: bool,
    cx: &App,
) -> Arc<RenderImage> {
    let t = cx.theme();
    let drawn = if selected {
        // The tint's text grade, as a chosen card's ink.
        let tinted = t
            .control_color_tinted_content_normal
            .resolve(Tint::Primary.ramp(t));
        host::hsla_to_rgb(tinted)
    } else {
        ink
    };
    let mut pixmap = tiny_skia::Pixmap::new(56, 56).expect("tiles are tiny");
    animation.paint(&mut pixmap, time, drawn);
    let mut data = pixmap.data().to_vec();
    for pixel in data.as_chunks_mut::<4>().0 {
        pixel.swap(0, 2);
    }
    let buffer =
        image::RgbaImage::from_raw(56, 56, data).expect("tile buffer matches its dimensions");
    Arc::new(RenderImage::new(smallvec::smallvec![image::Frame::new(
        buffer
    )]))
}
