//! The animations the example can play on a tray icon, the counterpart of
//! the Flutter example's `icon_animations.dart` — drawn with tiny-skia
//! instead of the Flutter canvas.
//!
//! All but [`IconAnimation::Widget`] are drawn straight onto a pixmap by
//! [`IconAnimation::paint`]. `Widget` is different on purpose: in Flutter its
//! frames are screenshots of a live widget, which shows that anything Flutter
//! can lay out can end up in the tray. GPUI has no way to capture an element,
//! so here it draws the same counter badge (a ring with a bouncing digit)
//! with the same geometry — anything a CPU can draw can end up in the tray.

use tiny_skia::{
    FillRule, LineCap, LineJoin, Paint, Path, PathBuilder, Pixmap, Point as SkPoint, Stroke,
    Transform,
};

/// The ink an animation draws with; alpha is applied per shape.
pub type Rgb = [u8; 3];

/// The animations, in the order the gallery shows them.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum IconAnimation {
    Spinner,
    Pulse,
    Blink,
    Progress,
    Wave,
    Rotate,
    Clock,
    Widget,
}

pub const ALL: [IconAnimation; 8] = [
    IconAnimation::Spinner,
    IconAnimation::Pulse,
    IconAnimation::Blink,
    IconAnimation::Progress,
    IconAnimation::Wave,
    IconAnimation::Rotate,
    IconAnimation::Clock,
    IconAnimation::Widget,
];

impl IconAnimation {
    pub fn label(self) -> &'static str {
        match self {
            IconAnimation::Spinner => "Spinner",
            IconAnimation::Pulse => "Pulse",
            IconAnimation::Blink => "Blink",
            IconAnimation::Progress => "Progress",
            IconAnimation::Wave => "Wave",
            IconAnimation::Rotate => "Rotate",
            IconAnimation::Clock => "Clock",
            IconAnimation::Widget => "Any widget",
        }
    }

    /// Progress of [`IconAnimation::Progress`] at time `t`, 0..1. The
    /// Download scene reads it to keep the tray title ("42%") in step with
    /// the ring.
    pub fn progress_at(t: f64) -> f64 {
        (t % 4.0) / 4.0
    }

    /// Draws the frame for time `t` (seconds) into the square pixmap.
    pub fn paint(self, pixmap: &mut Pixmap, t: f64, ink: Rgb) {
        let size = pixmap.width() as f32;
        let center = sk(size / 2., size / 2.);
        let stroke = size * 0.13;
        let radius = size / 2. - stroke / 2. - size * 0.04;

        match self {
            IconAnimation::Spinner => {
                stroke_circle(pixmap, center, radius, ink, 0.22, stroke, false);
                let arc = arc_path(center, radius, t as f32 * TAU, PI * 0.6);
                stroke_path(pixmap, &arc, &solid(ink, 1.0), &round_stroke(stroke));
            }
            IconAnimation::Pulse => {
                let k = 0.5 - 0.5 * (t as f32 * TAU / 1.2).cos();
                let r = (size / 2. - 1.) * (0.4 + 0.6 * k);
                fill_circle(pixmap, center, r, &solid(ink, 1.0));
            }
            IconAnimation::Blink => {
                if t % 1.0 < 0.5 {
                    fill_circle(pixmap, center, size * 0.36, &solid(ink, 1.0));
                }
            }
            IconAnimation::Progress => {
                stroke_circle(pixmap, center, radius, ink, 0.22, stroke, false);
                let sweep = TAU * Self::progress_at(t) as f32;
                let arc = arc_path(center, radius, -PI / 2., sweep);
                stroke_path(pixmap, &arc, &solid(ink, 1.0), &round_stroke(stroke));
            }
            IconAnimation::Wave => {
                const BARS: usize = 4;
                let gap = size / (BARS as f32 * 2. + 1.);
                for (i, _bar) in (0..BARS).enumerate() {
                    let k = 0.5 + 0.5 * (t as f32 * TAU / 0.9 - i as f32 * 0.9).sin();
                    let h = size * (0.25 + 0.6 * k);
                    let x = gap * (1. + i as f32 * 2.);
                    // A capsule: a vertical line stroked with round caps.
                    let mut builder = PathBuilder::new();
                    builder.move_to(x, (size - h) / 2. + gap / 2.);
                    builder.line_to(x, (size + h) / 2. - gap / 2.);
                    let capsule = builder.finish().unwrap();
                    stroke_path(pixmap, &capsule, &solid(ink, 1.0), &round_stroke(gap));
                }
            }
            IconAnimation::Rotate => {
                let side = size * 0.56;
                let square = rotated_square(center, side, t as f32 * TAU / 2.4);
                let joined = Stroke {
                    width: stroke,
                    line_cap: LineCap::Butt,
                    line_join: LineJoin::Round,
                    ..Stroke::default()
                };
                stroke_path(pixmap, &square, &solid(ink, 1.0), &joined);
            }
            IconAnimation::Clock => {
                // Real data, not a loop: the hands show the wall clock.
                let now = std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .unwrap_or_default();
                let secs = now.as_secs_f64();
                let seconds = secs % 60.;
                let minutes = (secs / 60.) % 60. + seconds / 60.;
                let hours = (secs / 3600.) % 12. + minutes / 60.;
                stroke_circle(pixmap, center, radius, ink, 1.0, stroke * 0.8, false);
                let mut hand = |turns: f64, length: f32, width: f32| {
                    let a = turns as f32 * TAU - PI / 2.;
                    let tip = sk(
                        center.x + a.cos() * (radius * length),
                        center.y + a.sin() * (radius * length),
                    );
                    let mut builder = PathBuilder::new();
                    builder.move_to(center.x, center.y);
                    builder.line_to(tip.x, tip.y);
                    let hand_path = builder.finish().unwrap();
                    stroke_path(pixmap, &hand_path, &solid(ink, 1.0), &round_stroke(width));
                };
                hand(hours / 12., 0.45, stroke * 0.8);
                hand(minutes / 60., 0.7, stroke * 0.8);
                hand(seconds / 60., 0.8, stroke * 0.4);
            }
            IconAnimation::Widget => paint_counter_badge(pixmap, t, ink),
        }
    }
}

/// The widget behind [`IconAnimation::Widget`] in Flutter: a ring with a
/// digit that counts the seconds and bounces on every change. GPUI cannot
/// screenshot a widget, so the same badge is drawn here, as segments.
fn paint_counter_badge(pixmap: &mut Pixmap, t: f64, ink: Rgb) {
    let size = pixmap.width() as f32;
    let phase = (t % 1.0) as f32;
    let bounce = if phase < 0.25 {
        (phase / 0.25 * std::f32::consts::PI).sin()
    } else {
        0.
    };
    let scale = 0.84 + 0.16 * bounce;
    let center = sk(size / 2., size / 2.);

    let ring = size / 2. * scale - size * 0.05;
    stroke_circle(pixmap, center, ring, ink, 1.0, size * 0.1 * scale, true);

    // A seven-segment digit standing in for the badge's bold numeral.
    let digit = (t.floor() as i64 % 10).rem_euclid(10) as u8;
    let [top, tl, tr, mid, bl, br, bottom] = seven_segment(digit);
    let height = size * 0.5 * scale;
    let width = height * 0.55;
    let thick = height * 0.16;
    let left = center.x - width / 2.;
    let top_y = center.y - height / 2.;
    let mut bar = |x: f32, y: f32, w: f32, h: f32| {
        let Some(rect) = tiny_skia::Rect::from_xywh(x, y, w, h) else {
            return;
        };
        let mut builder = PathBuilder::new();
        builder.push_rect(rect);
        let path = builder.finish().unwrap();
        pixmap.fill_path(
            &path,
            &solid(ink, 1.0),
            FillRule::Winding,
            Transform::default(),
            None,
        );
    };
    if top {
        bar(left, top_y, width, thick);
    }
    if mid {
        bar(left, top_y + height / 2. - thick / 2., width, thick);
    }
    if bottom {
        bar(left, top_y + height - thick, width, thick);
    }
    if tl {
        bar(left, top_y, thick, height / 2.);
    }
    if tr {
        bar(left + width - thick, top_y, thick, height / 2.);
    }
    if bl {
        bar(left, top_y + height / 2., thick, height / 2.);
    }
    if br {
        bar(
            left + width - thick,
            top_y + height / 2.,
            thick,
            height / 2.,
        );
    }
}

/// Which of the seven segments each digit lights, in the order
/// top, top-left, top-right, middle, bottom-left, bottom-right, bottom.
fn seven_segment(digit: u8) -> [bool; 7] {
    match digit {
        0 => [true, true, true, false, true, true, true],
        1 => [false, false, true, false, false, true, false],
        2 => [true, false, true, true, true, false, true],
        3 => [true, false, true, true, false, true, true],
        4 => [false, true, true, true, false, true, false],
        5 => [true, true, false, true, false, true, true],
        6 => [true, true, false, true, true, true, true],
        7 => [true, false, true, false, false, true, false],
        8 => [true, true, true, true, true, true, true],
        _ => [true, true, true, true, false, true, true],
    }
}

/// A five-pointed star, the still icon that is drawn on a canvas.
pub fn paint_star(pixmap: &mut Pixmap, ink: Rgb) {
    let size = pixmap.width() as f32;
    let mut builder = PathBuilder::new();
    for i in 0..10 {
        let r = size / 2. * if i % 2 == 0 { 0.95 } else { 0.42 };
        let a = -PI / 2. + i as f32 * PI / 5.;
        let p = sk(size / 2. + r * a.cos(), size / 2. + r * a.sin());
        if i == 0 {
            builder.move_to(p.x, p.y);
        } else {
            builder.line_to(p.x, p.y);
        }
    }
    builder.close();
    let star = builder.finish().unwrap();
    pixmap.fill_path(
        &star,
        &solid(ink, 1.0),
        FillRule::Winding,
        Transform::default(),
        None,
    );
}

// tiny-skia helpers ------------------------------------------------------------

const TAU: f32 = std::f32::consts::TAU;
const PI: f32 = std::f32::consts::PI;

fn sk(x: f32, y: f32) -> SkPoint {
    SkPoint { x, y }
}

fn solid(ink: Rgb, alpha: f32) -> Paint<'static> {
    let mut paint = Paint::default();
    paint.set_color_rgba8(ink[0], ink[1], ink[2], (alpha.clamp(0., 1.) * 255.) as u8);
    paint.anti_alias = true;
    paint
}

fn round_stroke(width: f32) -> Stroke {
    Stroke {
        width,
        line_cap: LineCap::Round,
        ..Stroke::default()
    }
}

fn stroke_path(pixmap: &mut Pixmap, path: &Path, paint: &Paint, stroke: &Stroke) {
    pixmap.stroke_path(path, paint, stroke, Transform::default(), None);
}

fn fill_circle(pixmap: &mut Pixmap, center: SkPoint, radius: f32, paint: &Paint) {
    let Some(circle) = PathBuilder::from_circle(center.x, center.y, radius) else {
        return;
    };
    pixmap.fill_path(
        &circle,
        paint,
        FillRule::Winding,
        Transform::default(),
        None,
    );
}

fn stroke_circle(
    pixmap: &mut Pixmap,
    center: SkPoint,
    radius: f32,
    ink: Rgb,
    alpha: f32,
    width: f32,
    round: bool,
) {
    let Some(circle) = PathBuilder::from_circle(center.x, center.y, radius) else {
        return;
    };
    let pen = if round {
        round_stroke(width)
    } else {
        Stroke {
            width,
            ..Stroke::default()
        }
    };
    stroke_path(pixmap, &circle, &solid(ink, alpha), &pen);
}

/// tiny-skia has no arcs; a polyline is indistinguishable at tray sizes.
fn arc_path(center: SkPoint, radius: f32, start: f32, sweep: f32) -> Path {
    const STEPS: usize = 32;
    let mut builder = PathBuilder::new();
    for step in 0..=STEPS {
        let a = start + sweep * step as f32 / STEPS as f32;
        let p = sk(center.x + radius * a.cos(), center.y + radius * a.sin());
        if step == 0 {
            builder.move_to(p.x, p.y);
        } else {
            builder.line_to(p.x, p.y);
        }
    }
    builder.finish().unwrap()
}

fn rotated_square(center: SkPoint, side: f32, angle: f32) -> Path {
    let half = side / 2.;
    let mut builder = PathBuilder::new();
    for (i, (dx, dy)) in [(-1., -1.), (1., -1.), (1., 1.), (-1., 1.)]
        .iter()
        .enumerate()
    {
        let x = center.x + dx * half;
        let y = center.y + dy * half;
        let (sin, cos) = angle.sin_cos();
        let rx = center.x + (x - center.x) * cos - (y - center.y) * sin;
        let ry = center.y + (x - center.x) * sin + (y - center.y) * cos;
        if i == 0 {
            builder.move_to(rx, ry);
        } else {
            builder.line_to(rx, ry);
        }
    }
    builder.close();
    builder.finish().unwrap()
}
