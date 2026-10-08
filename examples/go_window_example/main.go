package main

import (
	"flag"
	"fmt"
	"os"
	"runtime"
	"time"

	. "github.com/libnativeapi/nativeapi/bindings/go"
)

func main() {
	// Lock before any native API call; all UI work stays on this OS thread.
	runtime.LockOSThread()
	Init()
	smoke := flag.Bool("smoke", false, "close automatically after checking the live window")
	flag.Parse()

	window, err := NewWindow()
	if err != nil {
		fmt.Fprintln(os.Stderr, "Failed to create window:", err)
		os.Exit(1)
	}
	defer window.Release()
	window.SetTitle("Go Window Example")
	window.SetContentSize(Size{Width: 640, Height: 400})
	window.Center()

	root := window.ContentView()
	if root != nil {
		defer root.Release()
		label, err := NewLabel("Hello from Go and libnativeapi")
		if err != nil {
			fmt.Fprintln(os.Stderr, "Failed to create label:", err)
		} else {
			defer label.Release()
			label.SetFrame(Rectangle{X: 24, Y: 24, Width: 560, Height: 48})
			root.AddSubview(label.View)
		}
	}

	listener, err := WindowManager.AddListener(func(event WindowEvent) {
		if event.WindowID != window.ID() {
			return
		}
		fmt.Printf("Window event: %d\n", event.Type)
		if event.Type == WindowEventClosed {
			Application.Quit(0)
		}
	})
	if err != nil {
		fmt.Fprintln(os.Stderr, "Failed to register window listener:", err)
		os.Exit(1)
	}
	defer WindowManager.RemoveListener(listener)
	if *smoke {
		go func() {
			time.Sleep(time.Second)
			if err := Dispatch(func() {
				fmt.Printf("Live window: %q, size=%+v, visible=%v\n", window.Title(), window.ContentSize(), window.IsVisible())
				if window.Title() != "Go Window Example" || !window.IsVisible() ||
					window.ContentSize() != (Size{Width: 640, Height: 400}) {
					Application.Quit(1)
					return
				}
				if err := window.Close(); err != nil {
					Application.Quit(1)
				}
			}); err != nil {
				fmt.Fprintln(os.Stderr, "Failed to dispatch smoke check:", err)
				os.Exit(1)
			}
		}()
	}
	code := Application.RunWithWindow(window)
	fmt.Printf("Event loop exited: %d\n", code)
	// Run deferred native releases before exiting with a nonzero status.
	if code != 0 {
		panic(fmt.Sprintf("application failed: %d", code))
	}
}
