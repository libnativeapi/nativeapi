# nsh_shortcut_example

A [Native Shell](https://nsh.dev/) Rust window registers nativeapi's global
**Ctrl+Shift+Space** shortcut and shows an activation counter. Press **Register**,
then try the shortcut with another app focused. **Toggle enabled** pauses it,
**Unregister** releases it, and **Invoke callback** exercises the native callback
without synthesizing a key press.

```sh
cargo run --manifest-path examples/nsh_shortcut_example/Cargo.toml
```

Run from the repository root on macOS or Windows. See the
[shared setup and threading notes](../nsh_shared). Registration is opt-in;
failures such as an occupied accelerator are displayed in the window. If macOS
requests Accessibility permission, grant it in System Settings and register
again. Closing the window unregisters only this example's shortcut.
