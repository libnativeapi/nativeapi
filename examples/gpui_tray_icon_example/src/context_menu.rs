//! The context menu of one tray icon, the counterpart of the Flutter
//! example's `context_menu.dart`.
//!
//! It is small on purpose but covers every item kind the checklist needs: a
//! normal item, a checkbox, a disabled item, a separator and a submenu — and
//! the submenu is useful too: it switches the icon's animation from the tray.

use futures::channel::mpsc::UnboundedSender;

use nativeapi::menu::{Menu, MenuBackend, MenuEvent, MenuItem, MenuItemState, MenuItemType};
use nativeapi::tray_icon::ListenerId;

use crate::icon_animations::{self as animations, IconAnimation};
use crate::tray_controller::TrayMsg;

/// What the menu of one tray icon tells the controller. Listeners fire on the
/// platform thread with no GPUI context; everything is routed through a
/// channel and applied from the controller's GPUI task.
#[derive(Clone, Copy, Debug)]
pub enum MenuMsg {
    Opened,
    Closed,
    SubmenuOpened,
    ShowWindow,
    EnlargePreview,
    Animation(Option<IconAnimation>),
    Checkbox(bool),
    About,
    Quit,
}

/// The menu of tray icon `number`, with everything kept alive: the submenus,
/// the items and every listener.
///
/// Handles are single-ownership, so the listener that flips the checkbox
/// borrows its item by raw handle. The safety invariant:
/// [`TrayMenu::dispose`] removes every listener before the items are
/// dropped, so a live listener always finds a live item behind its handle.
pub struct TrayMenu {
    pub menu: Menu,
    /// Tracked from the opened/closed events.
    pub is_open: std::cell::Cell<bool>,
    number: u32,
    sender: UnboundedSender<TrayMsg>,
    /// Items in add order; dropped in reverse on [`TrayMenu::dispose`].
    items: Vec<MenuItem>,
    menus: Vec<Menu>,
    listeners: Vec<(RawItem, ListenerId)>,
    menu_listener: Option<ListenerId>,
}

/// A menu item borrowed by raw handle; see [`TrayMenu`]'s documentation.
#[derive(Clone, Copy)]
struct RawItem(u64);

impl RawItem {
    /// # Safety
    /// The item behind `handle` must outlive every use of this value.
    unsafe fn with<R>(&self, f: impl FnOnce(&MenuItem) -> R) -> Option<R> {
        let item = unsafe { MenuItem::from_raw(self.0 as _) }?;
        Some(f(&item))
    }
}

impl TrayMenu {
    pub fn new(
        number: u32,
        backend: MenuBackend,
        is_sign: bool,
        sender: &UnboundedSender<TrayMsg>,
    ) -> Option<Self> {
        let menu = Menu::new()?;
        menu.set_backend(backend);
        let mut tray_menu = TrayMenu {
            menu,
            is_open: std::cell::Cell::new(false),
            number,
            sender: sender.clone(),
            items: Vec::new(),
            menus: Vec::new(),
            listeners: Vec::new(),
            menu_listener: None,
        };

        // The menu's own open/close events.
        let tx = sender.clone();
        tray_menu.menu_listener = Some(tray_menu.menu.add_listener(move |event: &MenuEvent| {
            let msg = match event {
                MenuEvent::Opened { .. } => MenuMsg::Opened,
                MenuEvent::Closed { .. } => MenuMsg::Closed,
                _ => return,
            };
            let _ = tx.unbounded_send(TrayMsg::Menu { number, msg });
        }));

        tray_menu.add_item("Show window", MenuMsg::ShowWindow);
        tray_menu.add_separator();

        if is_sign {
            // A sign's menu enlarges the preview instead of animating.
            tray_menu.add_item("Enlarge preview", MenuMsg::EnlargePreview);
        } else {
            let animations_menu = Menu::new()?;
            for animation in animations::ALL {
                tray_menu.item_into(
                    &animations_menu,
                    animation.label(),
                    MenuMsg::Animation(Some(animation)),
                );
            }
            animations_menu.add_separator();
            tray_menu.item_into(&animations_menu, "Stop", MenuMsg::Animation(None));

            let animate = MenuItem::with_label_and_type("Animate", MenuItemType::Submenu)?;
            animate.set_submenu(Some(&animations_menu));
            let tx = tray_menu.sender.clone();
            let listener = animate.add_listener(move |event: &MenuEvent| {
                if matches!(event, MenuEvent::ItemSubmenuOpened { .. }) {
                    let _ = tx.unbounded_send(TrayMsg::Menu {
                        number,
                        msg: MenuMsg::SubmenuOpened,
                    });
                }
            });
            tray_menu
                .listeners
                .push((RawItem(animate.as_raw()), listener));
            tray_menu.menu.add_item(Some(&animate));
            tray_menu.items.push(animate);
            tray_menu.menus.push(animations_menu);
        }

        let checkbox = MenuItem::with_label_and_type("Notifications", MenuItemType::Checkbox)?;
        checkbox.set_state(MenuItemState::Checked);
        let tx = tray_menu.sender.clone();
        let checkbox_raw = RawItem(checkbox.as_raw());
        let listener = checkbox.add_listener(move |event: &MenuEvent| {
            if !matches!(event, MenuEvent::ItemClicked { .. }) {
                return;
            }
            // SAFETY: the checkbox outlives its listener (dispose removes it).
            let checked = unsafe {
                checkbox_raw.with(|item| {
                    let checked = item.state() != MenuItemState::Checked;
                    item.set_state(if checked {
                        MenuItemState::Checked
                    } else {
                        MenuItemState::Unchecked
                    });
                    checked
                })
            };
            if let Some(checked) = checked {
                let _ = tx.unbounded_send(TrayMsg::Menu {
                    number,
                    msg: MenuMsg::Checkbox(checked),
                });
            }
        });
        tray_menu.listeners.push((checkbox_raw, listener));
        tray_menu.menu.add_item(Some(&checkbox));
        tray_menu.items.push(checkbox);

        let disabled = MenuItem::with_label_and_type("Check for updates", MenuItemType::Normal)?;
        disabled.set_enabled(false);
        tray_menu.menu.add_item(Some(&disabled));
        tray_menu.items.push(disabled);

        tray_menu.add_item("About", MenuMsg::About);
        tray_menu.add_separator();
        tray_menu.add_item("Quit", MenuMsg::Quit);

        Some(tray_menu)
    }

    /// Releases everything the menu owns: the listeners first, then the
    /// items in reverse add order. Dropping does the same.
    #[allow(dead_code)]
    pub fn dispose(mut self) {
        if let Some(listener) = self.menu_listener.take() {
            self.menu.remove_listener(listener);
        }
        for (item, listener) in self.listeners.drain(..) {
            // SAFETY: the item is still alive; its listener is being removed.
            unsafe { item.with(|item| item.remove_listener(listener)) };
        }
        self.menus.clear();
        self.items.clear();
    }

    /// An item of this tray icon's menu: clicked, it sends `msg`.
    fn add_item(&mut self, label: &str, msg: MenuMsg) {
        let item = self.plain_item(label, msg);
        self.menu.add_item(Some(&item));
        self.items.push(item);
    }

    /// The same item, added to another menu of this tray icon's menu tree.
    fn item_into(&mut self, parent: &Menu, label: &str, msg: MenuMsg) {
        let item = self.plain_item(label, msg);
        parent.add_item(Some(&item));
        self.items.push(item);
    }

    fn plain_item(&mut self, label: &str, msg: MenuMsg) -> MenuItem {
        let item =
            MenuItem::with_label_and_type(label, MenuItemType::Normal).expect("a normal item");
        let tx = self.sender.clone();
        let number = self.number;
        let listener = item.add_listener(move |event: &MenuEvent| {
            if matches!(event, MenuEvent::ItemClicked { .. }) {
                let _ = tx.unbounded_send(TrayMsg::Menu { number, msg });
            }
        });
        self.listeners.push((RawItem(item.as_raw()), listener));
        item
    }

    fn add_separator(&mut self) {
        let separator =
            MenuItem::with_label_and_type("", MenuItemType::Separator).expect("a separator");
        self.menu.add_item(Some(&separator));
        self.items.push(separator);
    }
}
