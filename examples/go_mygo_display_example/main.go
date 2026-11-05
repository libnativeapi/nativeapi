package main

import (
	"flag"
	"fmt"
	"log"
	"runtime"
	"time"

	"github.com/egoist/mygo"
	"github.com/egoist/mygo/ui"
	. "github.com/libnativeapi/nativeapi/bindings/go"
)

type displayInfo struct {
	id      DisplayID
	name    string
	pos     Point
	size    Size
	work    Rectangle
	scale   float64
	primary bool
}

type demo struct {
	win      *mygo.Window
	displays []displayInfo
	cursor   Point
	status   string
}

func (d *demo) refresh() {
	d.displays = nil
	for _, display := range DisplayManager.All() {
		d.displays = append(d.displays, displayInfo{
			id: display.ID(), name: display.Name(), pos: display.Position(),
			size: display.Size(), work: display.WorkArea(),
			scale: display.ScaleFactor(), primary: display.IsPrimary(),
		})
		// Keep copied values in the UI, releasing every handle from All().
		display.Release()
	}
	d.cursor = DisplayManager.CursorPosition()
	if d.win != nil {
		d.win.Invalidate()
	}
}

func (d *demo) view(c *ui.Context) {
	ui.Column(c).Fill().Padding(24).Gap(16).Children(func() {
		ui.Text(c, "Displays").FontSize(28).Bold()
		ui.Text(c, "MyGo UI + nativeapi DisplayManager").TextColor(c.Theme().TextMuted)
		ui.Row(c).Gap(16).Children(func() {
			if ui.PrimaryButton(c, "Refresh").Clicked() {
				d.refresh()
				d.status = "Refreshed displays and cursor position"
			}
			ui.Text(c, fmt.Sprintf("Cursor: %.0f, %.0f", d.cursor.X, d.cursor.Y))
		})
		ui.Text(c, fmt.Sprintf("%d display(s) · %s", len(d.displays), d.status)).Wrap()
		ui.Scroll(c).Grow(1).Children(func() {
			ui.Column(c).Gap(12).Children(func() {
				for _, display := range d.displays {
					ui.Column(c).Padding(16).Gap(8).Border(1, c.Theme().Border).Radius(8).Children(func() {
						ui.Text(c, fmt.Sprintf("%s (#%d)", display.name, display.id)).FontSize(20).Bold()
						ui.Text(c, fmt.Sprintf("Size: %.0f × %.0f · Scale: %.2f · Primary: %v",
							display.size.Width, display.size.Height, display.scale, display.primary)).Wrap()
						ui.Text(c, fmt.Sprintf("Position: %.0f, %.0f", display.pos.X, display.pos.Y))
						ui.Text(c, fmt.Sprintf("Work area: %.0f, %.0f · %.0f × %.0f",
							display.work.X, display.work.Y, display.work.Width, display.work.Height)).Wrap()
					})
				}
			})
		})
	})
}

func (d *demo) check() error {
	d.refresh()
	if !d.win.IsVisible() || len(d.displays) == 0 {
		return fmt.Errorf("window is not visible or no displays were found")
	}
	for _, display := range d.displays {
		if display.size.Width <= 0 || display.size.Height <= 0 || display.scale <= 0 {
			return fmt.Errorf("invalid geometry for display #%d", display.id)
		}
	}
	return nil
}

func run(smoke bool) error {
	d := &demo{status: "Ready"}
	var result error
	var removeListener func()
	defer func() {
		if removeListener != nil {
			removeListener()
		}
	}()
	mygo.App.WhenReady(func() {
		Init()
		d.refresh()
		id, err := DisplayManager.AddListener(func(event DisplayEvent) {
			// event.Display is borrowed; refresh obtains its own short-lived handles.
			d.status = fmt.Sprintf("Display event: %d", event.Type)
			d.refresh()
		})
		if err != nil {
			result = err
			mygo.App.Quit()
			return
		}
		removeListener = func() { _ = DisplayManager.RemoveListener(id) }
		d.win = mygo.NewWindow(mygo.WindowOptions{
			Title: "MyGo + nativeapi — Displays", Width: 700, Height: 520,
			MinWidth: 520, MinHeight: 400,
			Content: ui.View(d.view),
		})
		if smoke {
			time.AfterFunc(time.Second, func() {
				// Background work returns to MyGo's main thread for native calls.
				mygo.RunOnMain(func() {
					result = d.check()
					mygo.App.Quit()
				})
			})
		}
	})
	if err := mygo.App.Run(); err != nil {
		return err
	}
	return result
}

func main() {
	runtime.LockOSThread()
	smoke := flag.Bool("smoke", false, "check the live window and display geometry, then quit")
	flag.Parse()
	if err := run(*smoke); err != nil {
		log.Fatal(err)
	}
	if *smoke {
		log.Println("PASS: MyGo window and nativeapi display geometry")
	}
}
