package nativeapi

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"runtime"
	"strings"
	"testing"
	"time"
)

// TestMain keeps the process's initial OS thread for native dispatch. Tests run
// on Go worker goroutines and marshal every native operation through Dispatch.
func init() { runtime.LockOSThread() }

func TestMain(m *testing.M) {
	Init()
	done := make(chan int, 1)
	go func() { done <- m.Run() }()
	for {
		select {
		case code := <-done:
			PollEvents(0) // Drain the final callback releases.
			os.Exit(code)
		default:
			PollEvents(5 * time.Millisecond)
		}
	}
}

func onUI(t *testing.T, fn func()) {
	t.Helper()
	done := make(chan struct{})
	if err := Dispatch(func() { defer close(done); fn() }); err != nil {
		t.Fatalf("Dispatch failed: %v", err)
	}
	select {
	case <-done:
	case <-time.After(10 * time.Second):
		t.Fatal("UI callback timed out")
	}
}

func TestValues(t *testing.T) {
	onUI(t, func() {
		color := ColorFromHex("#123456")
		if color != (Color{R: 0x12, G: 0x34, B: 0x56, A: 255}) || color.ToRGBA() != 0x123456ff {
			t.Errorf("color round trip: %+v, %#x", color, color.ToRGBA())
		}
		if got := EdgeInsetsSymmetric(3, 7); got != (EdgeInsets{Top: 3, Right: 7, Bottom: 3, Left: 7}) {
			t.Errorf("insets: %+v", got)
		}
		accelerator := KeyboardAccelerator{Modifiers: ModifierKeyCtrl | ModifierKeyShift, Key: "A"}
		if accelerator.IsEmpty() || !strings.Contains(accelerator.String(), "A") {
			t.Errorf("accelerator: %q", accelerator.String())
		}
		values := []string{"", "你好", "a b"}
		raw, cleanup := stringsToC(values)
		defer cleanup()
		if got := stringsFromC(raw); !reflect.DeepEqual(got, values) {
			t.Errorf("string list: %v", got)
		}
	})
}

func TestPreferences(t *testing.T) {
	onUI(t, func() {
		prefs, err := NewPreferencesWithScope(fmt.Sprintf("go-test-%d-%d", os.Getpid(), time.Now().UnixNano()))
		if err != nil {
			t.Errorf("create preferences failed: %v", err)
			return
		}
		defer prefs.Release()
		key := "go-binding-你好"
		defer prefs.Remove(key)
		if err := prefs.Set(key, "value ✓"); err != nil || prefs.Get(key, "fallback") != "value ✓" {
			t.Error("preference string round trip failed")
		}
		if !prefs.Contains(key) || prefs.All()[key] != "value ✓" {
			t.Error("preference map round trip failed")
		}
		found := false
		for _, item := range prefs.Keys() {
			found = found || item == key
		}
		if !found {
			t.Error("preference key missing from list")
		}
		if prefs.Get("missing", "fallback") != "fallback" {
			t.Error("default value lost")
		}
		if err := prefs.Remove(key); err != nil {
			t.Error("remove preference failed")
		}
		copy := *prefs
		prefs.Release()
		copy.Release()
		if copy.nativeHandleValue() != 0 {
			t.Error("copied wrapper did not share release state")
		}
	})
}

func TestCallbacks(t *testing.T) {
	onUI(t, func() {
		calls := 0
		shortcut, err := NewShortcut(42, ShortcutOptions{
			Accelerator: "Ctrl+A", Description: "Go callback", Enabled: true,
			Callback: func() { calls++ },
		})
		if err != nil {
			t.Errorf("create shortcut failed: %v", err)
			return
		}
		defer shortcut.Release()
		shortcut.Invoke()
		shortcut.SetCallback(func() {
			calls += 10
			shortcut.SetCallback(nil) // Replacement during an active callback.
		})
		shortcut.Invoke()
		shortcut.Invoke()
		if calls != 11 || shortcut.ID() != 42 || shortcut.Description() != "Go callback" {
			t.Errorf("callback/struct round trip: calls=%d", calls)
		}
		second, err := NewShortcutWithCallback(43, "Ctrl+B", func() { calls++ })
		if err != nil {
			t.Errorf("callback constructor failed: %v", err)
			return
		}
		defer second.Release()
		second.Invoke()
		if calls != 12 {
			t.Errorf("constructor callback: %d", calls)
		}
	})
}

func TestEvents(t *testing.T) {
	delivered := make(chan ShortcutEvent, 2)
	var listener ListenerID
	onUI(t, func() {
		var err error
		listener, err = ShortcutManager.AddListener(func(event ShortcutEvent) { delivered <- event })
		if err != nil {
			t.Errorf("listener registration failed: %v", err)
			return
		}
		ShortcutManager.EmitShortcutActivated(73, "Ctrl+你好")
	})
	select {
	case event := <-delivered:
		if event.Type != ShortcutEventActivated || event.ShortcutID != 73 || event.Accelerator != "Ctrl+你好" {
			t.Errorf("event payload: %+v", event)
		}
	case <-time.After(10 * time.Second):
		t.Fatal("event timed out")
	}
	onUI(t, func() {
		if err := ShortcutManager.RemoveListener(listener); err != nil {
			t.Error("listener removal did not track registration")
		}
		if err := ShortcutManager.RemoveListener(listener); !errors.Is(err, ErrOperationFailed) {
			t.Errorf("unknown listener error: %v", err)
		}
		ShortcutManager.EmitShortcutActivated(74, "Ctrl+B")
	})
	// A second dispatched turn follows the queued emit and verifies removal.
	onUI(t, func() {
		select {
		case event := <-delivered:
			t.Errorf("removed listener received %+v", event)
		default:
		}
	})
}

func TestNilHandles(t *testing.T) {
	onUI(t, func() {
		var shortcut *Shortcut
		shortcut.Release()
		shortcut.SetCallback(nil)
		if shortcut.ID() != 0 {
			t.Error("nil handle should fail safely")
		}
		if id, err := Application.AddListener(nil); id != 0 || !errors.Is(err, ErrInvalidArgument) {
			t.Errorf("nil listener callback: %d, %v", id, err)
		}
		if err := Dispatch(nil); err != nil {
			t.Errorf("nil dispatch callback: %v", err)
		}
		for _, timeout := range []time.Duration{-1, 1 << 62} {
			if err := PollEvents(timeout); !errors.Is(err, ErrInvalidArgument) {
				t.Errorf("invalid timeout %v: %v", timeout, err)
			}
		}
	})
}

type callbackProbe struct{ data [128]byte }

func trackedCallback(released chan struct{}) func() {
	probe := &callbackProbe{}
	runtime.SetFinalizer(probe, func(*callbackProbe) { close(released) })
	return func() { runtime.KeepAlive(probe) }
}

func TestCallbackResourcesReleased(t *testing.T) {
	// Check both normal destruction and the C ABI's early-failure release path.
	for _, invalid := range []bool{false, true} {
		released := make(chan struct{})
		onUI(t, func() {
			var shortcut *Shortcut
			if !invalid {
				var err error
				shortcut, err = NewShortcutWithCallback(99, "Ctrl+Z", nil)
				if err != nil {
					t.Errorf("create tracked shortcut: %v", err)
					return
				}
			}
			shortcut.SetCallback(trackedCallback(released))
			shortcut.Release()
		})
		onUI(t, func() {}) // Ordered after the native callback release hook.
		deadline := time.Now().Add(5 * time.Second)
		for {
			runtime.GC()
			select {
			case <-released:
				goto reclaimed
			case <-time.After(10 * time.Millisecond):
				if time.Now().After(deadline) {
					t.Fatalf("callback retained after release (invalid=%v)", invalid)
				}
			}
		}
	reclaimed:
	}
}

// Compile against the standard interface, exercising String rather than ToString.
var _ fmt.Stringer = KeyboardAccelerator{}

func TestOperationErrors(t *testing.T) {
	onUI(t, func() {
		var prefs *Preferences
		if err := prefs.Set("key", "value"); !errors.Is(err, ErrOperationFailed) || !strings.Contains(err.Error(), "Preferences.Set") {
			t.Errorf("failed setter did not preserve operation: %v", err)
		}
		if prefs.Contains("key") {
			t.Error("a predicate should still return false for a missing value")
		}
		var decision *EventDecision
		if err := decision.Cancel(); !errors.Is(err, ErrOperationFailed) {
			t.Errorf("Cancel was mistaken for a Can predicate: %v", err)
		}
		var menu *Menu
		if id, err := menu.AddListener(func(MenuEvent) {}); id != 0 || !errors.Is(err, ErrOperationFailed) {
			t.Errorf("failed listener registration: %d, %v", id, err)
		}
		image, err := ImageFromFile(filepath.Join(t.TempDir(), "missing.png"))
		if image != nil || !errors.Is(err, ErrOperationFailed) {
			t.Errorf("failed factory: %v, %v", image, err)
		}
		if Application.PrimaryWindow() != nil {
			t.Error("missing object getter should return nil")
		}
	})
}
