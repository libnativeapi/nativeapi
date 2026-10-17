# HarmonyOS clipboard concurrency and lifetime acceptance — 2026-10-16

This checkpoint fixes the remaining reproduced clipboard read/observer defects
and adds phone/computer acceptance for permissions, directories and native host
lifetime. Public C++, C ABI and generated binding signatures are unchanged.
The larger HarmonyOS API project remains paused outside this clipboard task.
Core implementation and fixtures: `d66f7f9ed859724116bd58d81e5970cb1496639b`; commits are local, with no push.

## Corrections

- Read revision metadata is captured when the caller submits a read, including
  worker submissions. Both the beginning and end of actual conversion must match
  it. A write between submission and dispatch now produces `false` with a default
  payload rather than a successful read of replacement content. All five read
  variants are covered through both C++ and C.
- SDK observer delivery uses one UI dispatch with the subscription's active flag,
  native environment generation and emitter lifetime guard. The previous second
  `EmitAsync` hop lost subscription identity. Removing all listeners and adding
  new ones could therefore deliver an old notice to the new listeners. User
  callbacks run without the old listener mutex and may unsubscribe themselves.
- Native services can register cleanup from a worker without waiting for the UI
  thread. Environment cleanup disables dispatch, removes clipboard listeners and
  unsubscribes the native observer. Cleanup callbacks run outside the runtime
  mutex; initialization is refused while cleanup is in progress. Queued reads
  are discarded on N-API abort, and old SDK notices cannot enter a replacement
  environment. Failed posting during environment loss also drops old work.

The read defect was reproduced on a real computer guest: all thirty invalidated
reads incorrectly succeeded before the correction. The observer defect was also
reproduced: a held real SDK notice incremented both replacement C++ and C counters.
These failures were retained in the diagnostic logs; assertions were not weakened.

## Acceptance scenarios

The boundary suite validates:

- Thirty stable mixed reads and thirty reads invalidated by an actual subsequent
  native publication. C++/C UI calls and four joined worker threads exercise
  Read/ReadText/ReadHtml/ReadImage/ReadFilePaths. C delivery leases complete once;
  forty-five C read user-data releases are checked across all three batches.
- Exact independent producer text/HTML, all 24 RGBA bytes, and both actual foreign
  file byte streams. Requests for one format omit all other formats.
- Actual external SDK notifications, queued epoch replacement, subscription from
  a worker, partial removal, self-removal, last removal and no later delivery.
  An independent SDK observer proves a real change even when the test dispatcher
  refuses posting; nativeapi must not fall back to invoking users on the IPC thread.
- Directory URIs with Unicode, spaces and reserved characters, in both transfer
  directions, with exact bytes from two nested files. C++, C and worker readers,
  plus C++ and C publishers, use the actual public SDK and different application UIDs.
- Actual producer-file deletion: cached foreign paths become unreadable and fresh
  reads omit those unresolved URIs while preserving readable text/HTML/image data,
  as the existing public OHOS contract specifies.
- Real READ_PASTEBOARD revocation. The OS terminates the consumer, so the driver
  requires a new process and verifies its identity. An independent NDK read must
  report 201, followed by thirty nativeapi failures with no partial payload.
  Restoring permission must restore exact independent producer data.
- Three real ArkTS worker environments: pending reads, completed reads, pending
  reads again. Each owns an isolated symbol-bound copy of core and a real native
  pasteboard observer. Pending reads never invoke users; every environment releases
  all thirty read captures. Actual SDK notices are deliberately held across real
  environment disposal and drained after replacement listeners are installed.
  None may reach the new generation. The final held notices are also drained.

The worker fixture uses a second test-only native module linked with `-Bsymbolic`,
so destroying a worker exercises real N-API cleanup without replacing the application's
UI host. Pending workers close their own port in the same JS turn; an infinite JS
loop was rejected during fixture development because termination did not complete.
The independent clipboard peer still contains no native shared libraries.

## Results

Host: macbook-pro ARM64, DevEco Studio 26.0.0.851, HarmonyOS 7.0.0.107 / API 26.
Only NativeAPI_Phone or NativeAPI_Desktop is running at a time. All interaction
uses the isolated apps and public SDKs; no host mouse or keyboard input is sent.

| Guest | Functional / permission / interop PIDs | Boundary assertions | Cross-app regression |
| --- | --- | --- | --- |
| NativeAPI_Desktop | 7303 / 7550 / 11144 | 322 + 111 + 41 = 474, zero failures; 58 commands | 34 assertions, 17 commands, zero failures |
| NativeAPI_Phone | 18701 / 18927 / 22292 | 322 + 111 + 41 = 474, zero failures; 58 commands | 34 assertions, 17 commands, zero failures |

The boundary command count excludes the additional idempotent final cleanup.
The 41 lifetime assertions are the final cumulative count, not the sum of the
three intermediate snapshots. Observer notifications may coalesce; correctness
assertions use actual independent reference events and monotonic counters.

Local CTest passes 35/35. Read-only codegen verification passes 75 C ABI and 339
binding/reference outputs. Python compilation, Ruff E9/F and Bash syntax pass.
No generated files are changed and no claim is made about new native runtime
coverage on Windows, Linux, Android or iOS.

## Reproduction and retained evidence

With exactly one named disposable guest running, pass its actual HDC address:

```sh
source ~/Library/Huawei/ohos-env.sh
cd ~/.fastforge/_work/core-02d614d4e50d
NATIVEAPI_TEST_SUITE=clipboard-boundaries \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME" 127.0.0.1:5557
NATIVEAPI_TEST_SUITE=clipboard \
  NATIVEAPI_TEST_PYTHON="$HOME/Library/Huawei/test-tools/bin/python" \
  ./tests/ohos_smoke/run.sh "$DEVECO_STUDIO_HOME" 127.0.0.1:5557 --skip-build
```

The driver validates the named emulator, real application UIDs, exact process
command lines and fresh request IDs. It restores the debug read permission and
removes subscriptions after acceptance. Test applications and their data remain
installed; clipboard content is cleared at the end.

Remote archive: `~/Library/Huawei/Logs/clipboard-boundary-20261016/{2in1,phone}/`.
Local ignored copy: `core/build-ohos/verification/clipboard-boundary-20261016/`.
Each contains functional/permission/interop logs, JSON replies, final UI/window
captures, both HAPs, build logs and matching SHA-256 hashes for all 17 changed
core/fixture source files. HAP hashes are in `hap-hashes.json`.

## Evidence boundaries

This acceptance covers the public clipboard wrapper and independent public-SDK
applications. It does not claim an interactive paste through Huawei's system
File Manager, uninstall of the original peer, or privileged URI-grant revocation.
The shipped normal-app `uriPermissionManager` declarations expose no grant/revoke
functions. Files are returned only when the real consumer can read them; the
wrapper neither adds privileges nor promises that a returned path retains access.
Broader codec fuzzing and service fault injection are outside these reproduced
clipboard fixes. The tests above must not be described as those missing scenarios.
