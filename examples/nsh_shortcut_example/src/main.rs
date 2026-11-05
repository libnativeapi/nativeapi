//! A native global shortcut updating NSH signals through a channel.

mod native;

use nsh_core::prelude::*;
use nsh_example_shared::{Root, button, on_main, page, unbounded};

fn main() -> std::process::ExitCode {
    nsh_example_shared::run(
        Root::of::<App>(),
        "NSH · Global Shortcut",
        "dev.nativeapi.nsh.shortcut",
        native::shutdown,
    )
}

#[component]
fn App() -> impl IntoElement {
    let count = signal(0_u32);
    let status = signal(String::from(
        "Not registered. Press Register to start listening.",
    ));
    let (events, receive) = unbounded();
    spawn(async move {
        while receive.recv().await.is_ok() {
            count.update(|n| *n += 1);
        }
    });

    page(
        "Global shortcut",
        "Register Ctrl+Shift+Space, then try it while another app has focus.",
        view().gap_4()
            .child(text("Ctrl + Shift + Space").text_size(px(26.0)))
            .child(text(move || format!("Activations: {}", count.get())).text_size(px(36.0)))
            .child(button("Register", move || {
                let events = events.clone();
                spawn(async move { status.set(on_main(move || native::register(events)).await.into()); });
            }))
            .child(button("Toggle enabled", move || {
                spawn(async move { status.set(on_main(native::toggle).await.into()); });
            }))
            .child(button("Invoke callback", move || {
                spawn(async move { status.set(on_main(native::invoke).await.into()); });
            }))
            .child(button("Unregister", move || {
                spawn(async move { status.set(on_main(native::unregister).await.into()); });
            }))
            .child(text(move || status.get()))
            .child(text("Invoke callback exercises native delivery without a key press. A busy shortcut or missing OS permission is reported here.")),
    )
}
