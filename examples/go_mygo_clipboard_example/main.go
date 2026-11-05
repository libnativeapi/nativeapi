package main

import (
	"flag"
	"fmt"
	"html"
	"log"
	"runtime"
	"time"

	"github.com/egoist/mygo"
	"github.com/egoist/mygo/ui"
	. "github.com/libnativeapi/nativeapi/bindings/go"
)

type demo struct {
	win     *mygo.Window
	text    string
	readout string
	status  string
	changes int
	busy    bool
	closed  bool
}

func (d *demo) report(err error, success string) {
	if err != nil {
		d.status = err.Error()
	} else {
		d.status = success
	}
	d.win.Invalidate()
}

func (d *demo) read(done func(error)) {
	d.busy = true
	d.status = "Reading clipboard…"
	Clipboard.Read(func(ok bool, data ClipboardData) {
		// Read() returns an owned image handle, even when we only show metadata.
		defer data.Image.Release()
		if d.closed {
			return
		}
		d.busy = false
		var err error
		if !ok {
			err = fmt.Errorf("clipboard read failed")
		} else {
			text, markup := "(none)", "(none)"
			if data.Text != nil {
				text = *data.Text
			}
			if data.HTML != nil {
				markup = *data.HTML
			}
			d.readout = fmt.Sprintf("Text:\n%s\n\nHTML:\n%s\n\nImage: %v · Files: %d",
				text, markup, data.Image != nil, len(data.FilePaths))
		}
		d.report(err, "Clipboard read complete")
		if done != nil {
			done(err)
		}
	})
}

func (d *demo) view(c *ui.Context) {
	ui.Column(c).Fill().Padding(24).Gap(16).Children(func() {
		ui.Text(c, "Clipboard").FontSize(28).Bold()
		ui.Text(c, "MyGo UI + nativeapi Clipboard").TextColor(c.Theme().TextMuted)
		ui.TextArea(c, &d.text).Height(100)
		ui.Row(c).Gap(8).Children(func() {
			if ui.PrimaryButton(c, "Copy text").Disabled(d.busy).Clicked() {
				d.report(Clipboard.WriteText(d.text), "Copied text")
			}
			if ui.Button(c, "Copy text + HTML").Disabled(d.busy).Clicked() {
				text := d.text
				markup := "<p>" + html.EscapeString(text) + "</p>"
				d.report(Clipboard.Write(ClipboardData{Text: &text, HTML: &markup}), "Copied text and HTML")
			}
			if ui.Button(c, "Read clipboard").Disabled(d.busy).Clicked() {
				d.read(nil)
			}
		})
		ui.Text(c, fmt.Sprintf("%s · Changes observed: %d", d.status, d.changes)).Wrap()
		ui.Scroll(c).Grow(1).Children(func() {
			ui.Text(c, d.readout).Wrap()
		})
	})
}

func run(smoke bool) error {
	d := &demo{
		text: "Hello from MyGo and nativeapi!", status: "Ready",
		readout: "Copy something, then click Read clipboard.",
	}
	var result error
	var removeListener func()
	defer func() {
		d.closed = true
		if removeListener != nil {
			removeListener()
		}
	}()
	mygo.App.WhenReady(func() {
		Init()
		if !Clipboard.IsSupported() {
			result = fmt.Errorf("nativeapi clipboard is not supported")
			mygo.App.Quit()
			return
		}
		if Clipboard.IsChangeMonitoringSupported() {
			id, err := Clipboard.AddListener(func(event ClipboardEvent) {
				if event.Type == ClipboardEventChanged {
					d.changes++
					if d.win != nil {
						d.win.Invalidate()
					}
				}
			})
			if err != nil {
				result = err
				mygo.App.Quit()
				return
			}
			removeListener = func() { _ = Clipboard.RemoveListener(id) }
		} else {
			d.status = "Change monitoring unavailable; read manually"
		}
		d.win = mygo.NewWindow(mygo.WindowOptions{
			Title: "MyGo + nativeapi — Clipboard", Width: 740, Height: 560,
			MinWidth: 600, MinHeight: 450,
			Content: ui.View(d.view),
		})
		if smoke {
			finished := false
			time.AfterFunc(time.Second, func() {
				mygo.RunOnMain(func() {
					// The smoke check only reads, leaving the user's clipboard intact.
					d.read(func(err error) {
						finished = true
						result = err
						if !d.win.IsVisible() {
							result = fmt.Errorf("window is not visible")
						}
						mygo.App.Quit()
					})
				})
			})
			time.AfterFunc(8*time.Second, func() {
				mygo.RunOnMain(func() {
					if !finished {
						result = fmt.Errorf("clipboard callback timed out")
						mygo.App.Quit()
					}
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
	smoke := flag.Bool("smoke", false, "check a read-only clipboard callback, then quit")
	flag.Parse()
	if err := run(*smoke); err != nil {
		log.Fatal(err)
	}
	if *smoke {
		log.Println("PASS: MyGo window and nativeapi asynchronous clipboard read")
	}
}
