# Windows compositor frame probe

Build with `cmake -S tools/gui/frame_probe -B tools/gui/frame_probe/build`, then
`cmake --build tools/gui/frame_probe/build --config Release`.

Run `frame_probe.exe "exact test window title" output.jsonl 30` before starting
the test window. It waits up to 40 seconds for that window, binds to its monitor,
and observes composed desktop images through DXGI Desktop Duplication. The ROI
is the middle 30–65% of the client width and 15–80% of its height. The target must
be visible, stationary and unobscured. The tool never sends input, changes the
display mode, or saves screenshots. An access, rotation, occlusion or capture
failure is an error, not a zero-frame pass.

When a producer records `frequency` and `samples` with `phase`, `start_qpc`,
`finish_qpc` and `cpu_ms`, join them with:

```text
python tools/gui/frame_probe/analyze.py capture.jsonl producer.json report.json
```

For frame identity, a test-only producer can paint a sequence strip **after its
real WM_PAINT** and record the same nonzero `frame_id` in each sample. Paint 48
black/white cells, each 3 physical pixels wide and 8 high, at ROI origin + (8, 8).
In least-significant-bit order encode `0xA55A`, the 16-bit sequence, then its
16-bit complement (white = 1). Use unique IDs within one run (at most 65535).
Include marker painting in `cpu_ms` and flush GDI before returning from paint.
The observer validates and decodes the strip independently from desktop pixels;
the analyzer reports the first observation of each producer ID, missing IDs,
intervals and update-to-composed-marker latency. Capture must continue briefly
after the final producer sample to observe its asynchronous presentation.
The matched-ID report also includes refresh-cycle counts and complete one-second
frame counts. Use these to check sustained cadence; an average near 60 Hz alone
does not establish smooth presentation. Missing producer IDs remain explicit.

This marker is test instrumentation, not part of a production UI. Its cost and
the observer's CPU/readback load remain in the paced test. Observing a marker
proves that the corresponding paint reached the compositor; it does not prove
that every pixel was published atomically. Keep pixel correctness tests separate.

Each record contains the desktop image's `LastPresentTime` QPC timestamp, capture
completion time, `AccumulatedFrames`, and a sampled RGB content hash. Pointer-only
updates are ignored. Compare **changed target hashes**, not all desktop updates,
against the test's steady-state QPC interval. Report both changed-frame intervals
and frames accumulated by the observer; accumulation can hide intermediate
frames and prevents a claim that every application frame was displayed.

The metadata includes monitor refresh and DWM refresh. A 60 Hz producer on a
165 Hz monitor naturally alternates between display intervals of two or three
refreshes. A desktop capture observes composed output; it does not measure the
panel's scanout, remote-stream delivery, or input-to-photon latency. Hash changes
alone cannot distinguish a partially published frame from a complete one: pair
them with application timing records and pixel correctness tests.

API semantics: [DXGI frame information](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_outdupl_frame_info),
[AcquireNextFrame](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgioutputduplication-acquirenextframe).
