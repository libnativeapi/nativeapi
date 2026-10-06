import assert from "node:assert/strict";
import { createRequire } from "node:module";
import { Worker } from "node:worker_threads";

// The embedded fixture registers its observer before production cleanup.
// The standalone fixture is loaded first for the same reverse cleanup order.
const fixtureAddon = createRequire(import.meta.url)(process.argv[2]);
const fixture = fixtureAddon.__eventDeliveryFixture ?? fixtureAddon;
fixture.schedule();
const { Application, ShortcutManager, DisplayManager, EventRequest } = await import("../../bindings/js/lib/index.ts");
const { deliverEvent, native, stopEventLoop } = await import("../../bindings/js/lib/runtime.ts");
const tick = () => new Promise((resolve) => setTimeout(resolve, 20));
const deferred = () => {
  let resolve;
  const promise = new Promise((done) => { resolve = done; });
  return { promise, resolve };
};
async function wait(promise) {
  let timer;
  try {
    return await Promise.race([promise, new Promise((_, reject) => {
      timer = setTimeout(() => reject(new Error("native callback did not arrive")), 3000);
    })]);
  } finally { clearTimeout(timer); }
}
const requestCallback = (listener) => (event, delivery) =>
  deliverEvent(delivery, () => new EventRequest(event.request, false), listener);

if (process.argv[3]) {
  if (["owned", "owned-worker", "owned-accepted", "owned-disposed"].includes(process.argv[3])) {
    if (process.argv[3] === "owned-worker") {
      Application.addListener((event) => {
        if (event.type === "quitRequested") event.request.cancel();
      });
    }
    const started = deferred();
    globalThis.keptDecision = undefined;
    fixture.request(requestCallback((request) => {
      globalThis.keptDecision = request.defer();
      if (process.argv[3] === "owned-disposed") {
        const dropped = request.defer(); dropped.dispose();
      }
      started.resolve();
    }), true);
    await wait(started.promise); await tick();
    if (process.argv[3] === "owned-disposed") assert.equal(globalThis.keptDecision.isPending, false);
    else assert.ok(globalThis.keptDecision.isPending);
    if (process.argv[3] === "owned-accepted") {
      assert.equal(globalThis.keptDecision.accept(), true);
      assert.equal(fixture.outcome(), 1);
    }
  } else if (process.argv[3] === "pending") {
    const started = deferred();
    fixture.request(requestCallback(async () => {
      started.resolve();
      await new Promise(() => {});
    }), true);
    await wait(started.promise);
  } else {
    fixture.request(requestCallback(async () => {
      console.log("queued-callback-started");
      await new Promise(() => {});
    }), true);
  }
  const expected = process.argv[3] === "owned-accepted" ? 1 : 0;
  if (!["owned-accepted", "owned-disposed"].includes(process.argv[3])) assert.equal(fixture.outcome(), -1);
  fixture.checkCleanup(expected);
} else {
  const initial = fixture.live();
  const started = deferred(), gate = deferred(), finished = deferred();
  const listener = ShortcutManager.addListener(async (event) => {
    assert.equal(event.type, "registrationFailed");
    assert.equal(event.errorMessage, "owned UTF-8 payload: 异步确认");
    started.resolve();
    await gate.promise;
    assert.equal(event.accelerator, "Ctrl+ForeignThread");
    finished.resolve();
  });
  fixture.emitShortcut();
  await wait(started.promise);
  assert.equal(fixture.live(), initial + 1n);
  ShortcutManager.removeListener(listener);
  fixture.drain();
  gate.resolve();
  await wait(finished.promise);
  await tick();
  fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS foreign-thread generated event retains strings through Promise and removal");

  let calls = 0;
  const queued = ShortcutManager.addListener(() => { ++calls; });
  fixture.emitShortcut();
  ShortcutManager.removeListener(queued);
  await tick(); fixture.drain();
  assert.equal(calls, 0);
  assert.equal(fixture.live(), initial);
  console.log("PASS removed queued callback is skipped and released");

  let borrowed;
  const displayDone = deferred();
  const displayListener = DisplayManager.addListener(async (event) => {
    borrowed = event.display;
    const id = borrowed.id;
    assert.notEqual(id, 0);
    await tick();
    assert.equal(borrowed.id, id);
    displayDone.resolve();
  });
  fixture.emitDisplay();
  await wait(displayDone.promise);
  await tick();
  assert.equal(borrowed.id, 0);
  DisplayManager.removeListener(displayListener); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS borrowed identity survives await and expires after acknowledgment");

  const confirmation = deferred(), requestStarted = deferred();
  let saved;
  fixture.request(requestCallback(async (request) => {
    saved = request;
    assert.equal(request.isCancelable, true);
    requestStarted.resolve();
    await confirmation.promise;
    assert.equal(request.isPending, true);
  }), true);
  await wait(requestStarted.promise);
  assert.equal(fixture.outcome(), -1);
  confirmation.resolve(); await tick();
  assert.equal(fixture.outcome(), 1);
  assert.equal(saved.isPending, false);
  fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS implicit request vote waits for Promise then accepts");

  const error = deferred();
  process.once("unhandledRejection", (reason) => error.resolve(reason));
  fixture.request(requestCallback(async () => {
    await tick(); throw new Error("intentional event callback failure");
  }), true);
  assert.match((await wait(error.promise)).message, /intentional/);
  assert.equal(fixture.outcome(), 0);
  fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS rejected Promise cancels request and frees payload before error surfaces");

  const rawError = deferred();
  process.once("uncaughtException", (reason) => rawError.resolve({ reason, outcome: fixture.outcome() }));
  fixture.request(() => { throw new Error("intentional raw callback failure"); }, true);
  const rawFailure = await wait(rawError.promise);
  assert.match(rawFailure.reason.message, /intentional raw/);
  assert.equal(rawFailure.outcome, 0);
  fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS native invocation failure cancels request before uncaught handler runs");

  let owned;
  const voted = deferred();
  fixture.request(requestCallback((request) => {
    owned = request.defer(); voted.resolve();
  }), true);
  await wait(voted.promise); await tick();
  assert.equal(fixture.outcome(), -1);
  assert.equal(owned.accept(), true);
  assert.equal(fixture.outcome(), 1);
  owned.dispose(); fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS owned decision outlives callback and delivery payload");

  // Another worker must not cancel this environment's explicit vote or its
  // unresolved implicit delivery. Both load the exact same addon/handle table.
  const parentStarted = deferred(), parentGate = deferred();
  let parentRequest, parentVote;
  fixture.request(requestCallback(async (request) => {
    parentRequest = request; parentVote = request.defer();
    parentStarted.resolve(); await parentGate.promise;
  }), true);
  await wait(parentStarted.promise);
  const beforeWorker = fixture.live();
  const beforeListeners = fixture.applicationListeners();
  let childOutput = "";
  const child = new Worker(new URL(import.meta.url), {
    argv: [process.argv[2], "owned-worker"], stdout: true,
  });
  child.stdout.setEncoding("utf8"); child.stdout.on("data", (text) => { childOutput += text; });
  await wait(new Promise((resolve, reject) => {
    child.once("error", reject);
    child.once("exit", (code) => code === 0 ? resolve() : reject(new Error(`worker exit ${code}: ${childOutput}`)));
  }));
  // The C++ cleanup hook checks outcome and the shared handle count itself.
  // Its printf writes the process fd, bypassing Node Worker stdout routing.
  assert.equal(fixture.live(), beforeWorker);
  assert.equal(fixture.applicationListeners(), beforeListeners);
  assert.equal(fixture.outcome(), -1);
  assert.equal(parentVote.isPending, true);
  assert.equal(parentRequest.isPending, true);
  // Native UserData releases may still be queued from the closed worker.
  // Allocate a new callback before draining them to expose address-reuse bugs.
  const lateReleaseDone = deferred();
  const afterWorker = ShortcutManager.addListener(() => lateReleaseDone.resolve());
  fixture.drain(); fixture.emitShortcut();
  await wait(lateReleaseDone.promise); await tick();
  ShortcutManager.removeListener(afterWorker); fixture.drain();
  parentGate.resolve(); await tick();
  assert.equal(parentVote.accept(), true);
  assert.equal(fixture.outcome(), 1);
  parentVote.dispose(); fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS Worker teardown cancels only its own votes and preserves another environment's Promise");

  const observed = deferred(), requiredGate = deferred();
  fixture.request(requestCallback(async (request) => {
    assert.equal(request.isCancelable, false);
    assert.equal(request.cancel(), false);
    assert.equal(request.defer(), null);
    observed.resolve(); await requiredGate.promise;
  }), false);
  assert.equal(fixture.outcome(), 1);
  await wait(observed.promise); requiredGate.resolve(); await tick();
  fixture.removeRequest(); fixture.drain();
  assert.equal(fixture.live(), initial);
  console.log("PASS required shutdown never waits for asynchronous observer");

  // Run the real JS timer and native Application producer while substituting
  // only AppKit's pump/start. The fixture supplies a headless host delegate;
  // no NSWindow, activation, platform run loop or process exit is requested.
  const originalStart = native.startEventLoop, originalPump = native.pumpEventLoop;
  native.startEventLoop = () => {};
  native.pumpEventLoop = () => -1;
  let mode = "cancel", attempts = 0, loopResolved = false, explicitVote;
  const quitStarted = deferred(), quitGate = deferred();
  const quitListener = Application.addListener(async (event) => {
    if (event.type !== "quitRequested") return;
    ++attempts;
    assert.ok(event.request?.isCancelable);
    if (mode === "cancel") {
      assert.equal(event.request.cancel(), true);
    } else if (mode === "promise") {
      quitStarted.resolve(); await quitGate.promise;
      assert.equal(event.request.isPending, true);
    } else if (mode === "owned") {
      explicitVote = event.request.defer();
    } else if (mode === "reject") {
      throw new Error("quit confirmation rejected");
    }
  });
  try {
    const running = Application.run();
    running.then(() => { loopResolved = true; });
    Application.quit(43);
    await tick(); fixture.drain();
    assert.equal(loopResolved, false); assert.equal(attempts, 1);
    console.log("PASS actual Application quit veto keeps the JS loop running");
    mode = "promise";
    Application.quit(47);
    await wait(quitStarted.promise);
    Application.quit(53);
    await tick();
    assert.equal(loopResolved, false); assert.equal(attempts, 2);
    quitGate.resolve();
    assert.equal(await wait(running), 53);
    await tick(); fixture.drain();
    assert.equal(fixture.live(), initial);
    console.log("PASS actual Application quit awaits Promise once and uses the last exit code");

    mode = "reject";
    const retry = Application.run(); loopResolved = false;
    retry.then(() => { loopResolved = true; });
    const rejection = new Promise((resolve) => process.once("unhandledRejection", resolve));
    Application.quit(59);
    assert.match((await wait(rejection)).message, /quit confirmation rejected/);
    await tick(); fixture.drain();
    assert.equal(loopResolved, false);
    mode = "owned";
    Application.quit(61);
    await tick(); fixture.drain();
    assert.ok(explicitVote?.isPending); assert.equal(loopResolved, false);
    assert.equal(explicitVote.accept(), true);
    assert.equal(await wait(retry), 61);
    explicitVote.dispose(); await tick(); fixture.drain();
    assert.equal(fixture.live(), initial);
    console.log("PASS actual quit rejection permits retry; an owned decision can complete it later");
    mode = "owned";
    const oldRun = Application.run();
    Application.quit(67); await tick(); fixture.drain();
    const staleVote = explicitVote;
    assert.ok(staleVote?.isPending);
    stopEventLoop(71);
    assert.equal(await wait(oldRun), 71);
    const newRun = Application.run(); loopResolved = false;
    newRun.then(() => { loopResolved = true; });
    assert.equal(staleVote.accept(), false);
    staleVote.dispose();
    await tick(); fixture.drain();
    assert.equal(loopResolved, false);
    Application.quit(73); await tick(); fixture.drain();
    assert.ok(explicitVote?.isPending);
    assert.equal(explicitVote.accept(), true);
    assert.equal(await wait(newRun), 73);
    explicitVote.dispose(); await tick(); fixture.drain();
    assert.equal(fixture.live(), initial);
    console.log("PASS ending a JS loop invalidates old votes without ending the next run");
  } finally {
    stopEventLoop();
    Application.removeListener(quitListener); fixture.drain();
    native.startEventLoop = originalStart; native.pumpEventLoop = originalPump;
  }
}
