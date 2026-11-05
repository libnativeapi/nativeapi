//! Small host shared by the NSH examples, not a published integration crate.
//!
//! NSH components and signals live on the engine thread. Native UI objects
//! stay on the platform thread; only owned data crosses the channel between them.

#[cfg(not(any(target_os = "macos", target_os = "windows")))]
compile_error!(
    "These NSH examples support macOS and Windows. NSH's GTK 4 backend cannot share a process with nativeapi's GTK 3 backend."
);

pub use async_channel::{Sender, unbounded};
use nsh_base::theme;
pub use nsh_core::element::Root;
use nsh_core::prelude::*;

#[cfg(target_os = "macos")]
use nsh_platform_macos as platform;
#[cfg(target_os = "windows")]
use nsh_platform_windows as platform;

/// Schedule native work without moving any native handle off its owning thread.
pub fn dispatch(work: impl FnOnce() + Send + 'static) {
    #[cfg(target_os = "macos")]
    dispatch2::DispatchQueue::main().exec_async(work);
    #[cfg(target_os = "windows")]
    win::dispatch(Box::new(work));
}

/// Await a plain-data result on the NSH engine without blocking its event loop.
pub async fn on_main<R: Send + 'static>(work: impl FnOnce() -> R + Send + 'static) -> R {
    let (send, receive) = async_channel::bounded(1);
    dispatch(move || {
        let _ = send.try_send(work());
    });
    receive.recv().await.expect("native task finished")
}

/// Run NSH's loop, then release native resources on their owning thread.
pub fn run(root: Root, title: &str, app_id: &str, shutdown: fn()) -> std::process::ExitCode {
    env_logger::init();
    let options = platform::AppOptions {
        title: title.to_owned(),
        app_id: app_id.to_owned(),
        size: (560, 620),
        ..Default::default()
    };
    #[cfg(target_os = "macos")]
    let result = platform::run_app(platform::AppSource::Rust(root), options);
    #[cfg(target_os = "windows")]
    let result = win::run(root, options, shutdown);
    #[cfg(target_os = "macos")]
    shutdown();
    result
}

pub fn button(label: &'static str, action: impl Fn() + 'static) -> impl IntoElement {
    nsh_base::button()
        .accessibility_label(Some(SharedString::from(label)))
        .px_4()
        .py_3()
        .rounded(px(8.0))
        .bg(move || theme().primary)
        .active(|s| s.opacity(0.7))
        .on_press(action)
        .child(text(label).text_color(move || theme().primary_foreground))
}

pub fn page(
    title: &'static str,
    description: &'static str,
    body: impl IntoElement,
) -> impl IntoElement {
    scroll()
        .flex_1()
        .bg(move || theme().background)
        .text_color(move || theme().foreground)
        .child(
            view()
                .p_8()
                .gap_6()
                .child(
                    text("NATIVEAPI + NATIVE SHELL")
                        .text_size(px(12.0))
                        .text_color(rgb(0x64748b)),
                )
                .child(text(title).text_size(px(30.0)))
                .child(text(description).text_size(px(15.0)))
                .child(body),
        )
}

#[cfg(target_os = "windows")]
mod win {
    use super::platform;
    use std::sync::{OnceLock, mpsc};
    use windows::Win32::Foundation::{LPARAM, WPARAM};
    use windows::Win32::{
        System::{
            Ole::{OleInitialize, OleUninitialize},
            Threading::GetCurrentThreadId,
        },
        UI::{
            HiDpi::{DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2, SetProcessDpiAwarenessContext},
            WindowsAndMessaging::{
                DispatchMessageW, GetMessageW, MSG, PostThreadMessageW, TranslateMessage, WM_APP,
            },
        },
    };

    type Work = Box<dyn FnOnce() + Send>;
    static QUEUE: OnceLock<(mpsc::Sender<Work>, u32)> = OnceLock::new();
    const WAKE: u32 = WM_APP + 0x72;

    pub fn dispatch(work: Work) {
        let (send, thread) = QUEUE.get().expect("the NSH host is running");
        if send.send(work).is_ok() {
            // SAFETY: the host creates its message queue before publishing its id.
            unsafe { PostThreadMessageW(*thread, WAKE, WPARAM(0), LPARAM(0)) }
                .expect("wake the platform thread");
        }
    }

    pub fn run(
        root: platform::Root,
        options: platform::AppOptions,
        shutdown: fn(),
    ) -> std::process::ExitCode {
        // Use AppView rather than run_app, which starts a separate UI thread.
        // Both NSH and nativeapi here belong to this thread and share its pump.
        unsafe {
            let _ = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            OleInitialize(None).expect("initialize OLE");
        }
        let view = platform::AppView::new(None, options).expect("create the NSH window");
        let (send, receive) = mpsc::channel::<Work>();
        // GetMessage's queue already exists because AppView created its window.
        QUEUE
            .set((send, unsafe { GetCurrentThreadId() }))
            .expect("one host per process");
        view.load(platform::App::Rust(root));
        view.show();
        let mut message = MSG::default();
        let result = loop {
            let status = unsafe { GetMessageW(&mut message, None, 0, 0) }.0;
            if status <= 0 {
                break if status == 0 {
                    std::process::ExitCode::SUCCESS
                } else {
                    std::process::ExitCode::FAILURE
                };
            }
            if message.message == WAKE {
                while let Ok(work) = receive.try_recv() {
                    work();
                }
            } else {
                unsafe {
                    let _ = TranslateMessage(&message);
                    DispatchMessageW(&message);
                }
            }
        };
        view.stop();
        drop(view);
        // Finish work already submitted before the engine stopped.
        while let Ok(work) = receive.try_recv() {
            work();
        }
        shutdown();
        unsafe { OleUninitialize() };
        result
    }
}
