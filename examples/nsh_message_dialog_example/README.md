# nsh_message_dialog_example

A [Native Shell](https://nsh.dev/) Rust window opens nativeapi system message
dialogs. **Show information** works on macOS and Windows. **Show confirmation**
uses the extended dialog API on Windows and displays its result when dismissed;
on macOS it reports that confirmation buttons are unavailable.

```sh
cargo run --manifest-path examples/nsh_message_dialog_example/Cargo.toml
```

Run from the repository root on macOS or Windows. See the
[shared setup and threading notes](../nsh_shared). `MessageDialog::open` is modal:
the platform thread waits for dismissal, while the NSH engine awaits the
response asynchronously. The dialog is created and destroyed on the platform
thread. Repeated requests are ignored while a dialog is open.
