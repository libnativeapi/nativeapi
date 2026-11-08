//! Acceptance in one screen: what ticked itself, what still needs a look —
//! the counterpart of the Flutter example's `tabs/checklist_tab.dart`.

use dazzui::theme::ActiveTheme;
use dazzui::{Badge, BadgeVariant, StyledExt, Tint};
use gpui::prelude::*;
use gpui::{div, px, App, Entity, IntoElement, ParentElement, SharedString, Styled};

use crate::checklist::{CheckItem, CheckStatus};
use crate::example::TrayExample;
use crate::host;
use crate::tray_controller::TrayController;

pub fn checklist_tab(
    controller: &TrayController,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    let t = cx.theme();
    let auto: Vec<&CheckItem> = controller
        .checklist
        .items
        .iter()
        .filter(|item| !item.manual)
        .collect();
    let manual: Vec<&CheckItem> = controller
        .checklist
        .items
        .iter()
        .filter(|item| item.manual)
        .collect();

    div()
        .flex()
        .flex_col()
        .flex_1()
        .min_h_0()
        .child(
            div()
                .id("checklist-rows")
                .flex_1()
                .min_h_0()
                .overflow_y_scroll()
                .flex_col()
                .child(header(
                    "Auto · ticks itself from events and return values",
                    cx,
                ))
                .children(auto.iter().map(|item| row(item, this, cx)))
                .child(div().mt(px(t.spacing_1_5)).child(dazzui::Divider::new()))
                .child(header("Manual · look at the tray, then mark it", cx))
                .children(manual.iter().map(|item| row(item, this, cx)))
                .child(div().h(px(t.spacing_1_5))),
        )
        .child(dazzui::Divider::new())
        .child(
            div()
                .flex()
                .items_center()
                .gap(px(t.spacing_1))
                .px(px(t.spacing_2_5))
                .py(px(t.spacing_1_5))
                .child(
                    div()
                        .flex_1()
                        .type_style(host::mono_style(cx))
                        .text_color(host::muted_color(cx))
                        .child(SharedString::from(controller.checklist.summary())),
                )
                .child(host::ActionChip::new("copy-report", "Copy report").on_tap({
                    let report = controller.checklist.report();
                    Some(Box::new(move |_, cx| {
                        cx.write_to_clipboard(gpui::ClipboardItem::new_string(report.clone()));
                    }))
                }))
                .child(
                    host::ActionChip::new("reset-checklist", "Reset checklist").on_tap({
                        let this = this.clone();
                        Some(Box::new(move |_, cx| {
                            this.update(cx, |this, cx| {
                                this.controller.checklist.reset();
                                cx.notify();
                            });
                        }))
                    }),
                ),
        )
}

fn header(text: impl Into<gpui::SharedString>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    div()
        .flex_none()
        .px(px(t.spacing_2_5))
        .pt(px(t.spacing_2))
        .pb(px(t.spacing_1))
        .child(host::section_label(text))
}

fn row(item: &CheckItem, this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let detail = [
        item.note,
        (!item.detail.is_empty()).then_some(item.detail.as_str()),
    ]
    .into_iter()
    .flatten()
    .collect::<Vec<_>>()
    .join(" · ");
    let (tint, variant) = match item.status {
        CheckStatus::Pass => (Tint::Success, BadgeVariant::Tinted),
        CheckStatus::Fail => (Tint::Danger, BadgeVariant::Tinted),
        CheckStatus::Open => (Tint::Neutral, BadgeVariant::Outlined),
    };
    let mut line = div()
        .flex()
        .items_center()
        .px(px(t.spacing_2_5))
        .py(px(t.spacing_0_5 + 1.))
        .child(
            div().w(px(44.)).child(
                Badge::new()
                    .size(dazzui::Size::Small)
                    .tint(tint)
                    .variant(variant)
                    .child(SharedString::from(item.status.name())),
            ),
        )
        .child(
            div()
                .flex_1()
                .min_w_0()
                .overflow_hidden()
                .whitespace_nowrap()
                .text_ellipsis()
                .type_style(t.body_small)
                .child(item.label),
        );
    if !detail.is_empty() {
        line = line.child(
            div()
                .pl(px(t.spacing_1_5))
                .type_style(host::mono_style(cx))
                .text_color(if item.status == CheckStatus::Fail {
                    t.color_danger.shade(600).hsla()
                } else {
                    host::muted_color(cx)
                })
                .child(SharedString::from(detail)),
        );
    }
    if item.manual {
        line = line
            .child(div().w(px(t.spacing_1_5)).flex_none())
            .child(
                host::OptionChip::new(
                    SharedString::from(format!("check-pass-{}", item.id)),
                    "Pass",
                )
                .tint(Tint::Success)
                .selected(item.status == CheckStatus::Pass)
                .on_tap({
                    let this = this.clone();
                    let id = item.id;
                    Some(Box::new(move |_, cx| {
                        this.update(cx, |this, cx| {
                            this.controller.checklist.mark(id, CheckStatus::Pass);
                            cx.notify();
                        });
                    }))
                }),
            )
            .child(div().w(px(t.spacing_1)).flex_none())
            .child(
                host::OptionChip::new(
                    SharedString::from(format!("check-fail-{}", item.id)),
                    "Fail",
                )
                .tint(Tint::Danger)
                .selected(item.status == CheckStatus::Fail)
                .on_tap({
                    let this = this.clone();
                    let id = item.id;
                    Some(Box::new(move |_, cx| {
                        this.update(cx, |this, cx| {
                            this.controller.checklist.mark(id, CheckStatus::Fail);
                            cx.notify();
                        });
                    }))
                }),
            );
    }
    line
}
