//! The page: the sidebar, the tabs, the live preview, the event footer and
//! the text-edit dialog — the counterpart of the Flutter example's
//! `main.dart`. The tab bodies live in `tabs/`, the preview and the Add menu
//! in `widgets/`.

use futures::channel::mpsc;
use gpui::prelude::*;
use gpui::{
    div, px, App, Context, Entity, IntoElement, ParentElement, Render, SharedString, Styled, Window,
};

use dazzui::theme::ActiveTheme;
use dazzui::{
    Divider, Icon, IconButton, Length, Popover, SegmentedControl, SegmentedItem,
    Size as WidgetSize, StyledExt,
};

use nativeapi::window::{Window as NativeWindow, WindowEvent};
use nativeapi_gpui::observe_window_events;

use crate::host;
use crate::signs::sign_art::SignArt;
use crate::tabs;
use crate::tray_controller::{TrayController, TrayMsg};
use crate::widgets;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum Tab {
    Content,
    Properties,
    Checklist,
}

impl Tab {
    fn value(self) -> &'static str {
        match self {
            Tab::Content => "content",
            Tab::Properties => "properties",
            Tab::Checklist => "checklist",
        }
    }
}

/// What the Title… / Tooltip… dialogs edit.
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum EditTarget {
    Title,
    Tooltip,
}

#[derive(Clone)]
pub struct EditRequest {
    pub target: EditTarget,
    pub title: &'static str,
    pub value: String,
}

/// The example: the tray controller's state, the UI state around it, and the
/// channel every native listener reports through.
pub struct TrayExample {
    pub controller: TrayController,
    pub tab: Tab,
    /// This example's own window's id; `None` where nativeapi cannot reach
    /// GPUI's windows (Linux). A fresh handle is fetched by id whenever the
    /// window is asked for — handles are single-ownership.
    pub(crate) own_window_id: Option<nativeapi::window::WindowId>,
    pub(crate) msg_tx: mpsc::UnboundedSender<TrayMsg>,
    /// The open Title… / Tooltip… dialog, with its live text.
    pub edit: Option<EditRequest>,
    /// The Sign tab's text fields, held until Apply commits them, and the
    /// style/entry they were loaded for.
    pub sign_draft: Vec<String>,
    pub sign_draft_for: Option<(u32, crate::signs::sign_style::SignStyle)>,
    pub sign_error: Option<String>,
}

impl TrayExample {
    pub fn new(
        own_window: Option<NativeWindow>,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) -> Self {
        let own_window_id = own_window.as_ref().map(|window| window.id());
        // The theme follows the window's appearance, as the Flutter host
        // follows the platform's brightness.
        let appearance = window.appearance();
        dazzui::Theme::set(host::theme_name(appearance), cx);
        cx.observe_window_appearance(window, |_, window, cx| {
            let appearance = window.appearance();
            let name = host::theme_name(appearance);
            dazzui::Theme::set(name, cx);
        })
        .detach();

        // nativeapi calls its listeners on the platform thread; every tray
        // and menu event is queued and applied from this task.
        let (tx, mut rx) = mpsc::unbounded::<TrayMsg>();
        cx.spawn(async move |this, cx| {
            use futures::StreamExt;
            while let Some(msg) = rx.next().await {
                if this.update(cx, |this, cx| this.on_msg(msg, cx)).is_err() {
                    break;
                }
            }
        })
        .detach();

        // Window events (popup blur-and-hide, close-requested cancellation)
        // and the 250 ms tick that drives scenes, watches menu closes and
        // settles the signs' tray height.
        observe_window_events(cx, |this, event: WindowEvent, cx| {
            this.on_window_event(event, cx)
        })
        .detach();
        cx.spawn(async move |this, cx| loop {
            cx.background_executor()
                .timer(std::time::Duration::from_millis(250))
                .await;
            if this.update(cx, |this, cx| this.on_tick(cx)).is_err() {
                break;
            }
        })
        .detach();

        // The gallery's tiles animate on their own clock — the Flutter
        // example's Ticker keeps them moving even while the tray itself is
        // still. One redraw nudge per frame, only while the gallery is up.
        cx.spawn(async move |this, cx| loop {
            cx.background_executor()
                .timer(std::time::Duration::from_millis(33))
                .await;
            let _ = this.update(cx, |this, cx| {
                let gallery = this.tab == Tab::Content
                    && this
                        .controller
                        .selected_entry()
                        .is_some_and(|entry| entry.sign.is_none());
                // While the selected icon animates, its frames already
                // redraw the window; this nudge is for the still moments.
                let animating = this
                    .controller
                    .selected_entry()
                    .is_some_and(|entry| entry.animator.is_playing());
                if gallery && !animating {
                    cx.notify();
                }
            });
        })
        .detach();

        let mut controller = TrayController {
            checklist: crate::checklist::Checklist::new(),
            log: Vec::new(),
            supported: nativeapi::tray_manager::TrayManager::is_supported(),
            manager_count: 0,
            last_event: "No events yet".to_string(),
            selected: None,
            menu_backend: if nativeapi::menu::Menu::is_backend_supported(
                nativeapi::menu::MenuBackend::WinUi3,
            ) {
                nativeapi::menu::MenuBackend::WinUi3
            } else {
                nativeapi::menu::MenuBackend::Native
            },
            popup_mode: std::env::var("TRAY_POPUP").as_deref() == Ok("1"),
            entries: Vec::new(),
            next_number: 1,
            tick: 0,
            popup_hidden_at: std::time::Instant::now() - std::time::Duration::from_secs(3600),
            last_tray_event: None,
            last_tray_event_at: None,
            last_open_call_at: None,
            close_watch: None,
        };
        if controller.supported {
            controller
                .checklist
                .pass(crate::checklist::SUPPORTED, "true");
        } else {
            controller
                .checklist
                .fail(crate::checklist::SUPPORTED, "false");
        }
        let mut example = TrayExample {
            controller,
            tab: Tab::Content,
            own_window_id,
            msg_tx: tx,
            edit: None,
            sign_draft: Vec::new(),
            sign_draft_for: None,
            sign_error: None,
        };
        example.add_icon(None, cx);
        if example.controller.popup_mode {
            example.apply_popup_window_style(cx);
        }
        // TRAY_SMOKE=1: a scripted pass for the GUI test harness — play an
        // animation, add a sign, open its preview, then quit — so the
        // checklist lines a script asserts on appear without a hand on the
        // mouse.
        if std::env::var("TRAY_SMOKE").as_deref() == Ok("1") {
            let icon = example.controller.selected;
            example.play(
                crate::icon_animations::IconAnimation::Spinner,
                icon,
                false,
                cx,
            );
            let number = example
                .add_sign(crate::signs::sign_style::SignStyle::Missing, cx)
                .unwrap_or_default();
            cx.spawn(async move |this, cx| {
                for step in 0..10 {
                    cx.background_executor()
                        .timer(std::time::Duration::from_secs(1))
                        .await;
                    let _ = this.update(cx, |this, cx| {
                        if step == 1 {
                            // Give the sign's tray height a moment to settle.
                            this.enlarge_preview(number, cx);
                        }
                        if step == 2 {
                            // Three icons, three animations, all at once.
                            this.play_three_at_once(cx);
                        }
                        if step == 9 {
                            this.dispose_icons();
                            cx.quit();
                        }
                    });
                }
            })
            .detach();
        }
        example
    }

    /// A fresh handle to this example's own window, if nativeapi has one.
    pub(crate) fn own_window(&self) -> Option<NativeWindow> {
        self.own_window_id
            .and_then(nativeapi::window_manager::WindowManager::get)
    }
}

impl Render for TrayExample {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        self.sync_sign_draft();
        let t = cx.theme();
        let canvas = t.color_canvas.hsla();
        let this = cx.entity();
        let controller = &self.controller;
        let selected = controller.selected_entry();
        let mut column = div().flex_none();

        // The title bar row: the selected item, the tabs, the remove button.
        {
            let t2 = cx.theme();
            let mut row = div()
                .flex()
                .flex_none()
                .items_center()
                .gap(px(t2.spacing_2))
                .h(px(t2.frame_titlebar_size))
                .px(px(t2.spacing_4))
                .child(
                    div()
                        .flex_1()
                        .min_w_0()
                        .overflow_hidden()
                        .whitespace_nowrap()
                        .text_ellipsis()
                        .type_style(t2.title_small)
                        .child(SharedString::from(
                            selected
                                .map(|entry| entry.label())
                                .unwrap_or_else(|| "Tray items".to_string()),
                        )),
                );
            if let Some(entry) = selected {
                row = row.child(
                    SegmentedControl::new(
                        "tabs",
                        vec![
                            SegmentedItem::new(Tab::Content.value(), "Content"),
                            SegmentedItem::new(Tab::Properties.value(), "Properties"),
                            SegmentedItem::new(
                                Tab::Checklist.value(),
                                format!(
                                    "Checklist {}/{}",
                                    controller.checklist.passed(),
                                    controller.checklist.items.len()
                                ),
                            ),
                        ],
                        self.tab.value(),
                    )
                    .size(WidgetSize::Small)
                    .on_value_change({
                        let this = this.clone();
                        move |value, _, cx| {
                            let tab = match value.as_ref() {
                                "properties" => Tab::Properties,
                                "checklist" => Tab::Checklist,
                                _ => Tab::Content,
                            };
                            this.update(cx, |this, cx| {
                                this.tab = tab;
                                cx.notify();
                            });
                        }
                    }),
                );
                let number = entry.number;
                let remove_label = format!("Remove {}", entry.label().to_lowercase());
                row = row.child(
                    IconButton::new("remove", remove_label)
                        .size(WidgetSize::Small)
                        .child(Icon::new(dazzui::glyphs::DISMISS_REGULAR))
                        .on_click({
                            let this = this.clone();
                            move |_, _, cx| {
                                this.update(cx, |this, cx| this.remove_icon(number, cx));
                            }
                        }),
                );
            }
            column = column.child(row).child(Divider::new());
        }

        // The body: the preview strip, then the tab content.
        let mut body = div().flex().flex_1().flex_col().min_h_0();
        match selected {
            None => {
                body = body.child(
                    div()
                        .flex_1()
                        .flex()
                        .items_center()
                        .justify_center()
                        .child("Use Add to create your first tray item."),
                );
            }
            Some(entry) => {
                let preview = div()
                    .flex_none()
                    .h(px(112.))
                    .bg(t.color_surface_sunken.hsla())
                    .child(sign_or_live_preview(entry, &this, cx));
                let example: &TrayExample = self;
                let tab_body = match self.tab {
                    Tab::Content => {
                        if entry.sign.is_some() {
                            tabs::sign_tab(example, entry, &this, cx).into_any_element()
                        } else {
                            tabs::animate_tab(entry, &this, cx).into_any_element()
                        }
                    }
                    Tab::Properties => {
                        tabs::properties_tab(example, entry, &this, cx).into_any_element()
                    }
                    Tab::Checklist => {
                        tabs::checklist_tab(&example.controller, &this, cx).into_any_element()
                    }
                };
                let content = div()
                    .id("tab-content")
                    .flex_1()
                    .min_h_0()
                    .overflow_y_scroll()
                    .child(tab_body);
                body = body.child(preview).child(Divider::new()).child(content);
            }
        }
        column = column.child(body).child(Divider::new());

        // The footer: the last event, the newest log line, the log popover.
        column = column.child(footer(self, &this, cx));

        // The sidebar and the main column, on the canvas.
        div()
            .id("root")
            .flex()
            .size_full()
            .bg(canvas)
            .child(sidebar(self, &this, cx))
            .child(div().flex().flex_1().flex_col().min_w_0().child(column))
            .when(self.edit.is_some(), |page| {
                page.child(edit_dialog(self, &this, cx))
            })
    }
}

/// The 176 px column: the header, the item groups, the Add menu.
fn sidebar(example: &mut TrayExample, this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let mut groups = div().flex().flex_col().gap(px(t.spacing_2));
    for is_sign in [false, true] {
        let entries: Vec<_> = example
            .controller
            .entries
            .iter()
            .filter(|entry| entry.is_sign == is_sign)
            .collect();
        if entries.is_empty() {
            continue;
        }
        let mut cards = div().flex().flex_col();
        for entry in entries {
            let number = entry.number;
            let selected = example.controller.selected == Some(number);
            cards = cards.child(
                dazzui::OptionCard::new(
                    ("entry", number as u64),
                    div().child(SharedString::from(entry.label())),
                )
                .description(SharedString::from(entry.description()))
                .selected(selected)
                .on_click({
                    let this = this.clone();
                    move |_, _, cx| {
                        this.update(cx, |this, cx| this.select(number, cx));
                    }
                }),
            );
        }
        groups = groups.child(
            div()
                .flex()
                .flex_col()
                .py(px(t.spacing_1_5))
                .child(
                    div()
                        .px(px(t.spacing_2_5))
                        .py(px(t.spacing_1))
                        .type_style(t.label_small)
                        .text_color(t.color_content_faint.hsla())
                        .child(if is_sign { "Signs" } else { "Icons" }),
                )
                .child(cards),
        );
    }

    div()
        .flex()
        .flex_none()
        .flex_col()
        .h_full()
        .w(px(176.))
        .bg(t.color_surface_column.hsla())
        .border_r(px(t.stroke_hairline))
        .border_color(t.color_border.hsla())
        .child(
            div()
                .flex()
                .flex_none()
                .items_center()
                .justify_between()
                .h(px(t.frame_titlebar_size))
                .px(px(t.spacing_4))
                .child(div().type_style(t.title_small).child("Tray Icon"))
                // Which toolkit draws this example, at a glance — the
                // counterpart examples carry the same tag (Flutter, GPUI…).
                .child(
                    dazzui::Tag::new("stack-tag")
                        .size(dazzui::Size::Small)
                        .child("GPUI"),
                ),
        )
        .child(
            div()
                .id("sidebar-groups")
                .flex_1()
                .min_h_0()
                .overflow_y_scroll()
                .p(px(t.spacing_2_5))
                .child(groups),
        )
        .child(
            div()
                .flex()
                .flex_none()
                .p(px(t.spacing_2_5))
                .child(widgets::add_tray_menu(this, cx)),
        )
}

/// The preview strip: the drawn sign for a sign, the tray's own pixels for
/// an icon.
fn sign_or_live_preview(
    entry: &crate::tray_controller::TrayEntry,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    if let Some(sign) = &entry.sign {
        let t = cx.theme();
        // The Flutter example hosts the native sign in an 80 pt strip at the
        // tray's own bar height scaled ×1.43 (40/28); the drawn replica
        // stands in at the same size, with the read-back line beside it.
        return div()
            .flex()
            .w_full()
            .items_center()
            .gap(px(t.spacing_2_5))
            .px(px(t.spacing_2_5))
            .overflow_hidden()
            .child(
                div()
                    .flex()
                    .flex_none()
                    .items_center()
                    .justify_center()
                    .child(SignArt::new(sign.style).content(sign.content()).height(46.)),
            )
            .child(
                div()
                    .flex()
                    .flex_1()
                    .min_w_0()
                    .flex_col()
                    .gap(px(t.spacing_0_5))
                    .child(
                        div()
                            .type_style(t.title_small)
                            .child(SharedString::from(sign.display_title())),
                    )
                    .child(
                        div()
                            .type_style(host::mono_style(cx))
                            .text_color(host::muted_color(cx))
                            .overflow_hidden()
                            .whitespace_nowrap()
                            .text_ellipsis()
                            .child(format!(
                                "contentView {} · {}×{} px · the same views the tray shows",
                                style_name(sign.style),
                                sign.tray_width().round(),
                                sign.tray_height().round(),
                            )),
                    ),
            )
            .into_any_element();
    }
    widgets::live_preview(entry, this, cx).into_any_element()
}

fn style_name(style: crate::signs::sign_style::SignStyle) -> &'static str {
    use crate::signs::sign_style::SignStyle;
    match style {
        SignStyle::Missing => "missing",
        SignStyle::Welcome => "welcome",
        SignStyle::Travel => "travel",
        SignStyle::Guide => "guide",
    }
}

/// The event footer: the headline, the newest call, the log popover, Clear.
fn footer(example: &mut TrayExample, this: &Entity<TrayExample>, cx: &App) -> impl IntoElement {
    let t = cx.theme();
    let log_len = example.controller.log.len();
    let mut log_lines = example.controller.log.clone();
    log_lines.truncate(60);
    div()
        .flex()
        .flex_none()
        .items_center()
        .gap(px(t.spacing_2))
        .px(px(t.spacing_4))
        .py(px(t.spacing_2))
        .child(
            div()
                .flex_1()
                .min_w_0()
                .flex()
                .flex_col()
                .child(
                    div()
                        .type_style(t.label_small)
                        .overflow_hidden()
                        .whitespace_nowrap()
                        .text_ellipsis()
                        .child(SharedString::from(example.controller.last_event.clone())),
                )
                .child(
                    div()
                        .type_style(host::mono_style(cx))
                        .text_color(host::muted_color(cx))
                        .overflow_hidden()
                        .whitespace_nowrap()
                        .text_ellipsis()
                        .child(SharedString::from(
                            example
                                .controller
                                .log
                                .first()
                                .cloned()
                                .unwrap_or_else(|| "No calls yet".to_string()),
                        )),
                ),
        )
        .child(
            Popover::new("log")
                .title("Calls and events")
                .side(dazzui::Side::Top)
                .align(dazzui::Align::End)
                .width(Length::Px(360.))
                .trigger({
                    move |press, _open, _window, _cx| {
                        host::ActionChip::new("log", format!("Log ({log_len})"))
                            .on_tap(Some(Box::new(move |window, cx| press(window, cx))))
                            .into_any_element()
                    }
                })
                .child(
                    div()
                        .id("log-lines")
                        .flex()
                        .flex_col()
                        .h(px(240.))
                        .overflow_y_scroll()
                        .gap(px(t.spacing_0_5))
                        .children(log_lines.into_iter().map(|line| {
                            div()
                                .type_style(host::mono_style(cx))
                                .text_color(host::muted_color(cx))
                                .child(SharedString::from(line))
                        })),
                ),
        )
        .child(host::ActionChip::new("clear", "Clear").on_tap({
            let this = this.clone();
            Some(Box::new(move |_, cx| {
                this.update(cx, |this, cx| this.clear_log(cx));
            }))
        }))
}

/// The Title… / Tooltip… dialog.
fn edit_dialog(
    example: &mut TrayExample,
    this: &Entity<TrayExample>,
    cx: &App,
) -> impl IntoElement {
    let t = cx.theme();
    let edit = example.edit.clone().expect("rendered only while editing");
    let value = edit.value.clone();
    let title = edit.title;
    let _target = edit.target;
    dazzui::Dialog::new("text-edit")
        .open(true)
        .on_open_change({
            let this = this.clone();
            move |open, _, cx| {
                if !open {
                    this.update(cx, |this, cx| {
                        this.edit = None;
                        cx.notify();
                    });
                }
            }
        })
        .child(
            div()
                .flex()
                .items_center()
                .px(px(t.spacing_4))
                .h(px(45.))
                .border_b(px(t.stroke_hairline))
                .border_color(t.color_border.hsla())
                .type_style(t.label_large)
                .child(title),
        )
        .child(
            div().p(px(t.spacing_4)).child(
                dazzui::TextField::new("text-edit-field")
                    .size(WidgetSize::Medium)
                    .value(SharedString::from(value))
                    .on_change({
                        let this = this.clone();
                        move |text, _, cx| {
                            this.update(cx, |this, cx| {
                                if let Some(edit) = this.edit.as_mut() {
                                    edit.value = text.to_string();
                                }
                                cx.notify();
                            });
                        }
                    })
                    .on_submit({
                        let this = this.clone();
                        move |_, _, cx| {
                            this.update(cx, |this, cx| this.apply_edit(cx));
                        }
                    }),
            ),
        )
        .child(
            div()
                .flex()
                .justify_end()
                .gap(px(t.spacing_2))
                .px(px(t.spacing_4))
                .py(px(t.spacing_3))
                .border_t(px(t.stroke_hairline))
                .border_color(t.color_border.hsla())
                .child(host::ActionChip::new("edit-cancel", "Cancel").on_tap({
                    let this = this.clone();
                    Some(Box::new(move |_, cx| {
                        this.update(cx, |this, cx| {
                            this.edit = None;
                            cx.notify();
                        });
                    }))
                }))
                .child(
                    host::ActionChip::new("edit-apply", "Apply")
                        .primary(true)
                        .on_tap({
                            let this = this.clone();
                            Some(Box::new(move |_, cx| {
                                this.update(cx, |this, cx| this.apply_edit(cx));
                            }))
                        }),
                ),
        )
}

impl TrayExample {
    /// Applies the open dialog's text to its target.
    pub fn apply_edit(&mut self, cx: &mut Context<Self>) {
        let Some(edit) = self.edit.take() else {
            return;
        };
        let value = edit.value.clone();
        match edit.target {
            EditTarget::Title => self.set_title(Some(&value), cx),
            EditTarget::Tooltip => self.set_tooltip(Some(&value), cx),
        }
    }

    /// Opens the Title… / Tooltip… dialog for the selected icon.
    pub fn open_edit(&mut self, target: EditTarget, cx: &mut Context<Self>) {
        let Some(entry) = self.controller.selected_entry() else {
            return;
        };
        let (title, value) = match target {
            EditTarget::Title => ("Title", entry.tray_icon.get_title().unwrap_or_default()),
            EditTarget::Tooltip => ("Tooltip", entry.tray_icon.get_tooltip().unwrap_or_default()),
        };
        self.edit = Some(EditRequest {
            target,
            title,
            value,
        });
        cx.notify();
    }
}

// The helpers the tabs share.
pub(crate) use crate::tray_controller::{bounds_supported, open_menu_supported, title_supported};
