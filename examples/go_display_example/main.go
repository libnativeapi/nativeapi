package main

import (
	"fmt"
	"os"
	"runtime"

	. "github.com/libnativeapi/nativeapi/bindings/go"
)

func main() {
	runtime.LockOSThread()
	Init()
	for _, display := range DisplayManager.All() {
		fmt.Printf("Display #%d: %s, size=%+v, scale=%.2f, primary=%v\n",
			display.ID(), display.Name(), display.Size(), display.ScaleFactor(), display.IsPrimary())
		display.Release()
	}
	fmt.Printf("Cursor: %+v\n", DisplayManager.CursorPosition())
	if err := PollEvents(0); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
