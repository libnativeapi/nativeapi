package main

import (
	_ "embed"
	"encoding/base64"
	"flag"
	"fmt"
	"log"
	"runtime"
	"strings"
	"time"

	"github.com/egoist/mygo"
	"github.com/egoist/mygo/ui"
	. "github.com/libnativeapi/nativeapi/bindings/go"
)

//go:embed icon.png
var iconPNG []byte

type demo struct {
	win       *mygo.Window
	tray      *TrayIcon
	menu      *Menu
	items     []*MenuItem
	listeners []func()
	increment *MenuItem
	pause     *MenuItem
	count     int
	paused    bool
	events    []string
}

func (d *demo) record(message string) {
	log.Println(message)
	d.events = append([]string{message}, d.events...)
	if len(d.events) > 8 {
		d.events = d.events[:8]
	}
	if d.win != nil {
		d.win.Invalidate()
	}
}

func (d *demo) addItem(label string, kind MenuItemType, action func()) (*MenuItem, error) {
	item, err := NewMenuItem(label, kind)
	if err != nil {
		return nil, err
	}
	d.items = append(d.items, item)
	id, err := item.AddListener(func(event MenuEvent) {
		if event.Type == MenuEventItemClicked {
			action()
		}
	})
	if err != nil {
		return nil, err
	}
	d.listeners = append(d.listeners, func() { _ = item.RemoveListener(id) })
	d.menu.AddItem(item)
	return item, nil
}

func (d *demo) addCount() {
	if d.paused {
		return
	}
	d.count++
	title := fmt.Sprintf("%d", d.count)
	d.tray.SetTitle(&title)
	tooltip := fmt.Sprintf("MyGo + nativeapi — Counter: %d", d.count)
	d.tray.SetTooltip(&tooltip)
	d.record(fmt.Sprintf("Counter: %d", d.count))
}

func (d *demo) togglePause() {
	d.paused = !d.paused
	state := MenuItemStateUnchecked
	if d.paused {
		state = MenuItemStateChecked
	}
	d.pause.SetState(state)
	d.increment.SetEnabled(!d.paused)
	d.record(fmt.Sprintf("Paused: %v", d.paused))
}

func (d *demo) showWindow() {
	d.win.Show()
	d.win.Focus()
	d.record("Show window")
}

func (d *demo) setup() error {
	if !TrayManager.IsSupported() {
		return fmt.Errorf("nativeapi tray icons are not supported")
	}
	var err error
	d.tray, err = NewTrayIconWithIdentifier("dev.libnativeapi.mygo.tray")
	if err != nil {
		return err
	}
	icon, err := ImageFromBase64(base64.StdEncoding.EncodeToString(iconPNG))
	if err != nil {
		return err
	}
	defer icon.Release()
	d.tray.SetIcon(icon)
	d.tray.SetIconTemplate(runtime.GOOS == "darwin")
	d.tray.SetIconSize(Size{Width: 18, Height: 18})
	tooltip := "MyGo + nativeapi: left-click to show, right-click for menu"
	d.tray.SetTooltip(&tooltip)
	title := "0"
	d.tray.SetTitle(&title)
	d.menu, err = NewMenu()
	if err != nil {
		return err
	}
	if _, err = d.addItem("Show window", MenuItemTypeNormal, d.showWindow); err != nil {
		return err
	}
	d.menu.AddSeparator()
	d.increment, err = d.addItem("Increment counter", MenuItemTypeNormal, d.addCount)
	if err != nil {
		return err
	}
	d.pause, err = d.addItem("Pause counter", MenuItemTypeCheckbox, d.togglePause)
	if err != nil {
		return err
	}
	d.menu.AddSeparator()
	if _, err = d.addItem("Quit", MenuItemTypeNormal, mygo.App.Quit); err != nil {
		return err
	}
	d.tray.SetContextMenu(d.menu)
	d.tray.SetContextMenuTrigger(ContextMenuTriggerRightClicked)
	id, err := d.tray.AddListener(func(event TrayIconEvent) {
		switch event.Type {
		case TrayIconEventClicked, TrayIconEventDoubleClicked:
			d.showWindow()
		case TrayIconEventRightClicked:
			d.record("Tray right-click")
		}
	})
	if err != nil {
		return err
	}
	d.listeners = append(d.listeners, func() { _ = d.tray.RemoveListener(id) })
	if err := d.tray.SetVisible(true); err != nil {
		return err
	}
	d.record("Tray ready")
	return nil
}

func (d *demo) close() {
	for i := len(d.listeners) - 1; i >= 0; i-- {
		d.listeners[i]()
	}
	if d.tray != nil {
		_ = d.tray.SetVisible(false)
		d.tray.SetContextMenu(nil)
		d.tray.Release()
	}
	d.menu.Release()
	for _, item := range d.items {
		item.Release()
	}
}

func (d *demo) view(c *ui.Context) {
	ui.Column(c).Fill().Padding(24).Gap(16).Children(func() {
		ui.Text(c, "Tray icon").FontSize(28).Bold()
		ui.Text(c, "MyGo window + nativeapi tray and native menu").TextColor(c.Theme().TextMuted).Wrap()
		ui.Text(c, fmt.Sprint(d.count)).FontSize(48).Bold()
		ui.Row(c).Gap(8).Children(func() {
			if ui.PrimaryButton(c, "Increment").Disabled(d.paused).Clicked() {
				d.addCount()
			}
			if ui.Button(c, "Pause / resume").Clicked() {
				d.togglePause()
			}
			if ui.Button(c, "Hide window").Clicked() {
				d.win.Hide()
				d.record("Window hidden; use the tray to show it again")
			}
		})
		ui.Text(c, fmt.Sprintf("Paused: %v", d.paused))
		ui.Text(c, "Left-click the tray icon to restore this window. Right-click for its native menu. Closing the window quits.").Wrap()
		ui.Text(c, "Events").Bold()
		ui.Scroll(c).Grow(1).Children(func() {
			ui.Text(c, strings.Join(d.events, "\n")).Wrap()
		})
	})
}

func (d *demo) check() error {
	if !d.win.IsVisible() || !d.tray.IsVisible() || d.menu.ItemCount() != 6 {
		return fmt.Errorf("window, tray or menu is not ready")
	}
	if d.tray.Identifier() != "dev.libnativeapi.mygo.tray" {
		return fmt.Errorf("tray identifier did not persist")
	}
	d.addCount()
	title := d.tray.Title()
	if runtime.GOOS != "windows" && (title == nil || *title != "1") {
		return fmt.Errorf("tray counter title did not update")
	}
	tooltip := d.tray.Tooltip()
	if tooltip == nil || !strings.Contains(*tooltip, "Counter: 1") {
		return fmt.Errorf("tray counter tooltip did not update")
	}
	d.togglePause()
	if d.pause.State() != MenuItemStateChecked || d.increment.IsEnabled() {
		return fmt.Errorf("native menu state did not update")
	}
	d.togglePause()
	d.win.Hide()
	if d.win.IsVisible() {
		return fmt.Errorf("MyGo window did not hide")
	}
	d.showWindow()
	if !d.win.IsVisible() {
		return fmt.Errorf("MyGo window did not restore")
	}
	if err := d.tray.SetVisible(false); err != nil {
		return err
	}
	if d.tray.IsVisible() {
		return fmt.Errorf("tray did not hide")
	}
	return d.tray.SetVisible(true)
}

func run(smoke bool) error {
	d := &demo{}
	defer d.close()
	var result error
	mygo.App.WhenReady(func() {
		// MyGo owns the OS event loop. Its ready callback and ui.View run on
		// the main thread, which nativeapi also uses for native objects/events.
		Init()
		if result = d.setup(); result != nil {
			mygo.App.Quit()
			return
		}
		d.win = mygo.NewWindow(mygo.WindowOptions{
			Title: "MyGo + nativeapi — Tray", Width: 680, Height: 560,
			MinWidth: 560, MinHeight: 460,
			Content: ui.View(d.view),
		})
		if smoke {
			time.AfterFunc(time.Second, func() {
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
	smoke := flag.Bool("smoke", false, "check the live tray and menu, then quit")
	flag.Parse()
	if err := run(*smoke); err != nil {
		log.Fatal(err)
	}
	if *smoke {
		log.Println("PASS: MyGo window, nativeapi tray, counter and menu state")
	}
}
