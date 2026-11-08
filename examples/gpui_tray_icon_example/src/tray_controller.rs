//! Owns the tray icons and is the only place that calls the tray API — the
//! counterpart of the Flutter example's `tray_controller.dart`.
//!
//! The state lives in [`TrayController`] inside the [`TrayExample`] entity;
//! the `impl TrayExample` blocks below are the controller's methods, which
//! take the entity's context so they can notify and spawn loops. nativeapi
//! calls its listeners on the platform thread with no GPUI context, so tray
//! and menu events are queued through a channel and applied from a GPUI
//! task, the same bridge `nativeapi_gpui::observe_window_events` builds.

use std::time::{Duration, Instant};

use base64::Engine as _;
use gpui::{Context, Task};
use nativeapi::menu::{Menu, MenuBackend};
use nativeapi::tray_icon::ListenerId;
use nativeapi::tray_icon::{ContextMenuTrigger, TrayIcon, TrayIconEvent};
use nativeapi::tray_manager::TrayManager;
use nativeapi::window::{WindowEvent, WindowId};

use crate::checklist::{self as checks, CheckStatus, Checklist};
use crate::context_menu::{MenuMsg, TrayMenu};
use crate::example::{Tab, TrayExample};
use crate::icon_animations::{IconAnimation, Rgb};
use crate::icon_animator::{FrameImage, IconAnimator};
use crate::icon_tint::tint_png;
use crate::signs::sign_controller::SignController;
use crate::signs::sign_style::SignStyle;
use crate::signs::SIGNS_SUPPORTED;

/// The bundled tray icon, embedded: no working-directory-dependent asset
/// path. macOS treats it as a template icon for light and dark menu bars.
const ASSET_ICON: &[u8] = include_bytes!("../images/tray_icon.png");
const DEFAULT_TOOLTIP: &str = "nativeapi tray icon";

/// A 36 px diamond, embedded to show `Image::from_base64` with a literal.
const DIAMOND_PNG_BASE64: &str = "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAAfklEQVR42u3YywrAIAxEUf//p9NNt4ViHj1QB2Yn4UrUJK519FPFbQqGgIoHUzCfQMVLUzAjULFpCqYFKoo8BrOzth2mHSoTuByqImAZVOXu0rE68p+KyQFxKSMPNXntyYeRLB1kcSXbD7JBI1tYssknxyByUCRHafKz4WhUF0LqioTWhQw+AAAAAElFTkSuQmCC";

/// What a listener tells the controller. Routed through a channel because
/// nativeapi fires its listeners on the platform thread.
#[derive(Clone, Debug)]
pub enum TrayMsg {
    Tray { number: u32, kind: TrayEventKind },
    Menu { number: u32, msg: MenuMsg },
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum TrayEventKind {
    Clicked,
    RightClicked,
    DoubleClicked,
}

/// Where a still icon comes from: a bundled file, a canvas drawing, or a
/// base64 literal.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum StillIcon {
    Asset,
    Drawn,
    Base64,
}

impl StillIcon {
    pub fn label(self) -> &'static str {
        match self {
            StillIcon::Asset => "Asset",
            StillIcon::Drawn => "Drawn",
            StillIcon::Base64 => "Base64",
        }
    }

    pub const ALL: [StillIcon; 3] = [StillIcon::Asset, StillIcon::Drawn, StillIcon::Base64];
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum Scene {
    Download,
    Recording,
    Syncing,
}

impl Scene {
    pub fn label(self) -> &'static str {
        match self {
            Scene::Download => "Download",
            Scene::Recording => "Recording",
            Scene::Syncing => "Syncing",
        }
    }
}

/// One tray icon together with everything the example tracks about it.
pub struct TrayEntry {
    /// 1-based number shown in the UI ("#1"); not the native id.
    pub number: u32,
    pub tray_icon: TrayIcon,
    pub menu: Option<TrayMenu>,
    pub animator: IconAnimator,
    pub is_sign: bool,
    pub sign: Option<SignController>,
    listener: Option<ListenerId>,
    pub clicks: u32,
    pub right_clicks: u32,
    pub double_clicks: u32,
    pub still: Option<StillIcon>,
    pub scene: Option<Scene>,
    /// `None` means Auto; a fixed colour must never become Auto by chance.
    pub selected_color: Option<Rgb>,
    image_revision: u64,
    /// The running frame loop, and the run it renders; dropping stops it.
    animation_task: Option<Task<()>>,
    animation_run: u64,
    animation_busy: bool,
}

impl TrayEntry {
    pub fn label(&self) -> String {
        format!(
            "{} #{}",
            if self.is_sign { "Sign" } else { "Icon" },
            self.number
        )
    }

    pub fn description(&self) -> String {
        if let Some(sign) = &self.sign {
            return sign.display_title();
        }
        if let Some(scene) = self.scene {
            return format!("{} scene", scene.label());
        }
        if let Some(animation) = self.animator.animation {
            return animation.label().to_string();
        }
        format!(
            "{} icon",
            self.still.map(|still| still.label()).unwrap_or("Still")
        )
    }
}

/// The controller's state. The methods are on `TrayExample` (in the impl
/// blocks below) because they need the entity's context.
pub struct TrayController {
    pub checklist: Checklist,
    pub log: Vec<String>,
    pub supported: bool,
    pub manager_count: usize,
    /// Headline of the event footer: the last tray or menu event.
    pub last_event: String,
    pub selected: Option<u32>,
    pub menu_backend: MenuBackend,
    /// Popup behaviour, the way a tray utility has it: a click on the icon
    /// shows the window (and hides it again), losing the focus hides it.
    /// `TRAY_POPUP=1` in the environment turns it on from the start.
    pub popup_mode: bool,
    pub entries: Vec<TrayEntry>,
    pub(crate) next_number: u32,
    /// Half-ticks of the 250 ms loop, for the signs' 500 ms poll.
    pub(crate) tick: u32,
    pub(crate) popup_hidden_at: Instant,
    // Bookkeeping for the "which trigger opened the menu" check.
    pub(crate) last_tray_event: Option<ContextMenuTrigger>,
    pub(crate) last_tray_event_at: Option<Instant>,
    pub(crate) last_open_call_at: Option<Instant>,
    /// The menu that should close itself (`closing` false), or whose closed
    /// event is still outstanding (`closing` true), and when.
    pub(crate) close_watch: Option<(u32, Instant, bool)>,
}

impl TrayController {
    pub fn selected_entry(&self) -> Option<&TrayEntry> {
        self.entries
            .iter()
            .find(|entry| Some(entry.number) == self.selected)
    }
}

// Platform notes, as in the Flutter example ----------------------------------

/// Custom tray signs are available on macOS.
pub fn signs_supported() -> bool {
    SIGNS_SUPPORTED
}

/// Windows tray icons have no title: core's set_title is a no-op there and
/// get_title returns nothing, so the title is left out of the read-back
/// check.
pub fn title_supported() -> bool {
    !cfg!(target_os = "windows")
}

/// Linux tray icons are StatusNotifierItems: the shell owns the icon, so
/// there is no geometry to ask for and nothing the app can pop up itself —
/// core's get_bounds is empty and open_context_menu returns false there.
pub fn bounds_supported() -> bool {
    !cfg!(target_os = "linux")
}

pub fn open_menu_supported() -> bool {
    !cfg!(target_os = "linux")
}

/// Auto follows the shell; macOS delegates the final tint to the menu bar.
pub fn auto_color() -> Rgb {
    [0, 0, 0]
}

impl TrayExample {
    // -----------------------------------------------------------------------
    // Icons
    // -----------------------------------------------------------------------

    pub fn add_icon(
        &mut self,
        sign_style: Option<SignStyle>,
        cx: &mut Context<Self>,
    ) -> Option<u32> {
        if sign_style.is_some() && !signs_supported() {
            return None;
        }
        let Some(tray_icon) = TrayIcon::new() else {
            self.controller
                .checklist
                .fail(checks::CREATE, "create returned null");
            self.log_line("TrayIcon::new() returned None".to_string());
            return None;
        };
        tray_icon.set_icon_template(true);
        let number = self.controller.next_number;
        self.controller.next_number += 1;
        let is_sign = sign_style.is_some();
        let mut entry = TrayEntry {
            number,
            tray_icon,
            menu: None,
            animator: IconAnimator::new(auto_color()),
            is_sign,
            sign: None,
            listener: None,
            clicks: 0,
            right_clicks: 0,
            double_clicks: 0,
            still: if is_sign {
                None
            } else {
                Some(StillIcon::Asset)
            },
            scene: None,
            selected_color: None,
            image_revision: 0,
            animation_task: None,
            animation_run: 0,
            animation_busy: false,
        };

        if let Some(style) = sign_style {
            entry.tray_icon.set_icon_template(false);
            entry.still = None;
            let tray_supported = TrayManager::is_supported();
            let sign = SignController::new(style, &entry.tray_icon, tray_supported);
            entry.sign = Some(sign);
        }

        entry.menu = TrayMenu::new(number, self.controller.menu_backend, is_sign, &self.msg_tx);
        entry.listener = Some(entry.tray_icon.add_listener({
            let tx = self.msg_tx.clone();
            move |event: &TrayIconEvent| {
                let kind = match &event {
                    TrayIconEvent::Clicked { .. } => TrayEventKind::Clicked,
                    TrayIconEvent::RightClicked { .. } => TrayEventKind::RightClicked,
                    TrayIconEvent::DoubleClicked { .. } => TrayEventKind::DoubleClicked,
                };
                let _ = tx.unbounded_send(TrayMsg::Tray { number, kind });
            }
        }));
        if let Some(entry_menu) = entry.menu.as_ref() {
            entry.tray_icon.set_context_menu(Some(&entry_menu.menu));
        }
        if !entry.is_sign {
            entry.tray_icon.set_tooltip(Some(DEFAULT_TOOLTIP));
        }
        entry
            .tray_icon
            .set_context_menu_trigger(ContextMenuTrigger::RightClicked);
        entry.tray_icon.set_visible(true);

        let id = entry.tray_icon.get_id();
        self.controller.entries.push(entry);
        self.controller.selected = Some(number);
        if !is_sign {
            self.set_still(StillIcon::Asset, Some(number), true, cx);
        }

        self.refresh_manager();
        self.controller
            .checklist
            .pass(checks::CREATE, &format!("id {id}"));
        let known = TrayManager::get(id).is_some();
        let manager_count = self.controller.manager_count;
        let entry_count = self.controller.entries.len();
        if known && manager_count == entry_count {
            self.controller
                .checklist
                .pass(checks::MANAGED, &format!("{manager_count} in getAll"));
        } else {
            self.controller.checklist.fail(
                checks::MANAGED,
                &format!(
                    "get {}, getAll {manager_count}/{entry_count}",
                    if known { "ok" } else { "null" }
                ),
            );
        }
        self.log_line(format!("create #{number} → id {id}"));
        cx.notify();
        Some(number)
    }

    pub fn add_sign(&mut self, style: SignStyle, cx: &mut Context<Self>) -> Option<u32> {
        let number = self.add_icon(Some(style), cx)?;
        self.tab = Tab::Content;
        cx.notify();
        Some(number)
    }

    pub fn add_scene(&mut self, scene: Scene, cx: &mut Context<Self>) {
        if let Some(number) = self.add_icon(None, cx) {
            self.play_scene(scene, Some(number), cx);
        }
        self.tab = Tab::Content;
        cx.notify();
    }

    pub fn add_three_icons(&mut self, cx: &mut Context<Self>) {
        for animation in [
            IconAnimation::Spinner,
            IconAnimation::Pulse,
            IconAnimation::Wave,
        ] {
            if let Some(number) = self.add_icon(None, cx) {
                self.play(animation, Some(number), true, cx);
            }
        }
        self.tab = Tab::Content;
        cx.notify();
    }

    fn dispose_entry(&mut self, index: usize) {
        let entry = &mut self.controller.entries[index];
        entry.animation_task = None;
        entry.animation_run += 1;
        if let Some(listener) = entry.listener.take() {
            entry.tray_icon.remove_listener(listener);
        }
        if let Some(sign) = entry.sign.as_mut() {
            sign.release_preview(false);
        }
        entry.tray_icon.set_context_menu(None::<&Menu>);
        entry.tray_icon.set_visible(false);
        drop(entry.menu.take());
        if let Some(sign) = entry.sign.as_mut() {
            sign.release_tray(&entry.tray_icon);
        }
        // The tray icon and its sign go last, after everything attached to
        // them is gone.
        let entry = self.controller.entries.remove(index);
        drop(entry.tray_icon);
        drop(entry.sign);
    }

    pub fn remove_icon(&mut self, number: u32, cx: &mut Context<Self>) {
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| entry.number == number)
        else {
            return;
        };
        self.dispose_entry(index);
        let controller = &mut self.controller;
        if controller.selected == Some(number) {
            controller.selected = if controller.entries.is_empty() {
                None
            } else {
                Some(controller.entries[index.min(controller.entries.len() - 1)].number)
            };
        }
        self.refresh_manager();
        self.log_line(format!("dispose #{number}"));
        cx.notify();
    }

    pub fn select(&mut self, number: u32, cx: &mut Context<Self>) {
        if self
            .controller
            .entries
            .iter()
            .any(|entry| entry.number == number)
        {
            self.controller.selected = Some(number);
            cx.notify();
        }
    }

    /// Re-reads everything that is shown from native getters.
    pub fn refresh(&mut self, cx: &mut Context<Self>) {
        self.refresh_manager();
        if bounds_supported() {
            if let Some(entry) = self.controller.selected_entry() {
                let bounds = entry.tray_icon.get_bounds();
                if bounds.width <= 0. || bounds.height <= 0. {
                    self.controller.checklist.fail(checks::BOUNDS, "empty");
                } else {
                    self.controller.checklist.pass(
                        checks::BOUNDS,
                        &format!("{}×{}", bounds.width.round(), bounds.height.round()),
                    );
                }
            }
        }
        cx.notify();
    }

    pub fn dispose_icons(&mut self) {
        while !self.controller.entries.is_empty() {
            self.dispose_entry(0);
        }
        self.controller.selected = None;
    }

    fn refresh_manager(&mut self) {
        self.controller.manager_count = TrayManager::get_all().len();
    }

    // -----------------------------------------------------------------------
    // Icon image: animations and stills
    // -----------------------------------------------------------------------

    pub fn play(
        &mut self,
        animation: IconAnimation,
        number: Option<u32>,
        quiet: bool,
        cx: &mut Context<Self>,
    ) {
        let number = number.or(self.controller.selected);
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == number && !entry.is_sign)
        else {
            return;
        };
        entry.image_revision += 1;
        if !quiet {
            entry.scene = None;
        }
        entry.still = None;
        entry.animator.animation = Some(animation);
        entry.animator.paused = false;
        entry.animator.time = 0.;
        entry.animation_busy = false;
        let fps = entry.animator.fps;
        self.start_animation(number.unwrap(), cx);
        if !quiet {
            let number = number.unwrap_or_default();
            self.log_line(format!("animate #{number} {} {fps} fps", animation.label()));
        }
        cx.notify();
    }

    pub fn set_still(
        &mut self,
        kind: StillIcon,
        number: Option<u32>,
        quiet: bool,
        cx: &mut Context<Self>,
    ) {
        let number = number.or(self.controller.selected);
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| Some(entry.number) == number && !entry.is_sign)
        else {
            return;
        };
        let revision = {
            let entry = &mut self.controller.entries[index];
            entry.image_revision += 1;
            if !quiet {
                entry.scene = None;
            }
            entry.animation_task = None;
            entry.animation_run += 1;
            entry.animation_busy = false;
            entry.animator.animation = None;
            entry.animator.paused = false;
            entry.still = Some(kind);
            entry.image_revision
        };

        let png = {
            let entry = &mut self.controller.entries[index];
            match kind {
                StillIcon::Asset => tint_png(ASSET_ICON, entry.animator.color)
                    .unwrap_or_else(|| ASSET_ICON.to_vec()),
                StillIcon::Drawn => {
                    let (_, png) = entry.animator.render_star();
                    png
                }
                StillIcon::Base64 => base64::engine::general_purpose::STANDARD
                    .decode(DIAMOND_PNG_BASE64)
                    .unwrap_or_default(),
            }
        };
        if !self.apply_still(index, revision, kind, png) {
            return;
        }
        if !quiet {
            let number = number.unwrap_or_default();
            self.log_line(format!("icon #{number} ← {}", kind.label().to_lowercase()));
        }
        cx.notify();
    }

    /// The shared tail of `set_still`: hand the PNG to the tray and keep the
    /// same pixels for the preview, unless a newer image superseded them.
    fn apply_still(&mut self, index: usize, revision: u64, kind: StillIcon, png: Vec<u8>) -> bool {
        let entry = &mut self.controller.entries[index];
        if entry.still != Some(kind) || revision != entry.image_revision {
            return false;
        }
        let number = entry.number;
        let Some(image) = image_from_png(&png) else {
            self.log_line(format!(
                "icon #{number} {}: image failed to load",
                kind.label().to_lowercase()
            ));
            return false;
        };
        entry.tray_icon.set_icon(Some(&image));
        drop(image);
        if let Some(frame) = frame_from_png(&png) {
            entry.animator.show_still(frame);
        }
        true
    }

    pub fn set_fps(&mut self, fps: u32, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number)
        else {
            return;
        };
        entry.animator.fps = fps;
        let playing = entry.animator.is_playing();
        if playing {
            self.start_animation(number, cx);
        }
        self.log_line(format!("rate #{number} ← {fps} fps"));
        cx.notify();
    }

    pub fn set_scale(&mut self, scale: u32, cx: &mut Context<Self>) {
        let selected = self.controller.selected;
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| Some(entry.number) == selected && !entry.is_sign)
        else {
            return;
        };
        let (number, pixels, still) = {
            let entry = &mut self.controller.entries[index];
            entry.animator.scale = scale;
            (entry.number, entry.animator.pixel_size(), entry.still)
        };
        self.log_line(format!("resolution #{number} ← {pixels} px"));
        if let Some(still) = still {
            self.set_still(still, Some(number), true, cx);
        }
        cx.notify();
    }

    pub fn set_color(&mut self, color: Option<Rgb>, cx: &mut Context<Self>) {
        let selected = self.controller.selected;
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| Some(entry.number) == selected && !entry.is_sign)
        else {
            return;
        };
        let still = {
            let entry = &mut self.controller.entries[index];
            // A picked colour only shows on macOS once the icon stops being a
            // template.
            entry.selected_color = color;
            entry.tray_icon.set_icon_template(color.is_none());
            entry.animator.color = color.unwrap_or_else(auto_color);
            entry.still
        };
        if let Some(still) = still {
            self.set_still(still, selected, true, cx);
        }
        cx.notify();
    }

    pub fn toggle_paused(&mut self, cx: &mut Context<Self>) {
        if let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected)
        {
            if entry.animator.is_playing() {
                entry.animator.paused = !entry.animator.paused;
                cx.notify();
            }
        }
    }

    /// Renders exactly one more frame while paused.
    pub fn step_animation(&mut self, cx: &mut Context<Self>) {
        let selected = self.controller.selected;
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| Some(entry.number) == selected && !entry.is_sign)
        else {
            return;
        };
        let entry = &mut self.controller.entries[index];
        if !entry.animator.is_playing() {
            return;
        }
        entry.animator.paused = true;
        let run = entry.animation_run;
        let _number = entry.number;
        self.render_frame(index, run, cx);
        cx.notify();
    }

    // -----------------------------------------------------------------------
    // The frame loop
    // -----------------------------------------------------------------------

    /// Starts (or restarts) the frame loop of `number` at its animator's
    /// rate. A new run stops the previous one by its revision check.
    fn start_animation(&mut self, number: u32, cx: &mut Context<Self>) {
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| entry.number == number)
        else {
            return;
        };
        let run = self.controller.entries[index].animation_run + 1;
        self.controller.entries[index].animation_run = run;
        self.controller.entries[index].animator.begin_run();
        let task = cx.spawn(async move |this, cx| {
            // Deadline scheduling: each tick waits until its slot, so the
            // per-frame scheduling overhead cannot drift the rate down.
            let started = Instant::now();
            let mut slot = 0u32;
            loop {
                let interval = this
                    .update(cx, |this, _| {
                        this.controller
                            .entries
                            .iter()
                            .find(|entry| entry.number == number && entry.animation_run == run)
                            .map(|entry| Duration::from_secs_f64(1. / entry.animator.fps as f64))
                    })
                    .ok()
                    .flatten();
                let Some(interval) = interval else {
                    return;
                };
                slot += 1;
                let wait = (started + interval * slot).saturating_duration_since(Instant::now());
                cx.background_executor().timer(wait).await;

                // The frame is drawn and applied inside the same update, as
                // the Flutter animator draws on its UI thread: a hop through
                // the background executor costs more than drawing a 36 px
                // tray icon does.
                let _ = this.update(cx, |this, cx| {
                    if let Some((animation, time, ink, pixels)) = this.begin_frame(number, run, cx)
                    {
                        let mut pixmap =
                            tiny_skia::Pixmap::new(pixels, pixels).expect("tray icons are tiny");
                        animation.paint(&mut pixmap, time, ink);
                        let png = pixmap.encode_png().expect("PNG encoding cannot fail");
                        let frame = FrameImage::new(&pixmap);
                        this.finish_frame(
                            number,
                            run,
                            frame,
                            data_url_of(&png),
                            Instant::now(),
                            cx,
                        );
                    }
                });
            }
        });
        if let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number)
        {
            entry.animation_task = Some(task);
        }
    }

    /// One tick of the loop: decide whether a frame is due. `None` waits for
    /// the next tick (paused, or nothing to render).
    fn begin_frame(
        &mut self,
        number: u32,
        run: u64,
        _cx: &mut Context<Self>,
    ) -> Option<(IconAnimation, f64, Rgb, u32)> {
        let entry = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number && entry.animation_run == run)?;
        if entry.animator.paused || entry.animation_busy {
            if entry.animation_busy {
                // The previous frame is still being rendered/encoded: skip
                // this slot.
                entry.animator.note_dropped();
            }
            return None;
        }
        entry.animation_busy = true;
        // Time advances by 1/fps per frame rather than with the wall clock,
        // so Pause and Step behave and frames are reproducible.
        entry.animator.time += 1. / entry.animator.fps as f64;
        let animation = entry.animator.animation?;
        Some((
            animation,
            entry.animator.time,
            entry.animator.color,
            entry.animator.pixel_size(),
        ))
    }

    fn finish_frame(
        &mut self,
        number: u32,
        run: u64,
        frame: FrameImage,
        data_url: String,
        started: Instant,
        cx: &mut Context<Self>,
    ) {
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number && entry.animation_run == run)
        else {
            return;
        };
        entry.animation_busy = false;
        if let Some(image) = nativeapi::image::Image::from_base64(&data_url) {
            entry.tray_icon.set_icon(Some(&image));
            drop(image);
        }
        let render_ms = started.elapsed().as_secs_f64() * 1000.;
        let now_us = now_micros();
        let milestone = {
            entry.animator.last_frame = Some(frame);
            entry.animator.note_frame(render_ms, now_us);
            entry.animator.milestone.take()
        };
        let target_fps = entry.animator.fps;
        let selected = self.controller.selected == Some(number);
        if let Some((measured, dropped)) = milestone {
            self.on_frame_milestone(measured, dropped, target_fps);
        }
        // Only the selected icon's frame is on screen; a redraw per frame
        // for the background icons would multiply the window's redraws by
        // the number of animations.
        if selected {
            cx.notify();
        }
    }

    /// Renders one frame inline (Pause's Step).
    fn render_frame(&mut self, index: usize, run: u64, cx: &mut Context<Self>) {
        let entry = &mut self.controller.entries[index];
        if entry.animation_run != run {
            return;
        }
        entry.animator.time += 1. / entry.animator.fps as f64;
        let Some(animation) = entry.animator.animation else {
            return;
        };
        let number = entry.number;
        let time = entry.animator.time;
        let ink = entry.animator.color;
        let pixels = entry.animator.pixel_size();
        let (frame, data_url) = {
            let mut pixmap = tiny_skia::Pixmap::new(pixels, pixels).expect("tray icons are tiny");
            animation.paint(&mut pixmap, time, ink);
            let png = pixmap.encode_png().expect("PNG encoding cannot fail");
            (FrameImage::new(&pixmap), data_url_of(&png))
        };
        self.finish_frame(number, run, frame, data_url, Instant::now(), cx);
    }

    fn on_frame_milestone(&mut self, measured: f64, dropped: u32, target_fps: u32) {
        let detail = format!("{measured:.1} fps, {dropped} dropped");
        let on_target = measured >= target_fps as f64 * 0.9;
        let already_passed = self
            .controller
            .checklist
            .get(checks::FRAMES)
            .map(|item| item.status)
            == Some(CheckStatus::Pass);
        if on_target && dropped == 0 {
            self.controller.checklist.pass(checks::FRAMES, &detail);
        } else if !already_passed {
            self.controller.checklist.fail(checks::FRAMES, &detail);
        }
    }

    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------

    pub fn set_title(&mut self, title: Option<&str>, cx: &mut Context<Self>) {
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected && !entry.is_sign)
        else {
            return;
        };
        entry.scene = None;
        entry.tray_icon.set_title(title);
        let number = entry.number;
        let read_back = if title_supported() {
            Some(entry.tray_icon.get_title())
        } else {
            None
        };
        self.log_line(format!("setTitle({}) #{number}", quote(title)));
        if let Some(read) = read_back {
            self.check_read_back("title", title, read.as_deref());
        }
        cx.notify();
    }

    pub fn set_tooltip(&mut self, tooltip: Option<&str>, cx: &mut Context<Self>) {
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected)
        else {
            return;
        };
        entry.tray_icon.set_tooltip(tooltip);
        let number = entry.number;
        let read_back = entry.tray_icon.get_tooltip();
        self.log_line(format!("setTooltip({}) #{number}", quote(tooltip)));
        self.check_read_back("tooltip", tooltip, read_back.as_deref());
        cx.notify();
    }

    fn check_read_back(&mut self, what: &str, written: Option<&str>, read: Option<&str>) {
        // A cleared value may read back as null or as an empty string — and
        // proves nothing, so only a real value counts towards the item.
        if written.unwrap_or("") == read.unwrap_or("") {
            if !written.unwrap_or("").is_empty() {
                let total = if title_supported() { 2 } else { 1 };
                self.controller
                    .checklist
                    .part(checks::READ_BACK, what, total);
            }
        } else {
            self.controller.checklist.fail(
                checks::READ_BACK,
                &format!("{}: wrote {}, read {}", what, quote(written), quote(read)),
            );
        }
    }

    pub fn set_visible(&mut self, visible: bool, cx: &mut Context<Self>) {
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected)
        else {
            return;
        };
        let result = entry.tray_icon.set_visible(visible);
        let read = entry.tray_icon.is_visible();
        let number = entry.number;
        self.log_line(format!("setVisible({visible}) #{number} → {result}"));
        if read == visible {
            self.controller
                .checklist
                .part(checks::VISIBLE, &visible.to_string(), 2);
        } else {
            self.controller
                .checklist
                .fail(checks::VISIBLE, &format!("set {visible}, isVisible {read}"));
        }
        cx.notify();
    }

    pub fn set_trigger(&mut self, trigger: ContextMenuTrigger, cx: &mut Context<Self>) {
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected)
        else {
            return;
        };
        entry.tray_icon.set_context_menu_trigger(trigger);
        let number = entry.number;
        self.log_line(format!(
            "setContextMenuTrigger({}) #{number}",
            trigger_name(trigger)
        ));
        cx.notify();
    }

    pub fn set_menu_backend(&mut self, backend: MenuBackend, cx: &mut Context<Self>) {
        let mut failed = None;
        for entry in &self.controller.entries {
            let ok = entry
                .menu
                .as_ref()
                .map(|menu| menu.menu.set_backend(backend))
                .unwrap_or(true);
            if !ok {
                failed = Some(entry.number);
                break;
            }
        }
        if let Some(number) = failed {
            self.log_line(format!("setBackend({backend:?}) #{number} → false"));
            cx.notify();
            return;
        }
        self.controller.menu_backend = backend;
        self.log_line(format!("menu backend ← {backend:?}"));
        cx.notify();
    }

    /// Opens the menu; with `close_after` also closes it again from code. A
    /// button cannot do that — the window gets no clicks while a menu is
    /// open.
    pub fn open_menu(&mut self, close_after: Option<Duration>, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self
            .controller
            .entries
            .iter()
            .find(|entry| entry.number == number)
        else {
            return;
        };
        if let Some(close_after) = close_after {
            self.controller.close_watch = Some((number, Instant::now() + close_after, false));
        }
        self.controller.last_open_call_at = Some(Instant::now());
        let opened = entry.tray_icon.open_context_menu();
        self.log_line(format!("openContextMenu() #{number} → {opened}"));
        if opened {
            self.controller.checklist.pass(checks::OPEN_MENU, "");
        } else {
            self.controller
                .checklist
                .fail(checks::OPEN_MENU, "returned false");
        }
        cx.notify();
    }

    /// The close watch's first deadline: close it from code, then wait for
    /// the closed event (the second deadline).
    fn close_menu_now(&mut self, number: u32, cx: &mut Context<Self>) {
        let Some(entry) = self.controller.selected_entry() else {
            return;
        };
        let was_open = entry
            .menu
            .as_ref()
            .map(|menu| menu.is_open.get())
            .unwrap_or(false);
        if !was_open {
            // Dismissed by hand before the watch fired: nothing to judge.
            cx.notify();
            return;
        }
        let closed = entry.tray_icon.close_context_menu();
        let still_open = self
            .controller
            .entries
            .iter()
            .find(|entry| entry.number == number)
            .and_then(|entry| entry.menu.as_ref())
            .map(|menu| menu.is_open.get())
            .unwrap_or(false);
        self.log_line(format!("closeContextMenu() #{number} → {closed}"));
        if !still_open {
            // The closed event arrived while close_context_menu() was
            // running.
            self.controller
                .checklist
                .pass(checks::CLOSE_MENU, "closed event received");
        } else {
            self.controller.close_watch =
                Some((number, Instant::now() + Duration::from_millis(1500), true));
        }
        cx.notify();
    }

    /// Moves the window next to the selected icon, the way a tray popup
    /// would: below it where the tray is at the top of the screen (macOS,
    /// GNOME), above it where the tray is at the bottom (Windows), centred
    /// on the icon and kept inside the display's work area.
    pub fn move_window_to_icon(&mut self, cx: &mut Context<Self>) {
        let Some(entry) = self.controller.selected_entry() else {
            return;
        };
        let Some(window) = self.own_window() else {
            return;
        };
        let icon = entry.tray_icon.get_bounds();
        let number = entry.number;
        if icon.width <= 0. || icon.height <= 0. {
            self.log_line(format!("window to icon #{number}: getBounds is empty"));
            cx.notify();
            return;
        }
        let displays = nativeapi::display_manager::DisplayManager::get_all();
        let icon_center = (icon.x + icon.width / 2., icon.y + icon.height / 2.);
        let primary = nativeapi::display_manager::DisplayManager::get_primary();
        let display = displays
            .iter()
            .position(|display| {
                let position = display.position();
                let size = display.size();
                icon_center.0 >= position.x
                    && icon_center.0 <= position.x + size.width
                    && icon_center.1 >= position.y
                    && icon_center.1 <= position.y + size.height
            })
            .map(|index| &displays[index])
            .or(primary.as_ref());
        let Some(display) = display else {
            return;
        };
        let screen_position = display.position();
        let screen_size = display.size();
        let area = display.work_area();
        let size = window.size();
        let gap = 8.;
        let below = icon_center.1 < screen_position.y + screen_size.height / 2.;
        let clamp = |value: f64, min: f64, max: f64| value.max(min).min(max);
        let x = clamp(
            icon_center.0 - size.width / 2.,
            area.x + gap,
            area.x + area.width - size.width - gap,
        );
        let y = if below {
            clamp(
                icon.y + icon.height + gap,
                area.y + gap,
                area.y + area.height - size.height,
            )
        } else {
            clamp(
                icon.y - gap - size.height,
                area.y,
                area.y + area.height - size.height - gap,
            )
        };
        window.set_position(&nativeapi::geometry::Point { x, y });
        self.log_line(format!(
            "window to icon #{number}: {} {},{}",
            if below { "below" } else { "above" },
            x.round(),
            y.round()
        ));
        cx.notify();
    }

    pub fn reset_counters(&mut self, cx: &mut Context<Self>) {
        if let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == self.controller.selected)
        {
            entry.clicks = 0;
            entry.right_clicks = 0;
            entry.double_clicks = 0;
            cx.notify();
        }
    }

    // -----------------------------------------------------------------------
    // Popup: click shows, blur hides
    // -----------------------------------------------------------------------

    pub fn set_popup_mode(&mut self, on: bool, cx: &mut Context<Self>) {
        if self.controller.popup_mode == on {
            return;
        }
        self.controller.popup_mode = on;
        self.apply_popup_window_style(cx);
        self.log_line(format!("popup mode ← {}", if on { "on" } else { "off" }));
        cx.notify();
    }

    /// A popup has no title bar. On Linux that also keeps every show a
    /// single layout: a decorated GTK window maps with its CSD shadow
    /// margins and drops them when the compositor's states arrive — a
    /// visible "grow in". Without the header bar there is nothing to drop.
    pub(crate) fn apply_popup_window_style(&mut self, cx: &mut Context<Self>) {
        let Some(window) = self.own_window() else {
            return;
        };
        let style = if self.controller.popup_mode {
            nativeapi::window::TitleBarStyle::Hidden
        } else {
            nativeapi::window::TitleBarStyle::Normal
        };
        nativeapi_gpui::defer_native(cx, move || window.set_title_bar_style(style));
    }

    /// A click on the icon shows the window, or hides it when it is showing.
    /// A click that arrives right after a blur hid the window is the same
    /// gesture (the press took the focus away first), so it leaves the
    /// window hidden.
    fn toggle_popup(&mut self, cx: &mut Context<Self>) {
        let Some(window) = self.own_window() else {
            return;
        };
        if window.is_visible() {
            nativeapi_gpui::defer_native(cx, move || window.hide());
            self.controller.popup_hidden_at = Instant::now();
            self.popup("hidden (click)");
        } else if self.controller.popup_hidden_at.elapsed() < Duration::from_millis(400) {
            self.popup("stays hidden");
        } else {
            drop(window);
            if let Some(focus) = self.own_window() {
                nativeapi_gpui::defer_native(cx, move || {
                    focus.show();
                    focus.focus();
                });
            }
            self.popup("shown");
        }
    }

    // Also on stdout, where a script can read it without the UI probe.
    fn popup(&mut self, what: &str) {
        self.log_line(format!("popup {what}"));
        println!("[popup] {what}");
    }

    // -----------------------------------------------------------------------
    // Scenes: one click, a complete picture for the camera
    // -----------------------------------------------------------------------

    pub fn play_scene(&mut self, scene: Scene, number: Option<u32>, cx: &mut Context<Self>) {
        let number = number.or(self.controller.selected);
        let Some(index) = self
            .controller
            .entries
            .iter()
            .position(|entry| Some(entry.number) == number && !entry.is_sign)
        else {
            return;
        };
        let number = self.controller.entries[index].number;
        let tooltip = match scene {
            Scene::Download => Some("Downloading nativeapi.zip".to_string()),
            Scene::Recording => Some("Recording — click to stop".to_string()),
            Scene::Syncing => Some("Syncing 3 folders".to_string()),
        };
        if scene == Scene::Syncing {
            self.controller.entries[index]
                .tray_icon
                .set_title(Some("Syncing…"));
        }
        self.controller.entries[index].scene = Some(scene);
        let animation = match scene {
            Scene::Download => IconAnimation::Progress,
            Scene::Recording => IconAnimation::Blink,
            Scene::Syncing => IconAnimation::Spinner,
        };
        self.play(animation, Some(number), true, cx);
        if let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number)
        {
            entry.tray_icon.set_tooltip(tooltip.as_deref());
        }
        self.log_line(format!(
            "scene #{number} ← {}",
            scene.label().to_lowercase()
        ));
        self.drive_scenes(cx);
    }

    /// Back to the asset icon, no title, default tooltip.
    pub fn reset_scene(&mut self, cx: &mut Context<Self>) {
        let number = self.controller.selected;
        let Some(entry) = self
            .controller
            .entries
            .iter_mut()
            .find(|entry| Some(entry.number) == number && !entry.is_sign)
        else {
            return;
        };
        entry.scene = None;
        entry.tray_icon.set_title(None);
        entry.tray_icon.set_tooltip(Some(DEFAULT_TOOLTIP));
        let number = entry.number;
        self.set_still(StillIcon::Asset, Some(number), true, cx);
        self.log_line(format!("scene #{number} ← default"));
        cx.notify();
    }

    /// Three icons, three different animations, all running together.
    pub fn play_three_at_once(&mut self, cx: &mut Context<Self>) {
        let mut numbers: Vec<u32> = self
            .controller
            .entries
            .iter()
            .filter(|entry| !entry.is_sign)
            .map(|entry| entry.number)
            .collect();
        while numbers.len() < 3 {
            match self.add_icon(None, cx) {
                Some(number) => numbers.push(number),
                None => return,
            }
        }
        let animations = [
            IconAnimation::Spinner,
            IconAnimation::Pulse,
            IconAnimation::Wave,
        ];
        for (i, number) in numbers.iter().take(3).enumerate() {
            if let Some(entry) = self
                .controller
                .entries
                .iter_mut()
                .find(|entry| entry.number == *number)
            {
                entry.scene = None;
            }
            self.play(animations[i], Some(*number), true, cx);
        }
        self.controller.selected = Some(numbers[0]);
        self.log_line("scene ← three icons".to_string());
        cx.notify();
    }

    /// Keeps scene titles in step with the animation they belong to; called
    /// from the 250 ms tick.
    fn drive_scenes(&mut self, cx: &mut Context<Self>) {
        let mut changed = false;
        for entry in &mut self.controller.entries {
            let t = entry.animator.time;
            let title = match entry.scene {
                Some(Scene::Download) => Some(format!(
                    "{}%",
                    (IconAnimation::progress_at(t) * 100.).round() as u32
                )),
                Some(Scene::Recording) => {
                    let total = t.floor() as u64;
                    Some(format!("{:02}:{:02}", total / 60, total % 60))
                }
                _ => None,
            };
            if let Some(title) = title {
                if entry.tray_icon.get_title().as_deref() != Some(title.as_str()) {
                    entry.tray_icon.set_title(Some(&title));
                    changed = true;
                }
            }
        }
        if changed {
            cx.notify();
        }
    }

    // -----------------------------------------------------------------------
    // Events (routed through the channel by the listeners)
    // -----------------------------------------------------------------------

    pub(crate) fn on_msg(&mut self, msg: TrayMsg, cx: &mut Context<Self>) {
        match msg {
            TrayMsg::Tray { number, kind } => self.on_tray_event(number, kind, cx),
            TrayMsg::Menu { number, msg } => self.on_menu_msg(number, msg, cx),
        }
    }

    fn on_tray_event(&mut self, number: u32, kind: TrayEventKind, cx: &mut Context<Self>) {
        let is_sign = self
            .controller
            .entries
            .iter()
            .find(|entry| entry.number == number)
            .map(|entry| entry.is_sign)
            .unwrap_or(false);
        match kind {
            TrayEventKind::Clicked => {
                if let Some(entry) = self.entry_mut(number) {
                    entry.clicks += 1;
                }
                self.controller.checklist.count(checks::CLICKED);
                self.note_tray_event(ContextMenuTrigger::Clicked);
                self.event("Clicked", number);
                if is_sign {
                    self.show_sign_settings(number, cx);
                } else if self.controller.popup_mode {
                    self.controller.selected = Some(number);
                    self.tab = Tab::Content;
                    self.toggle_popup(cx);
                }
            }
            TrayEventKind::RightClicked => {
                if let Some(entry) = self.entry_mut(number) {
                    entry.right_clicks += 1;
                }
                self.controller.checklist.count(checks::RIGHT_CLICKED);
                self.note_tray_event(ContextMenuTrigger::RightClicked);
                self.event("Right clicked", number);
            }
            TrayEventKind::DoubleClicked => {
                if let Some(entry) = self.entry_mut(number) {
                    entry.double_clicks += 1;
                }
                self.controller.checklist.count(checks::DOUBLE_CLICKED);
                self.note_tray_event(ContextMenuTrigger::DoubleClicked);
                self.event("Double clicked", number);
            }
        }
        cx.notify();
    }

    fn entry_mut(&mut self, number: u32) -> Option<&mut TrayEntry> {
        self.controller
            .entries
            .iter_mut()
            .find(|entry| entry.number == number)
    }

    fn show_sign_settings(&mut self, _number: u32, cx: &mut Context<Self>) {
        let Some(window) = self.own_window() else {
            return;
        };
        // Showing and focusing the window makes the platform call back into
        // GPUI synchronously; from inside an update that would re-enter a
        // borrowed context, so it is deferred.
        nativeapi_gpui::defer_native(cx, move || {
            window.show();
            window.focus();
        });
        cx.notify();
    }

    fn note_tray_event(&mut self, trigger: ContextMenuTrigger) {
        self.controller.last_tray_event = Some(trigger);
        self.controller.last_tray_event_at = Some(Instant::now());
    }

    fn on_menu_msg(&mut self, number: u32, msg: MenuMsg, cx: &mut Context<Self>) {
        match msg {
            MenuMsg::Opened => {
                if let Some(menu) = self.menu_of(number) {
                    menu.is_open.set(true);
                }
                self.event("Menu opened", number);
                // Credit the trigger that is configured, if its event just
                // happened.
                let now = Instant::now();
                let recent = |at: Option<Instant>| {
                    at.is_some_and(|at| now.duration_since(at).as_millis() < 1500)
                };
                // Where the app cannot open the menu itself, "none" has no
                // way to pass.
                let total = if open_menu_supported() { 4 } else { 3 };
                let trigger = self
                    .entry_of(number)
                    .map(|entry| entry.tray_icon.get_context_menu_trigger());
                if let Some(trigger) = trigger {
                    if trigger == ContextMenuTrigger::None {
                        if recent(self.controller.last_open_call_at) {
                            self.controller.checklist.part(
                                checks::TRIGGERS,
                                trigger_name(trigger),
                                total,
                            );
                        }
                    } else if Some(trigger) == self.controller.last_tray_event
                        && recent(self.controller.last_tray_event_at)
                    {
                        self.controller.checklist.part(
                            checks::TRIGGERS,
                            trigger_name(trigger),
                            total,
                        );
                    }
                }
            }
            MenuMsg::Closed => {
                if let Some(menu) = self.menu_of(number) {
                    menu.is_open.set(false);
                }
                self.event("Menu closed", number);
                self.controller.checklist.count(checks::MENU_OPEN_CLOSE);
                if let Some((watched, _, true)) = self.controller.close_watch {
                    if watched == number {
                        self.controller.close_watch = None;
                        self.controller
                            .checklist
                            .pass(checks::CLOSE_MENU, "closed event received");
                    }
                }
            }
            MenuMsg::SubmenuOpened => {
                self.controller
                    .checklist
                    .part(checks::MENU_ITEMS, "submenu", 3);
                self.event("Submenu opened", number);
            }
            MenuMsg::ShowWindow => {
                self.controller
                    .checklist
                    .part(checks::MENU_ITEMS, "item", 3);
                self.event("Menu item \"Show window\"", number);
                self.select(number, cx);
                self.tab = Tab::Content;
                if let Some(window) = self.own_window() {
                    nativeapi_gpui::defer_native(cx, move || {
                        window.show();
                        window.focus();
                    });
                }
            }
            MenuMsg::EnlargePreview => {
                self.event("Menu item \"Enlarge preview\"", number);
                self.enlarge_preview(number, cx);
            }
            MenuMsg::Animation(animation) => {
                self.event(
                    &format!(
                        "Menu item \"{}\"",
                        animation.map(IconAnimation::label).unwrap_or("Stop")
                    ),
                    number,
                );
                self.controller
                    .checklist
                    .part(checks::MENU_ITEMS, "item", 3);
                match animation {
                    Some(animation) => self.play(animation, Some(number), false, cx),
                    None => self.set_still(StillIcon::Asset, Some(number), false, cx),
                }
            }
            MenuMsg::Checkbox(checked) => {
                self.controller
                    .checklist
                    .part(checks::MENU_ITEMS, "checkbox", 3);
                self.event(
                    &format!("Checkbox {}", if checked { "checked" } else { "unchecked" }),
                    number,
                );
            }
            MenuMsg::About => {
                self.controller
                    .checklist
                    .part(checks::MENU_ITEMS, "item", 3);
                self.event("Menu item \"About\"", number);
            }
            MenuMsg::Quit => {
                self.dispose_icons();
                cx.quit();
            }
        }
        cx.notify();
    }

    fn entry_of(&self, number: u32) -> Option<&TrayEntry> {
        self.controller
            .entries
            .iter()
            .find(|entry| entry.number == number)
    }

    fn menu_of(&self, number: u32) -> Option<&TrayMenu> {
        self.entry_of(number).and_then(|entry| entry.menu.as_ref())
    }

    pub(crate) fn enlarge_preview(&mut self, number: u32, cx: &mut Context<Self>) {
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        let Some(sign) = sign.as_mut() else {
            return;
        };
        // The preview window draws the sign's own native views; the Flutter
        // example additionally tunes their fonts through its macOS runner.
        sign.show_preview(tray_icon);
        cx.notify();
    }

    // -----------------------------------------------------------------------
    // Window events
    // -----------------------------------------------------------------------

    pub(crate) fn on_window_event(&mut self, event: WindowEvent, cx: &mut Context<Self>) {
        match &event {
            WindowEvent::Blurred { window_id } => {
                if !self.controller.popup_mode {
                    return;
                }
                let Some(window) = self.own_window() else {
                    return;
                };
                if *window_id != window.id() || !window.is_visible() {
                    return;
                }
                if let Some(hide) = self.own_window() {
                    nativeapi_gpui::defer_native(cx, move || hide.hide());
                }
                self.controller.popup_hidden_at = Instant::now();
                self.popup("hidden (blur)");
            }
            WindowEvent::CloseRequested { window_id, request } => {
                let own = self.own_window_id == Some(*window_id);
                if !own {
                    // A sign's preview window closes on its own.
                    self.release_preview_of(*window_id);
                    return;
                }
                if !self.controller.entries.is_empty() && request.with(|r| r.is_cancelable()) {
                    request.with(|r| {
                        r.cancel();
                    });
                    if let Some(window) = self.own_window() {
                        nativeapi_gpui::defer_native(cx, move || window.hide());
                    }
                }
            }
            WindowEvent::Closed { window_id } => {
                self.release_preview_of(*window_id);
            }
            _ => {}
        }
    }

    fn release_preview_of(&mut self, window_id: WindowId) {
        for entry in &mut self.controller.entries {
            if let Some(sign) = entry.sign.as_mut() {
                if sign.on_window_closed(window_id, false) {
                    break;
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // The 250 ms tick
    // -----------------------------------------------------------------------

    pub(crate) fn on_tick(&mut self, cx: &mut Context<Self>) {
        self.controller.tick += 1;
        self.drive_scenes(cx);
        // Signs settle their menu-bar height after placement: every other
        // tick, read it back and rebuild when it changed.
        if self.controller.tick.is_multiple_of(2) {
            for index in 0..self.controller.entries.len() {
                let entry = &mut self.controller.entries[index];
                let TrayEntry {
                    sign, tray_icon, ..
                } = entry;
                let Some(sign) = sign.as_mut() else {
                    continue;
                };
                let height = tray_icon.get_bounds().height;
                if height > 0. && sign.settle_tray_height(height) {
                    sign.update(tray_icon);
                }
            }
        }
        // The close watch: close the menu when asked, fail the item when the
        // closed event never comes.
        if let Some((number, deadline, closing)) = self.controller.close_watch {
            if Instant::now() >= deadline {
                self.controller.close_watch = None;
                if closing {
                    let still_open = self
                        .menu_of(number)
                        .map(|menu| menu.is_open.get())
                        .unwrap_or(false);
                    if still_open {
                        self.controller
                            .checklist
                            .fail(checks::CLOSE_MENU, "no closed event");
                    }
                } else {
                    self.close_menu_now(number, cx);
                }
            }
        }
        cx.notify();
    }

    // -----------------------------------------------------------------------
    // Sign settings
    // -----------------------------------------------------------------------

    /// Loads the selected sign's content into the draft fields whenever the
    /// selection or the style moves on.
    pub(crate) fn sync_sign_draft(&mut self) {
        let key = self.controller.selected_entry().map(|entry| {
            (
                entry.number,
                entry
                    .sign
                    .as_ref()
                    .map(|sign| sign.style)
                    .unwrap_or(SignStyle::Missing),
            )
        });
        if key == self.sign_draft_for {
            return;
        }
        self.sign_draft_for = key;
        self.sign_error = None;
        self.sign_draft = self
            .controller
            .selected_entry()
            .and_then(|entry| entry.sign.as_ref())
            .map(|sign| {
                sign.content()
                    .values()
                    .iter()
                    .map(|v| v.to_string())
                    .collect()
            })
            .unwrap_or_default();
    }

    pub(crate) fn select_sign_style(&mut self, index: usize, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        let Some(sign) = sign.as_mut() else {
            return;
        };
        sign.select_style(index, tray_icon);
        self.sign_draft_for = None;
        cx.notify();
    }

    pub(crate) fn edit_sign_field(&mut self, index: usize, text: String) {
        if index < self.sign_draft.len() {
            self.sign_draft[index] = text;
        } else {
            self.sign_draft.resize(index + 1, String::new());
            self.sign_draft[index] = text;
        }
    }

    /// Invalid input leaves the current sign intact and reports why.
    pub(crate) fn apply_sign(&mut self, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let draft: [String; 4] = [
            self.sign_draft.first().cloned().unwrap_or_default(),
            self.sign_draft.get(1).cloned().unwrap_or_default(),
            self.sign_draft.get(2).cloned().unwrap_or_default(),
            self.sign_draft.get(3).cloned().unwrap_or_default(),
        ];
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        let Some(sign) = sign.as_mut() else {
            return;
        };
        self.sign_error = sign.set_content(&draft[0], &draft[1], &draft[2], &draft[3], tray_icon);
        cx.notify();
    }

    pub(crate) fn reset_sign(&mut self, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        let Some(sign) = sign.as_mut() else {
            return;
        };
        sign.restore_defaults(tray_icon);
        self.sign_draft_for = None;
        cx.notify();
    }

    pub(crate) fn set_sign_green(&mut self, green: bool, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        if let Some(sign) = sign.as_mut() {
            sign.set_green(green, tray_icon);
        }
        cx.notify();
    }

    pub(crate) fn set_sign_right(&mut self, right: bool, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        if let Some(sign) = sign.as_mut() {
            sign.set_right(right, tray_icon);
        }
        cx.notify();
    }

    pub(crate) fn set_sign_english(&mut self, english: bool, cx: &mut Context<Self>) {
        let Some(number) = self.controller.selected else {
            return;
        };
        let Some(entry) = self.entry_mut(number) else {
            return;
        };
        let TrayEntry {
            sign, tray_icon, ..
        } = entry;
        if let Some(sign) = sign.as_mut() {
            sign.set_english(english, tray_icon);
        }
        cx.notify();
    }

    // -----------------------------------------------------------------------
    // Log
    // -----------------------------------------------------------------------

    fn event(&mut self, what: &str, number: u32) {
        self.controller.last_event = format!("{what} · #{number}");
        self.log_line(format!("{} #{number}", what.to_lowercase()));
    }

    pub fn clear_log(&mut self, cx: &mut Context<Self>) {
        self.controller.log.clear();
        self.controller.last_event = "No events yet".to_string();
        cx.notify();
    }

    pub(crate) fn log_line(&mut self, message: String) {
        let secs = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .map(|d| d.as_secs())
            .unwrap_or(0);
        let day = secs % 86_400;
        let time = format!("{:02}:{:02}:{:02}", day / 3600, day % 3600 / 60, day % 60);
        self.controller.log.insert(0, format!("{time} {message}"));
        if self.controller.log.len() > 200 {
            self.controller.log.pop();
        }
    }
}

// Helpers ---------------------------------------------------------------------

/// The data-URL form `Image::from_base64` takes.
pub fn data_url_of(png: &[u8]) -> String {
    let encoded = base64::engine::general_purpose::STANDARD.encode(png);
    format!("data:image/png;base64,{encoded}")
}

/// `Image::from_base64` takes the data-URL form the Flutter example passes.
pub fn image_from_png(png: &[u8]) -> Option<nativeapi::image::Image> {
    let encoded = base64::engine::general_purpose::STANDARD.encode(png);
    nativeapi::image::Image::from_base64(&format!("data:image/png;base64,{encoded}"))
}

/// The preview's pixels: the same PNG the tray received, decoded.
pub fn frame_from_png(png: &[u8]) -> Option<FrameImage> {
    let rgba = image::load_from_memory(png).ok()?.into_rgba8();
    let (width, height) = (rgba.width(), rgba.height());
    let raw = rgba.into_raw();
    let mut data = raw.clone();
    for pixel in data.as_chunks_mut::<4>().0 {
        pixel.swap(0, 2);
    }
    let buffer = image::RgbaImage::from_raw(width, height, data)?;
    Some(FrameImage::from_parts(raw, buffer))
}

fn quote(value: Option<&str>) -> String {
    match value {
        None => "null".to_string(),
        Some(value) => format!("\"{}\"", value.replace('\n', "\\n")),
    }
}

fn trigger_name(trigger: ContextMenuTrigger) -> &'static str {
    match trigger {
        ContextMenuTrigger::None => "none",
        ContextMenuTrigger::Clicked => "clicked",
        ContextMenuTrigger::RightClicked => "rightClicked",
        ContextMenuTrigger::DoubleClicked => "doubleClicked",
    }
}

fn now_micros() -> u64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_micros() as u64)
        .unwrap_or(0)
}
