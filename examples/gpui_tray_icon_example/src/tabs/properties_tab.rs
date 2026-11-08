//! One row per TrayIcon API — the counterpart of the Flutter example's
//! `tabs/properties_tab.dart`. The block on top never shows what was
//! written — it shows what the native getters return afterwards.

use dazzui::theme::ActiveTheme;
use dazzui::{Badge, BadgeVariant, StyledExt, Tint};
use gpui::prelude::*;
use gpui::{div, px, App, Entity, IntoElement, ParentElement, SharedString, Styled};

use nativeapi::menu::{Menu, MenuBackend};
use nativeapi::tray_icon::ContextMenuTrigger;

use crate::example::{
    bounds_supported, open_menu_supported, title_supported, EditTarget, TrayExample,
};
use crate::host;
use crate::tray_controller::TrayEntry;

/// The titles the Title row offers.
const TITLES: [(&str, Option<&str>); 4] = [
    ("No title", None),
    ("42%", Some("42%")),
    ("00:12", Some("00:12")),
    ("Chinese greeting", Some("你好")),
];

/// The tooltips the Tooltip row offers.
const TOOLTIPS: [(&str, Option<&str>); 4] = [
    ("No tooltip", None),
    ("Short", Some("nativeapi tray icon")),
    (
        "Long",
        Some(
            "A long tooltip that says rather more than a tooltip usually should, \
             to see where the platform cuts it off",
        ),
    ),
    ("2 lines", Some("Line one\nLine two")),
];

const TRIGGERS: [(&str, ContextMenuTrigger); 4] = [
    ("Manual", ContextMenuTrigger::None),
    ("Left", ContextMenuTrigger::Clicked),
    ("Right", ContextMenuTrigger::RightClicked),
    ("Double", ContextMenuTrigger::DoubleClicked),
];

pub fn properties_tab(
    example: &TrayExample,
    entry: &TrayEntry,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    let t = cx.theme();
    let icon = &entry.tray_icon;
    let title = icon.get_title();
    let tooltip = icon.get_tooltip();
    let visible = icon.is_visible();
    let trigger = icon.get_context_menu_trigger();

    div()
        .flex()
        .flex_col()
        .child(state_block(example, entry, this, cx))
        .child(host::option_row(
            "Title",
            {
                let mut chips: Vec<gpui::AnyElement> = TITLES
                    .iter()
                    .enumerate()
                    .map(|(index, (label, value))| {
                        let enabled = !entry.is_sign && title_supported();
                        let value = *value;
                        host::OptionChip::new(("title", index), *label)
                            .selected(title.as_deref() == value)
                            .when(enabled, |chip| {
                                let this = this.clone();
                                chip.on_tap(Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| {
                                        this.set_title(value, cx);
                                    });
                                })))
                            })
                            .into_any_element()
                    })
                    .collect();
                if entry.is_sign {
                    chips.push(host::hint("Signs use their own content", cx).into_any_element());
                } else if title_supported() {
                    chips.push(
                        host::ActionChip::new("title-edit", "Title…")
                            .on_tap({
                                let this = this.clone();
                                Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| {
                                        this.open_edit(EditTarget::Title, cx)
                                    });
                                }))
                            })
                            .into_any_element(),
                    );
                } else {
                    chips.push(host::hint("no titles on Windows", cx).into_any_element());
                }
                chips
            },
            cx,
        ))
        .child(host::option_row(
            "Tooltip",
            {
                let mut chips: Vec<gpui::AnyElement> = TOOLTIPS
                    .iter()
                    .enumerate()
                    .map(|(index, (label, value))| {
                        host::OptionChip::new(("tooltip", index), *label)
                            .selected(tooltip.as_deref() == *value)
                            .on_tap({
                                let this = this.clone();
                                let value = *value;
                                Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| {
                                        this.set_tooltip(value, cx);
                                    });
                                }))
                            })
                            .into_any_element()
                    })
                    .collect();
                chips.push(
                    host::ActionChip::new("tooltip-edit", "Tooltip…")
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.open_edit(EditTarget::Tooltip, cx));
                            }))
                        })
                        .into_any_element(),
                );
                chips
            },
            cx,
        ))
        .child(host::option_row(
            "Visible",
            vec![
                host::OptionChip::new("visible-shown", "Shown")
                    .selected(visible)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_visible(true, cx));
                        }))
                    })
                    .into_any_element(),
                host::OptionChip::new("visible-hidden", "Hidden")
                    .selected(!visible)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_visible(false, cx));
                        }))
                    })
                    .into_any_element(),
            ],
            cx,
        ))
        .child(host::option_row(
            "Trigger",
            TRIGGERS
                .iter()
                .enumerate()
                .map(|(index, (label, value))| {
                    host::OptionChip::new(("trigger", index), *label)
                        .selected(trigger == *value)
                        .on_tap({
                            let this = this.clone();
                            let value = *value;
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.set_trigger(value, cx));
                            }))
                        })
                        .into_any_element()
                })
                .collect(),
            cx,
        ))
        .child(host::option_row(
            "Popup",
            {
                let popup_mode = example.controller.popup_mode;
                vec![
                    host::OptionChip::new("popup-off", "Off")
                        .selected(!popup_mode)
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.set_popup_mode(false, cx));
                            }))
                        })
                        .into_any_element(),
                    host::OptionChip::new("popup-on", "Click shows, blur hides")
                        .selected(popup_mode)
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.set_popup_mode(true, cx));
                            }))
                        })
                        .into_any_element(),
                ]
            },
            cx,
        ))
        .child(host::option_row(
            "Menu",
            if open_menu_supported() {
                vec![
                    host::ActionChip::new("open-menu", "Open menu")
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.open_menu(None, cx));
                            }))
                        })
                        .into_any_element(),
                    host::ActionChip::new("open-close-menu", "Open, close in 2 s")
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| {
                                    this.open_menu(Some(std::time::Duration::from_secs(2)), cx);
                                });
                            }))
                        })
                        .into_any_element(),
                ]
            } else {
                vec![host::hint("only the shell opens it on Linux", cx).into_any_element()]
            },
            cx,
        ))
        // Only Windows has a second menu backend to choose.
        .when(Menu::is_backend_supported(MenuBackend::WinUi3), |tab| {
            tab.child(host::option_row(
                "Backend",
                [MenuBackend::Native, MenuBackend::WinUi3]
                    .iter()
                    .enumerate()
                    .map(|(index, &backend)| {
                        host::OptionChip::new(
                            ("backend", index),
                            if backend == MenuBackend::WinUi3 {
                                "WinUI 3"
                            } else {
                                "Native"
                            },
                        )
                        .selected(example.controller.menu_backend == backend)
                        .when(Menu::is_backend_supported(backend), |chip| {
                            let this = this.clone();
                            chip.on_tap(Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.set_menu_backend(backend, cx));
                            })))
                        })
                        .into_any_element()
                    })
                    .collect(),
                cx,
            ))
        })
        .child(div().h(px(t.spacing_2_5)))
}

/// The read-back block: everything the native getters say about the
/// selected icon, plus the click counters.
fn state_block(
    example: &TrayExample,
    entry: &TrayEntry,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    let t = cx.theme();
    let icon = &entry.tray_icon;
    let bounds = icon.get_bounds();
    let controller = &example.controller;
    let line = |text: String| {
        div()
            .type_style(t.body_medium)
            .text_color(t.color_content.hsla())
            .overflow_hidden()
            .whitespace_nowrap()
            .text_ellipsis()
            .child(SharedString::from(text))
    };
    let counter = |label: &str, count: u32| {
        Badge::new()
            .size(dazzui::Size::Small)
            .variant(if count > 0 {
                BadgeVariant::Tinted
            } else {
                BadgeVariant::Outlined
            })
            .tint(if count > 0 {
                Tint::Primary
            } else {
                Tint::Neutral
            })
            .child(SharedString::from(format!("{label} {count}")))
    };

    let mut content_view = String::new();
    if let Some(sign) = &entry.sign {
        content_view = format!(
            "contentView {}",
            match sign.style {
                crate::signs::sign_style::SignStyle::Missing => "missing",
                crate::signs::sign_style::SignStyle::Welcome => "welcome",
                crate::signs::sign_style::SignStyle::Travel => "travel",
                crate::signs::sign_style::SignStyle::Guide => "guide",
            }
        );
    }

    div()
        .flex()
        .flex_col()
        .flex_none()
        .border_b(px(t.stroke_hairline))
        .border_color(t.color_border.hsla())
        .child(
            div()
                .flex()
                .flex_col()
                .gap(px(t.spacing_0_5))
                .px(px(t.spacing_2_5))
                .py(px(t.spacing_2))
                .child(
                    div()
                        .flex()
                        .items_center()
                        .gap(px(t.spacing_1))
                        .child(
                            div()
                                .flex_1()
                                .child(host::section_label("Read back from getters")),
                        )
                        .child(
                            host::ActionChip::new("window-to-icon", "Window to icon").when(
                                bounds_supported(),
                                |chip| {
                                    let this = this.clone();
                                    chip.on_tap(Some(Box::new(move |_, cx| {
                                        this.update(cx, |this, cx| this.move_window_to_icon(cx));
                                    })))
                                },
                            ),
                        )
                        .child(host::ActionChip::new("refresh", "Refresh").on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.refresh(cx));
                            }))
                        })),
                )
                .child(line(format!(
                    "id {} · visible {} · trigger {}",
                    icon.get_id(),
                    icon.is_visible(),
                    trigger_name(icon.get_context_menu_trigger()),
                )))
                .when(!content_view.is_empty(), |block| {
                    block.child(line(content_view))
                })
                .child(line(format!("title {}", quote(&icon.get_title()))))
                .child(line(format!("tooltip {}", quote(&icon.get_tooltip()))))
                .child(line(format!(
                    "bounds {},{} {}×{} · supported {} · getAll {}",
                    bounds.x.round(),
                    bounds.y.round(),
                    bounds.width.round(),
                    bounds.height.round(),
                    controller.supported,
                    controller.manager_count,
                )))
                .child(
                    div()
                        .flex()
                        .flex_wrap()
                        .items_center()
                        .gap(px(t.spacing_1))
                        .mt(px(t.spacing_1_5))
                        .child(counter("Left", entry.clicks))
                        .child(counter("Right", entry.right_clicks))
                        .child(counter("Double", entry.double_clicks))
                        .child(
                            host::ActionChip::new("reset-counts", "Reset counts").on_tap({
                                let this = this.clone();
                                Some(Box::new(move |_, cx| {
                                    this.update(cx, |this, cx| this.reset_counters(cx));
                                }))
                            }),
                        ),
                ),
        )
}

fn trigger_name(trigger: ContextMenuTrigger) -> &'static str {
    match trigger {
        ContextMenuTrigger::None => "none",
        ContextMenuTrigger::Clicked => "clicked",
        ContextMenuTrigger::RightClicked => "rightClicked",
        ContextMenuTrigger::DoubleClicked => "doubleClicked",
    }
}

fn quote(value: &Option<String>) -> String {
    match value {
        None => "null".to_string(),
        Some(value) => format!("\"{}\"", value.replace('\n', "\\n")),
    }
}
