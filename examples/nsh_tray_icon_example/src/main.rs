//! NSH draws the window; nativeapi owns the tray icon and its native menu.

mod native;

use nsh_core::{element::desktop::close_window, prelude::*};
use nsh_example_shared::{Root, button, on_main, page, unbounded};

fn main() -> std::process::ExitCode {
    nsh_example_shared::run(
        Root::of::<App>(),
        "NSH · Tray Icon",
        "dev.nativeapi.nsh.tray",
        native::shutdown,
    )
}

#[component]
fn App() -> impl IntoElement {
    let count = signal(0_u32);
    let status = signal(String::from("Creating tray icon…"));
    let visible = signal(false);
    let (events, receive) = unbounded();
    spawn(async move {
        match on_main(move || native::create(events)).await {
            Ok(()) => {
                visible.set(true);
                status.set("Tray icon is visible. Right-click it to open the menu.".into());
            }
            Err(error) => status.set(error.into()),
        }
        while let Ok(event) = receive.recv().await {
            match event {
                native::Event::Click => {
                    count.update(|n| *n += 1);
                    status.set("Received a native click.".into());
                }
                native::Event::Reset => {
                    count.set(0);
                    status.set("Counter reset from the native menu.".into());
                }
                native::Event::Quit => close_window(),
            }
        }
    });

    page(
        "System tray",
        "A native tray icon and context menu, with events delivered to a Rust NSH component.",
        view()
            .gap_4()
            .child(text(move || format!("Clicks: {}", count.get())).text_size(px(36.0)))
            .child(text(move || {
                if visible.get() {
                    "Tray: visible"
                } else {
                    "Tray: hidden"
                }
            }))
            .child(button("Toggle tray visibility", move || {
                spawn(async move {
                    match on_main(native::toggle).await {
                        Ok(shown) => {
                            visible.set(shown);
                            status.set(
                                if shown {
                                    "Tray icon shown."
                                } else {
                                    "Tray icon hidden. Use this button to show it again."
                                }
                                .into(),
                            );
                        }
                        Err(error) => status.set(error.into()),
                    }
                });
            }))
            .child(button("Reset counter", move || count.set(0)))
            .child(text(move || status.get()))
            .child(text(
                "Click the tray icon, or choose Count a click, Reset counter or Quit in its menu.",
            )),
    )
}
