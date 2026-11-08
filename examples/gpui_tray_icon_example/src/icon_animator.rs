//! Renders [`IconAnimation`] frames and hands each one to a tray icon, the
//! counterpart of the Flutter example's `icon_animator.dart`.
//!
//! Every frame goes tiny-skia pixmap → PNG → nativeapi `Image` →
//! `TrayIcon::set_icon`. The same pixels are kept in `last_frame` (as a GPUI
//! image too), so what the in-window preview shows is exactly what the tray
//! received.

use std::sync::Arc;

use gpui::RenderImage;
use tiny_skia::Pixmap;

use crate::icon_animations::{self as animations, IconAnimation, Rgb};

/// Tray icons are laid out at this many points; `scale` decides how many
/// pixels are rendered into them.
pub const ICON_POINTS: f32 = 18.;

/// Called once per run when this many frames have been pushed.
pub const MILESTONE_FRAMES: u64 = 100;

/// The frame most recently pushed to the tray, or the current still icon:
/// the raw RGBA (a template's alpha mask, tinted for the preview) and the
/// same pixels as the GPUI image.
pub struct FrameImage {
    pub size: u32,
    /// The tray's own pixels, RGBA.
    pub rgba: Arc<Vec<u8>>,
    /// The same pixels as the GPUI image the preview draws, BGRA.
    pub bgra: Arc<RenderImage>,
}

impl FrameImage {
    pub fn new(pixmap: &Pixmap) -> Self {
        let size = pixmap.width();
        // GPUI expects BGRA (see gpui's own image loading); swap in place.
        let mut data = pixmap.data().to_vec();
        for pixel in data.as_chunks_mut::<4>().0 {
            pixel.swap(0, 2);
        }
        let buffer = image::RgbaImage::from_raw(size, pixmap.height(), data)
            .expect("frame buffer matches its dimensions");
        FrameImage::from_parts(pixmap.data().to_vec(), buffer)
    }

    /// From an RGBA buffer already laid out for GPUI (a decoded still icon's
    /// pixels, swapped to BGRA).
    pub fn from_parts(rgba: Vec<u8>, bgra: image::RgbaImage) -> Self {
        let size = bgra.width();
        FrameImage {
            size,
            rgba: Arc::new(rgba),
            bgra: Arc::new(RenderImage::new(smallvec::smallvec![image::Frame::new(
                bgra
            )])),
        }
    }

    /// The frame's mask painted in `ink` — what the shell does with a
    /// template icon, done here so the preview shows the same shape in the
    /// window's ink colour.
    pub fn tinted(&self, ink: Rgb) -> Arc<RenderImage> {
        let mut data = self.rgba.as_ref().clone();
        for pixel in data.as_chunks_mut::<4>().0 {
            pixel[0] = ink[2]; // BGRA
            pixel[1] = ink[1];
            pixel[2] = ink[0];
        }
        let buffer = image::RgbaImage::from_raw(self.size, self.size, data)
            .expect("frame buffer matches its dimensions");
        Arc::new(RenderImage::new(smallvec::smallvec![image::Frame::new(
            buffer
        )]))
    }
}

/// The state of one tray icon's animation. The frame loop that drives it
/// lives in the controller, which owns the tray icon and the GPUI context;
/// time advances by 1/fps per rendered frame rather than with the wall
/// clock, so Pause and Step behave and frames are reproducible.
pub struct IconAnimator {
    pub animation: Option<IconAnimation>,
    pub fps: u32,
    /// Pixels per point: 1, 2 or 3.
    pub scale: u32,
    pub color: Rgb,
    pub paused: bool,

    // Stats of the current run.
    pub frames: u64,
    pub dropped: u32,
    pub render_ms: f64,
    pub measured_fps: f64,
    /// Set once per run when [`MILESTONE_FRAMES`] frames have been pushed;
    /// the controller reads and clears it.
    pub milestone: Option<(f64, u32)>,
    /// Whether this run's milestone has been reported already.
    milestone_reported: bool,

    /// Animation time in seconds.
    pub time: f64,
    /// Timestamps (µs) of the last ≤30 frames, for the measured rate.
    stamps: VecDeque<u64>,

    /// The frame most recently pushed to the tray (or the current still).
    pub last_frame: Option<FrameImage>,
}

use std::collections::VecDeque;

impl IconAnimator {
    pub fn new(color: Rgb) -> Self {
        IconAnimator {
            animation: None,
            fps: 30,
            scale: 2,
            color,
            paused: false,
            frames: 0,
            dropped: 0,
            render_ms: 0.,
            measured_fps: 0.,
            milestone: None,
            milestone_reported: false,
            time: 0.,
            stamps: VecDeque::new(),
            last_frame: None,
        }
    }

    pub fn pixel_size(&self) -> u32 {
        (ICON_POINTS * self.scale as f32).round() as u32
    }

    pub fn is_playing(&self) -> bool {
        self.animation.is_some()
    }

    /// Starts a run's counters over (a new animation, or a new rate or
    /// resolution restarting the current one).
    pub fn begin_run(&mut self) {
        self.frames = 0;
        self.dropped = 0;
        self.measured_fps = 0.;
        self.milestone = None;
        self.milestone_reported = false;
        self.stamps.clear();
    }

    /// A frame slot the renderer could not take: the previous frame was
    /// still being rendered or encoded.
    pub fn note_dropped(&mut self) {
        self.dropped += 1;
    }

    /// Records a pushed frame: `render_ms` is everything from deciding the
    /// frame to the tray call returning, `now_us` the wall clock.
    pub fn note_frame(&mut self, render_ms: f64, now_us: u64) {
        self.render_ms = render_ms;
        self.frames += 1;
        self.stamps.push_back(now_us);
        if self.stamps.len() > 30 {
            self.stamps.pop_front();
        }
        if self.stamps.len() > 1 {
            let first = *self.stamps.front().unwrap();
            let last = *self.stamps.back().unwrap();
            if last > first {
                self.measured_fps =
                    (self.stamps.len() - 1) as f64 * 1_000_000. / (last - first) as f64;
            }
        }
        if !self.milestone_reported && self.frames >= MILESTONE_FRAMES {
            self.milestone_reported = true;
            self.milestone = Some((self.measured_fps, self.dropped));
        }
    }

    /// Shows a still image in the preview (the caller sets the tray icon).
    pub fn show_still(&mut self, frame: FrameImage) {
        self.last_frame = Some(frame);
    }

    /// Renders the frame for `time` at the current resolution and colour,
    /// returning the preview image and the PNG bytes for the tray.
    #[allow(dead_code)]
    pub fn render(&self, time: f64) -> (FrameImage, Vec<u8>) {
        let Some(animation) = self.animation else {
            unreachable!("render without an animation");
        };
        let pixels = self.pixel_size();
        let mut pixmap = Pixmap::new(pixels, pixels).expect("tray icons are tiny");
        animation.paint(&mut pixmap, time, self.color);
        let png = pixmap.encode_png().expect("PNG encoding cannot fail");
        (FrameImage::new(&pixmap), png)
    }

    /// The star still: drawn on a pixmap, encoded as PNG.
    pub fn render_star(&self) -> (FrameImage, Vec<u8>) {
        let pixels = self.pixel_size();
        let mut pixmap = Pixmap::new(pixels, pixels).expect("tray icons are tiny");
        animations::paint_star(&mut pixmap, self.color);
        let png = pixmap.encode_png().expect("PNG encoding cannot fail");
        (FrameImage::new(&pixmap), png)
    }
}
