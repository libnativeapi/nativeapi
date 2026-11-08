//! The Add menu at the foot of the sidebar — the counterpart of the Flutter
//! example's `widgets/add_tray_menu.dart`.

use dazzui::theme::ActiveTheme;
use dazzui::{Button, Popover, PopoverClose};
use gpui::{div, px, App, Entity, IntoElement, ParentElement, SharedString, Styled, Window};

use crate::example::TrayExample;
use crate::host;
use crate::signs::sign_style::ALL as ALL_STYLES;
use crate::signs::SIGNS_SUPPORTED;
use crate::tray_controller::Scene;

/// One menu button: it closes the popover, then does what it says and shows
/// the new item's content.
fn item(
    id: gpui::ElementId,
    label: SharedString,
    this: &Entity<TrayExample>,
    enabled: bool,
    action: impl Fn(&mut TrayExample, &mut gpui::Context<TrayExample>) + 'static,
) -> impl IntoElement {
    let this = this.clone();
    PopoverClose::new(move |close, _window: &mut Window, _cx| {
        Button::new(id)
            .size(dazzui::Size::Small)
            .variant(dazzui::Variant::Normal)
            .full_width(true)
            .disabled(!enabled)
            .on_click(move |_, window, cx| {
                close(window, cx);
                this.update(cx, |example, cx| {
                    action(example, cx);
                    example.tab = crate::example::Tab::Content;
                    cx.notify();
                });
            })
            .child(label)
            .into_any_element()
    })
}

pub fn add_tray_menu(this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let mut children: Vec<gpui::AnyElement> = vec![
        div().child(host::section_label("Icons")).into_any_element(),
        item(
            "add-default".into(),
            SharedString::from("Default icon"),
            this,
            true,
            |example, cx| {
                example.add_icon(None, cx);
            },
        )
        .into_any_element(),
        item(
            "add-download".into(),
            SharedString::from("Download"),
            this,
            true,
            |example, cx| {
                example.add_scene(Scene::Download, cx);
            },
        )
        .into_any_element(),
        item(
            "add-recording".into(),
            SharedString::from("Recording"),
            this,
            true,
            |example, cx| {
                example.add_scene(Scene::Recording, cx);
            },
        )
        .into_any_element(),
        item(
            "add-syncing".into(),
            SharedString::from("Syncing"),
            this,
            true,
            |example, cx| {
                example.add_scene(Scene::Syncing, cx);
            },
        )
        .into_any_element(),
        item(
            "add-three".into(),
            SharedString::from("Three icons"),
            this,
            true,
            |example, cx| {
                example.add_three_icons(cx);
            },
        )
        .into_any_element(),
        div().h(px(t.spacing_2)).into_any_element(),
        div().child(host::section_label("Signs")).into_any_element(),
    ];
    for (index, &style) in ALL_STYLES.iter().enumerate() {
        let supported = SIGNS_SUPPORTED;
        children.push(
            item(
                ("add-sign", index).into(),
                SharedString::from(style.label()),
                this,
                supported,
                move |example, cx| {
                    example.add_sign(style, cx);
                },
            )
            .into_any_element(),
        );
    }
    if !SIGNS_SUPPORTED {
        children
            .push(host::hint("Custom tray signs are available on macOS.", cx).into_any_element());
    }

    Popover::new("add-tray")
        .title("Add tray item")
        .side(dazzui::Side::Top)
        .align(dazzui::Align::Start)
        .width(dazzui::Length::Px(260.))
        .trigger(move |press, _open, _window, _cx| {
            Button::new("add-trigger")
                .size(dazzui::Size::Small)
                .variant(dazzui::Variant::Normal)
                .full_width(true)
                .on_click(move |_, window, cx| press(window, cx))
                .child("+ Add")
                .into_any_element()
        })
        .child(
            div()
                .flex()
                .flex_col()
                .gap(px(t.spacing_1))
                .children(children),
        )
}
