import 'dart:async';
import 'dart:ffi';

import 'package:nativeapi/nativeapi.dart';

void check(bool condition, String description) {
  if (!condition) throw StateError(description);
  print('PASS: $description');
}

Future<void> tick() => Future<void>.delayed(const Duration(milliseconds: 30));

Future<void> main(List<String> args) async {
  if (args.single == '--prepare') return; // Build the exact native asset first.
  final fixture = DynamicLibrary.open(args.single);
  final schedule = fixture.lookupFunction<Void Function(), void Function()>(
    'nativeapi_test_delivery_scheduler',
  );
  final drain = fixture.lookupFunction<Void Function(), void Function()>(
    'nativeapi_test_delivery_drain',
  );
  final restore = fixture.lookupFunction<Void Function(), void Function()>(
    'nativeapi_test_delivery_restore',
  );
  final emitShortcut = fixture.lookupFunction<Void Function(), void Function()>(
    'nativeapi_test_emit_shortcut',
  );
  final emitDisplay = fixture.lookupFunction<Void Function(), void Function()>(
    'nativeapi_test_emit_display',
  );
  final liveHandles = fixture.lookupFunction<Uint64 Function(), int Function()>(
    'nativeapi_test_delivery_live_handles',
  );
  final requestQuit = fixture
      .lookupFunction<Void Function(Int32), void Function(int)>(
        'nativeapi_test_request_quit',
      );
  final quitResult = fixture.lookupFunction<Int32 Function(), int Function()>(
    'nativeapi_test_quit_result',
  );
  schedule();
  try {
    final initial = liveHandles();
    final started = Completer<void>();
    final gate = Completer<void>();
    final finished = Completer<void>();
    final listener = ShortcutManager.instance.addListener((event) async {
      check(
        event is ShortcutRegistrationFailedEvent,
        'foreign-thread event reaches Dart isolate',
      );
      final failed = event as ShortcutRegistrationFailedEvent;
      check(
        failed.errorMessage == 'owned UTF-8 payload: 异步确认',
        'owned UTF-8 payload survives native return',
      );
      started.complete();
      await gate.future;
      check(
        failed.accelerator == 'Ctrl+ForeignThread',
        'callback payload survives an await',
      );
      finished.complete();
    });
    emitShortcut();
    await started.future.timeout(const Duration(seconds: 3));
    check(
      liveHandles() == initial + 1,
      'asynchronous callback keeps delivery alive',
    );
    ShortcutManager.instance.removeListener(listener);
    drain();
    gate.complete();
    await finished.future.timeout(const Duration(seconds: 3));
    await tick();
    drain();
    await tick();
    check(
      liveHandles() == initial,
      'removal during await releases completed delivery',
    );

    var calls = 0;
    final queued = ShortcutManager.instance.addListener((_) {
      ++calls;
    });
    emitShortcut(); // Native thread queues callback; Dart has not yielded yet.
    ShortcutManager.instance.removeListener(queued);
    drain();
    await tick();
    drain();
    await tick();
    check(
      calls == 0 && liveHandles() == initial,
      'removed queued callback is skipped and freed',
    );

    Display? borrowed;
    final displayFinished = Completer<void>();
    final displayListener = DisplayManager.instance.addListener((event) async {
      borrowed = event.display;
      final id = borrowed!.id;
      check(id != 0, 'borrowed identity resolves on the isolate');
      await tick();
      check(borrowed!.id == id, 'borrowed identity remains valid across await');
      displayFinished.complete();
    });
    emitDisplay();
    await displayFinished.future.timeout(const Duration(seconds: 3));
    await tick();
    check(
      borrowed!.id == 0,
      'completed callback invalidates borrowed handle safely',
    );
    DisplayManager.instance.removeListener(displayListener);
    drain();
    await tick();
    check(
      liveHandles() == initial,
      'borrowed identity and delivery leave no handles',
    );

    final error = Completer<Object>();
    late int throwing;
    runZonedGuarded(
      () {
        throwing = ShortcutManager.instance.addListener((_) async {
          await tick();
          throw StateError('intentional asynchronous callback failure');
        });
      },
      (exception, stack) {
        error.complete(exception);
      },
    );
    emitShortcut();
    final exception = await error.future.timeout(const Duration(seconds: 3));
    check(
      exception is StateError && liveHandles() == initial,
      'callback exception releases delivery before error reaches Zone',
    );
    ShortcutManager.instance.removeListener(throwing);
    drain();
    await tick();

    final quitStarted = Completer<void>();
    final quitGate = Completer<void>();
    var quitCalls = 0;
    final quitListener = Application.instance.addListener((event) async {
      if (event is! ApplicationQuitRequestedEvent) return;
      ++quitCalls;
      check(
        event.request.isCancelable,
        'actual quit carries shared confirmation to Dart',
      );
      quitStarted.complete();
      await quitGate.future;
      check(
        event.request.isPending,
        'borrowed quit request remains live through Future',
      );
    });
    requestQuit(43);
    await quitStarted.future.timeout(const Duration(seconds: 3));
    check(quitResult() == -1, 'actual quit awaits the Dart listener Future');
    requestQuit(47);
    check(
      quitCalls == 1 && quitResult() == -1,
      'pending actual quit deduplicates another request',
    );
    quitGate.complete();
    await tick();
    drain();
    await tick();
    check(
      quitResult() == 47 && liveHandles() == initial,
      'Dart approval completes once using the latest exit code',
    );
    Application.instance.removeListener(quitListener);
    drain();

    final vetoed = Completer<void>();
    final vetoListener = Application.instance.addListener((event) async {
      if (event is! ApplicationQuitRequestedEvent) return;
      await tick();
      check(event.request.cancel(), 'Dart can veto actual quit after an await');
      vetoed.complete();
    });
    requestQuit(53);
    await vetoed.future.timeout(const Duration(seconds: 3));
    await tick();
    drain();
    check(
      quitResult() == -1 && liveHandles() == initial,
      'Dart veto leaves the loop running without borrowed handles',
    );
    Application.instance.removeListener(vetoListener);
    drain();
    requestQuit(59);
    check(
      quitResult() == 59,
      'a later quit retries successfully after Dart cancellation',
    );
  } finally {
    drain();
    restore();
  }
}
