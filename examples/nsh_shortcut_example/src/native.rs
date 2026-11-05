use std::cell::RefCell;

use nativeapi::{shortcut::Shortcut, shortcut_manager::ShortcutManager};
use nsh_example_shared::Sender;

const ACCELERATOR: &str = "Ctrl+Shift+Space";

struct Registration(Shortcut);

impl Drop for Registration {
    fn drop(&mut self) {
        ShortcutManager::unregister_with_id(self.0.id());
    }
}

thread_local! { static SHORTCUT: RefCell<Option<Registration>> = const { RefCell::new(None) }; }

pub fn register(events: Sender<()>) -> &'static str {
    SHORTCUT.with(|state| {
        if state.borrow().is_some() { return "Already registered."; }
        if !ShortcutManager::is_supported() { return "Global shortcuts are unsupported on this platform."; }
        let Some(shortcut) = ShortcutManager::register_with_accelerator_and_callback(ACCELERATOR, move || {
            let _ = events.try_send(());
        }) else { return "Registration failed. Check OS permissions and whether the shortcut is already in use."; };
        *state.borrow_mut() = Some(Registration(shortcut));
        "Registered Ctrl+Shift+Space."
    })
}

pub fn toggle() -> &'static str {
    SHORTCUT.with(|state| {
        let state = state.borrow();
        let Some(Registration(shortcut)) = state.as_ref() else {
            return "Register a shortcut first.";
        };
        shortcut.set_enabled(!shortcut.is_enabled());
        if shortcut.is_enabled() {
            "Shortcut enabled."
        } else {
            "Shortcut disabled."
        }
    })
}

pub fn invoke() -> &'static str {
    SHORTCUT.with(|state| {
        let state = state.borrow();
        let Some(Registration(shortcut)) = state.as_ref() else {
            return "Register a shortcut first.";
        };
        if !shortcut.is_enabled() {
            return "Shortcut is disabled.";
        }
        shortcut.invoke();
        "Native callback invoked."
    })
}

pub fn unregister() -> &'static str {
    shutdown();
    "Shortcut unregistered."
}

pub fn shutdown() {
    SHORTCUT.with(|state| {
        state.borrow_mut().take();
    });
}
