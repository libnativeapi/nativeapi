# Native Shell + nativeapi examples

Three small Rust desktop apps combine [Native Shell (NSH)](https://nsh.dev/)
0.4.0 for the UI with the local nativeapi Rust binding for system integration:

| Example | Native APIs |
| --- | --- |
| [Tray icon](../nsh_tray_icon_example) | `TrayIcon`, `TrayManager`, `Menu`, `MenuItem`, `Image` |
| [Global shortcut](../nsh_shortcut_example) | `ShortcutManager`, `Shortcut` |
| [Message dialog](../nsh_message_dialog_example) | `MessageDialog`, `DialogModality` |

## Run

On macOS (14+) or Windows, install Rust 1.98+, CMake and a C++ compiler,
and initialize the core submodule from the repository root:

```sh
git submodule update --init core
cargo run --manifest-path examples/nsh_tray_icon_example/Cargo.toml
cargo run --manifest-path examples/nsh_shortcut_example/Cargo.toml
cargo run --manifest-path examples/nsh_message_dialog_example/Cargo.toml
```

Use `cargo run`: these apps embed NSH in their own host, which installs the
native dispatcher and releases native resources after the NSH engine stops.
The `nsh` CLI is not required. Closing the example window quits the app and
removes its tray icon or unregisters its shortcut.

The examples and this support crate share a separate Cargo workspace rooted
here, with one lockfile and build directory. From the repository root:

```sh
cargo check --manifest-path examples/nsh_shared/Cargo.toml --workspace --locked
cargo fmt --manifest-path examples/nsh_shared/Cargo.toml --all -- --check
# macOS desktop, with Accessibility permission for the terminal:
python3 tools/gui/nsh_examples_test_macos.py --build
```

Linux is currently excluded: nativeapi links GTK 3 and NSH's Linux backend links
GTK 4. Loading both GTK major versions in the same process is unsupported. These
examples also do not target mobile or the web.

## Threading and ownership

NSH runs Rust components, event handlers and reactive signals on an engine
thread. The system tray, menus and dialogs belong to the platform thread.
`on_main` schedules a closure there and asynchronously returns owned data. Native
callbacks send channel messages; an NSH `spawn` task receives them and writes
signals on the engine thread. Native handles and signals never cross threads.

On macOS, dispatch uses the main dispatch queue and NSH's `run_app`. On Windows,
the host mounts NSH's public `AppView` on the calling thread and pumps both its
window messages and dispatched native work. It does not start a second
nativeapi `Application::run` loop.

The examples keep native handles in platform-thread storage until the host's
shutdown callback. Listeners, menu items and registered shortcuts are released
there before the process exits. `nsh_example_shared` is example infrastructure,
not a published binding.
