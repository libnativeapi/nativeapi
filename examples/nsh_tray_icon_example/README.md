# nsh_tray_icon_example

A [Native Shell](https://nsh.dev/) Rust window controls a nativeapi tray icon.
Click the icon to increment the NSH counter, or right-click for **Count a click**,
**Reset counter** and **Quit**. The window can hide and show the tray icon, so a
hidden icon can always be restored.

```sh
cargo run --manifest-path examples/nsh_tray_icon_example/Cargo.toml
```

Run from the repository root on macOS or Windows. See the
[shared setup and threading notes](../nsh_shared). The PNG is embedded in the
binary; no working-directory-dependent asset path is needed. macOS treats it
as a template icon for light and dark menu bars. The native tray icon, menu,
items and listeners stay alive until the window closes or **Quit** is selected.
