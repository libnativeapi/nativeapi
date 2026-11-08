//! What this example shares with the other GPUI examples built on DazzUI —
//! the counterpart of the Flutter examples' `dazzui_host` package.
//!
//! [`follow_appearance`] is the `Host`'s theme: Studio Light or Studio Dark by
//! the window's appearance, re-set whenever it changes. The widgets are the
//! few every example needs beyond the kit itself, with the same shapes and
//! the same doc comments as their Flutter counterparts in
//! `flutter_dazzui_host/lib/src/chips.dart` and `styles.dart`.

use dazzui::theme::ActiveTheme;
use dazzui::tokens::{Face, TypeStyle};
use dazzui::{Button, SectionLabel, Size, StyledExt, Tint, Toggle, Variant};
use gpui::prelude::*;
use gpui::{
    div, px, AnyElement, App, Div, ElementId, Hsla, ParentElement, RenderOnce, SharedString,
    Styled, Window, WindowAppearance,
};

/// Sets the theme the appearance asks for: Studio Light or Studio Dark, as
/// the Flutter host does by platform brightness. Call it once before the
/// first draw; [`crate::example::TrayExample`] observes later changes.
pub fn follow_appearance(window: &mut Window, cx: &mut App) {
    dazzui::Theme::set(theme_name(window.appearance()), cx);
}

pub fn theme_name(appearance: WindowAppearance) -> &'static str {
    match appearance {
        WindowAppearance::Dark => "studio-dark",
        _ => "studio-light",
    }
}

/// A muted caption for row labels and hints (`vars.muted` in the Flutter
/// examples).
pub fn muted_style(cx: &App) -> TypeStyle {
    cx.theme().label_small
}

/// A monospace line for read-back values and the log (`vars.mono`).
pub fn mono_style(cx: &App) -> TypeStyle {
    cx.theme().caption_small.in_face(Face::Mono)
}

pub fn muted_color(cx: &App) -> Hsla {
    cx.theme().color_content_muted.hsla()
}

/// The theme's ink as the RGB triple the tray drawings take.
pub fn to_rgb(color: dazzui::tokens::Color) -> crate::icon_animations::Rgb {
    let value = color.0;
    [
        ((value >> 24) & 0xFF) as u8,
        ((value >> 16) & 0xFF) as u8,
        ((value >> 8) & 0xFF) as u8,
    ]
}

/// Any GPUI colour as the RGB triple the tray drawings take.
pub fn hsla_to_rgb(color: Hsla) -> crate::icon_animations::Rgb {
    let rgba = gpui::Rgba::from(color);
    [
        (rgba.r * 255.) as u8,
        (rgba.g * 255.) as u8,
        (rgba.b * 255.) as u8,
    ]
}

/// What a chip does when pressed. A boxed trait object (rather than
/// `impl Fn`) so `Option<Tap>` keeps its higher-ranked lifetimes at the call
/// sites.
pub type Tap = Box<dyn Fn(&mut Window, &mut App) + 'static>;

/// A small mouse-only choice: every setting in the example is one of these,
/// so a GUI test or a demo script never needs the keyboard. Labels are plain
/// text (no icon font), which also lets a UI probe find them by name.
///
/// It is a DazzUI `Toggle`: paper at rest, the tint's wash when it is the
/// current value.
#[derive(IntoElement)]
pub struct OptionChip {
    id: ElementId,
    label: SharedString,
    selected: bool,
    tint: Tint,
    on_tap: Option<Tap>,
}

impl OptionChip {
    pub fn new(id: impl Into<ElementId>, label: impl Into<SharedString>) -> Self {
        OptionChip {
            id: id.into(),
            label: label.into(),
            selected: false,
            tint: Tint::Primary,
            on_tap: None,
        }
    }

    pub fn selected(mut self, selected: bool) -> Self {
        self.selected = selected;
        self
    }

    pub fn tint(mut self, tint: Tint) -> Self {
        self.tint = tint;
        self
    }

    /// `None` disables the chip.
    pub fn on_tap(mut self, on_tap: Option<Tap>) -> Self {
        self.on_tap = on_tap;
        self
    }
}

impl RenderOnce for OptionChip {
    fn render(self, _: &mut Window, _: &mut App) -> impl IntoElement {
        // A chip is picked, never unpicked: pressing the current value again
        // just sets it again.
        let on_tap = self.on_tap;
        Toggle::new(self.id)
            .size(Size::Tiny)
            .variant(Variant::Normal)
            .tint(self.tint)
            .pressed(self.selected)
            .disabled(on_tap.is_none())
            .when_some(on_tap, |this, on_tap| {
                this.on_pressed_change(move |_, window, cx| on_tap(window, cx))
            })
            .child(self.label)
    }
}

/// A chip that does something rather than picks something — the same box as
/// [`OptionChip`], as a DazzUI `Button`, so a row of both reads as one row.
#[derive(IntoElement)]
pub struct ActionChip {
    id: ElementId,
    label: SharedString,
    primary: bool,
    on_tap: Option<Tap>,
}

impl ActionChip {
    pub fn new(id: impl Into<ElementId>, label: impl Into<SharedString>) -> Self {
        ActionChip {
            id: id.into(),
            label: label.into(),
            primary: false,
            on_tap: None,
        }
    }

    /// The one action a dialog or a row leads with.
    pub fn primary(mut self, primary: bool) -> Self {
        self.primary = primary;
        self
    }

    /// `None` disables the chip.
    pub fn on_tap(mut self, on_tap: Option<Tap>) -> Self {
        self.on_tap = on_tap;
        self
    }
}

impl RenderOnce for ActionChip {
    fn render(self, _: &mut Window, _: &mut App) -> impl IntoElement {
        let on_tap = self.on_tap;
        Button::new(self.id)
            .size(Size::Tiny)
            .variant(if self.primary {
                Variant::Filled
            } else {
                Variant::Normal
            })
            .tint(if self.primary {
                Tint::Primary
            } else {
                Tint::Neutral
            })
            .disabled(on_tap.is_none())
            .when_some(on_tap, |this, on_tap| {
                this.on_click(move |_, window, cx| on_tap(window, cx))
            })
            .child(self.label)
    }
}

/// One labelled row of chips, closed by a hairline.
pub fn option_row(label: &'static str, children: Vec<AnyElement>, cx: &App) -> Div {
    let t = cx.theme();
    div()
        .flex()
        .flex_none()
        .flex_col()
        .border_b(px(t.stroke_hairline))
        .border_color(t.color_border.hsla())
        .child(
            div()
                .flex()
                .items_center()
                .px(px(t.spacing_2_5))
                .py(px(t.spacing_1_5))
                .child(
                    div()
                        .flex_none()
                        .w(px(66.))
                        .type_style(muted_style(cx))
                        .text_color(muted_color(cx))
                        .child(label),
                )
                .child(
                    div()
                        .flex_1()
                        .flex()
                        .flex_wrap()
                        .items_center()
                        .gap(px(t.spacing_1))
                        .children(children),
                ),
        )
}

/// Muted remark placed after the chips of a row.
pub fn hint(text: impl Into<SharedString>, cx: &App) -> Div {
    div()
        .type_style(muted_style(cx))
        .text_color(muted_color(cx))
        .child(text.into())
}

/// The micro-heading, with the example's muted row-label colouring where a
/// pane section needs one.
pub fn section_label(text: impl Into<SharedString>) -> SectionLabel {
    SectionLabel::new().child(div().child(text.into()))
}
