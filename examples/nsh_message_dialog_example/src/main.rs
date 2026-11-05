//! A platform message dialog awaited by a Rust NSH component.

use nativeapi::{dialog::DialogModality, message_dialog::MessageDialog};
use nsh_core::prelude::*;
use nsh_example_shared::{Root, button, on_main, page};

fn main() -> std::process::ExitCode {
    nsh_example_shared::run(
        Root::of::<App>(),
        "NSH · Message Dialog",
        "dev.nativeapi.nsh.dialog",
        || {},
    )
}

// Created, opened, inspected and dropped entirely on the platform thread.
fn open(confirmation: bool) -> String {
    if confirmation && !MessageDialog::is_extended_supported() {
        return "This platform supports basic messages; confirmation buttons and results are unavailable.".into();
    }
    let Some(dialog) = MessageDialog::new(
        if confirmation {
            "Continue?"
        } else {
            "Hello from nativeapi"
        },
        "This is a system dialog opened from a Native Shell Rust component.",
    ) else {
        return "Could not create the message dialog.".into();
    };
    dialog.set_modality(DialogModality::Application);
    if confirmation && !dialog.set_buttons("Continue", "Go back", "Cancel") {
        return "Could not configure confirmation buttons.".into();
    }
    if !dialog.open() {
        return "Could not open the message dialog.".into();
    }
    if MessageDialog::is_extended_supported() {
        format!("Dialog dismissed: {:?}.", dialog.result())
    } else {
        "Dialog dismissed. This platform does not report a button result.".into()
    }
}

#[component]
fn App() -> impl IntoElement {
    let status = signal(String::from("Choose a dialog to open."));
    let busy = signal(false);
    let count = signal(0_u32);
    let show = move |confirmation| {
        if busy.get() {
            return;
        }
        busy.set(true);
        status.set("Waiting for the native dialog…".into());
        spawn(async move {
            status.set(on_main(move || open(confirmation)).await);
            busy.set(false);
        });
    };

    page(
        "Message dialogs",
        "NSH draws the controls. nativeapi opens the operating system's message dialog.",
        view().gap_4()
            .child(button("Show information", move || show(false)))
            .child(button("Show confirmation", move || show(true)))
            .child(text(move || status.get()))
            .child(text("Confirmation buttons are currently supported on Windows. macOS supports the basic information dialog."))
            .child(button("Count in NSH", move || count.update(|n| *n += 1)))
            .child(text(move || format!("NSH count: {}", count.get()))),
    )
}
