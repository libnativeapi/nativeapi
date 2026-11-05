use std::cell::RefCell;

use nativeapi::{
    geometry::Size,
    image::Image,
    menu::{Menu, MenuEvent, MenuItem, MenuItemType},
    tray_icon::{ContextMenuTrigger, TrayIcon, TrayIconEvent},
    tray_manager::TrayManager,
};
use nsh_example_shared::Sender;

pub enum Event {
    Click,
    Reset,
    Quit,
}

// Same visible diamond as flutter_tray_icon_example; no file path at runtime.
const ICON: &str = concat!(
    "data:image/png;base64,",
    "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAAfklEQVR42u3YywrAIAxEUf//",
    "p9NNt4ViHj1QB2Yn4UrUJK519FPFbQqGgIoHUzCfQMVLUzAjULFpCqYFKoo8BrOzth2mHSoT",
    "uByqImAZVOXu0rE68p+KyQFxKSMPNXntyYeRLB1kcSXbD7JBI1tYssknxyByUCRHafKz4WhU",
    "F0LqioTWhQw+AAAAAElFTkSuQmCC",
);

struct Tray {
    icon: TrayIcon,
    menu: Menu,
    _items: Vec<MenuItem>,
    icon_listener: nativeapi::tray_icon::ListenerId,
    menu_listener: nativeapi::menu::ListenerId,
}

impl Drop for Tray {
    fn drop(&mut self) {
        self.icon.remove_listener(self.icon_listener);
        self.menu.remove_listener(self.menu_listener);
        self.icon.set_visible(false);
        self.icon.set_context_menu(None);
    }
}

// Accessed exclusively by on_main() jobs and native platform callbacks.
thread_local! { static TRAY: RefCell<Option<Tray>> = const { RefCell::new(None) }; }

pub fn create(events: Sender<Event>) -> Result<(), &'static str> {
    if !TrayManager::is_supported() {
        return Err("System tray is unavailable on this platform.");
    }
    let icon = TrayIcon::new().ok_or("Could not create a tray icon.")?;
    let image = Image::from_base64(ICON).ok_or("Could not decode the tray icon.")?;
    icon.set_icon(Some(&image));
    icon.set_icon_template(cfg!(target_os = "macos"));
    icon.set_icon_size(&Size {
        width: 22.0,
        height: 22.0,
    });
    icon.set_title(Some("NSH"));
    icon.set_tooltip(Some("NativeAPI + Native Shell"));

    let menu = Menu::new().ok_or("Could not create the tray menu.")?;
    let mut items = Vec::new();
    for label in ["Count a click", "Reset counter", "Quit"] {
        let item = MenuItem::with_label_and_type(label, MenuItemType::Normal)
            .ok_or("Could not create a menu item.")?;
        menu.add_item(Some(&item));
        items.push(item);
    }
    let ids = [items[0].id(), items[1].id(), items[2].id()];
    let menu_events = events.clone();
    let menu_listener = menu.add_listener(move |event| {
        if let MenuEvent::ItemClicked { item_id } = event {
            let event = if *item_id == ids[0] {
                Event::Click
            } else if *item_id == ids[1] {
                Event::Reset
            } else if *item_id == ids[2] {
                Event::Quit
            } else {
                return;
            };
            let _ = menu_events.try_send(event);
        }
    });
    let icon_listener = icon.add_listener(move |event| {
        if matches!(event, TrayIconEvent::Clicked { .. }) {
            let _ = events.try_send(Event::Click);
        }
    });
    icon.set_context_menu(Some(&menu));
    icon.set_context_menu_trigger(ContextMenuTrigger::RightClicked);
    let tray = Tray {
        icon,
        menu,
        _items: items,
        icon_listener,
        menu_listener,
    };
    if !tray.icon.set_visible(true) {
        return Err("Could not show the tray icon.");
    }
    TRAY.with(|state| *state.borrow_mut() = Some(tray));
    Ok(())
}

pub fn toggle() -> Result<bool, &'static str> {
    TRAY.with(|state| {
        let state = state.borrow();
        let tray = state.as_ref().ok_or("No tray icon was created.")?;
        if !tray.icon.set_visible(!tray.icon.is_visible()) {
            return Err("Could not change tray visibility.");
        }
        Ok(tray.icon.is_visible())
    })
}

pub fn shutdown() {
    TRAY.with(|state| {
        state.borrow_mut().take();
    });
}
