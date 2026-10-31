# Screen capture example

Flutter desktop example for `ScreenCapturer`, `CaptureSource`, and `ScreenRecorder`.
Requires macOS 14+ or Windows 10 version 2004+ with an available H.264 encoder.
Other platforms currently expose unsupported stubs.

## Run

From the workspace root:

```sh
flutter pub get
cd examples/flutter_screen_capture_example
flutter run -d macos
# On Windows:
flutter run -d windows
```

## Try it

1. Select **This window**, a display, or **System picker…**. On Windows the
   system picker lets the user choose either a window or a display.
2. Optionally include the cursor or crop a rectangle. Crop coordinates are
   physical pixels relative to the selected source, not desktop coordinates.
3. **Take screenshot** shows a preview; **Save PNG** writes a separate file.
4. **Start recording**, then **Stop & save** to finalize a silent H.264 MP4.
   **Discard** cancels and removes the temporary recording.
5. Choose an output directory to keep captures outside the default temporary
   directory. Each output receives a unique filename; existing files are not
   overwritten.

Allow the operating system's screen-capture request when prompted. Direct
capture on macOS may require Screen & System Audio Recording permission;
use the system picker to select a source explicitly. Closing the app during an
operation is blocked until it finishes or the recording is discarded.

The controller keeps native source/image/recorder handles alive until completion
and explicitly disposes them. Capture options are supplied explicitly because
binding defaults do not yet consistently match the C++ defaults. The screenshot
preview decodes the data URI returned by `Image.toBase64()`.

## Validation and remaining work

The macOS example has been built and run: actual screenshots (including a
2560 × 1440 frame) and short MP4 recordings succeeded. The Windows core capture
backend has desktop smoke coverage; the Flutter example has not yet been run
on Windows. Longer recordings, permission denial, source disappearance and
runtime shutdown still need end-to-end coverage.

The current **Open folder**, **Open** and **Play** actions use `UrlOpener`, whose
current implementation accepts HTTP/HTTPS only. Local files are saved correctly,
but these actions report an error; open the displayed path manually for now.
