//! The sign's own settings: style, content fields, display options — the
//! counterpart of the Flutter example's `tabs/sign_tab.dart`.

use dazzui::theme::ActiveTheme;
use dazzui::{OptionCard, StyledExt, Switch, TextField};
use gpui::{div, px, App, Entity, IntoElement, ParentElement, SharedString, Styled};

use crate::example::TrayExample;
use crate::host;
use crate::signs::sign_art::SignArt;
use crate::signs::sign_style::{SignStyle, ALL as ALL_STYLES};
use crate::tray_controller::TrayEntry;

pub fn sign_tab(
    example: &TrayExample,
    entry: &TrayEntry,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    let t = cx.theme();
    let Some(sign) = &entry.sign else {
        return div();
    };
    let style = sign.style;
    let number = entry.number;
    let fields = style.fields();
    let draft: Vec<String> = example
        .sign_draft
        .iter()
        .take(fields.len())
        .cloned()
        .collect();
    let error = example.sign_error.clone().or_else(|| sign.error.clone());

    // The style cards, each with its drawn thumbnail.
    let mut cards = div().flex().gap(px(t.spacing_2));
    for (index, &card_style) in ALL_STYLES.iter().enumerate() {
        cards = cards.child(
            div().flex_1().min_w_0().child(
                OptionCard::new(
                    ("sign-style", index),
                    div()
                        .flex()
                        .flex_col()
                        .items_center()
                        .justify_center()
                        .gap(px(t.spacing_1))
                        .py(px(t.spacing_1))
                        .child(SignArt::new(card_style).height(24.))
                        .child(
                            div()
                                .type_style(t.caption_small)
                                .text_color(t.color_content_muted.hsla())
                                .child(card_style.short_label()),
                        ),
                )
                .selected(style == card_style)
                .on_click({
                    let this = this.clone();
                    move |_, _, cx| {
                        this.update(cx, |this, cx| {
                            this.select_sign_style(index, cx);
                        });
                    }
                }),
            ),
        );
    }

    let mut tab = div()
        .flex()
        .flex_col()
        .p(px(t.spacing_4))
        .gap(px(t.spacing_2))
        .child(host::section_label("Sign style"))
        .child(cards)
        .child(div().h(px(t.spacing_1)))
        .child(host::section_label("Sign content"));
    for (index, field) in fields.iter().enumerate() {
        let value = draft.get(index).cloned().unwrap_or_default();
        tab = tab
            .child(
                div()
                    .type_style(t.label_small)
                    .text_color(host::muted_color(cx))
                    .child(*field),
            )
            .child(
                TextField::new(SharedString::from(format!("sign-field-{index}")))
                    .value(SharedString::from(value))
                    .on_change({
                        let this = this.clone();
                        move |text, _, cx| {
                            this.update(cx, |this, _cx| {
                                this.edit_sign_field(index, text.to_string());
                            });
                        }
                    })
                    .on_submit({
                        let this = this.clone();
                        move |_, _, cx| {
                            this.update(cx, |this, cx| this.apply_sign(cx));
                        }
                    }),
            )
            .child(div().h(px(t.spacing_1)));
    }
    if let Some(error) = error {
        tab = tab.child(
            div()
                .type_style(t.body_small)
                .text_color(t.color_danger.shade(600).hsla())
                .child(SharedString::from(error)),
        );
    }
    tab = tab.child(
        div()
            .flex()
            .gap(px(t.spacing_2))
            .child(
                host::ActionChip::new("apply-sign", "Apply sign")
                    .primary(true)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.apply_sign(cx));
                        }))
                    }),
            )
            .child(
                host::ActionChip::new("reset-sign", "Reset example").on_tap({
                    let this = this.clone();
                    Some(Box::new(move |_, cx| {
                        this.update(cx, |this, cx| this.reset_sign(cx));
                    }))
                }),
            ),
    );

    // Display options.
    let mut options = vec![];
    if style == SignStyle::Missing {
        options.push(host::option_row(
            "Sign color",
            vec![
                host::OptionChip::new("sign-blue", "Blue")
                    .selected(!sign.green)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_sign_green(false, cx));
                        }))
                    })
                    .into_any_element(),
                host::OptionChip::new("sign-green", "Green")
                    .selected(sign.green)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_sign_green(true, cx));
                        }))
                    })
                    .into_any_element(),
            ],
            cx,
        ));
    }
    if style.has_direction() {
        let (label, left, right) = if style == SignStyle::Missing {
            ("Direction", "W → E", "E → W")
        } else {
            ("Upper arrow", "Left", "Right")
        };
        options.push(host::option_row(
            label,
            vec![
                host::OptionChip::new("sign-left", left)
                    .selected(!sign.right)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_sign_right(false, cx));
                        }))
                    })
                    .into_any_element(),
                host::OptionChip::new("sign-right", right)
                    .selected(sign.right)
                    .on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| this.set_sign_right(true, cx));
                        }))
                    })
                    .into_any_element(),
            ],
            cx,
        ));
    }
    if style.has_subtitle_toggle() {
        options.push(
            div()
                .flex()
                .items_center()
                .gap(px(t.spacing_1_5))
                .px(px(t.spacing_2_5))
                .py(px(t.spacing_1_5))
                .child(
                    div()
                        .flex_1()
                        .type_style(t.label_small)
                        .text_color(host::muted_color(cx))
                        .child("English"),
                )
                .child(
                    Switch::new(SharedString::from(format!("sign-english-{number}")))
                        .checked(sign.english)
                        .on_checked_change({
                            let this = this.clone();
                            move |checked, _, cx| {
                                this.update(cx, |this, cx| this.set_sign_english(checked, cx));
                            }
                        }),
                )
                .child(host::hint("Show English subtitle", cx)),
        );
    }
    options.push(
        div()
            .flex()
            .px(px(t.spacing_2_5))
            .py(px(t.spacing_1_5))
            .child(
                host::ActionChip::new("enlarge-preview", "Enlarge preview").on_tap({
                    let this = this.clone();
                    Some(Box::new(move |_, cx| {
                        this.update(cx, |this, cx| this.enlarge_preview(number, cx));
                    }))
                }),
            ),
    );
    for option in options {
        tab = tab.child(option);
    }
    tab.child(div().h(px(t.spacing_2_5)))
}
