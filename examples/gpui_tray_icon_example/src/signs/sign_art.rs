//! The drawn sign, the counterpart of the Flutter example's
//! `signs/sign_art.rs` (its `_SignPainter`).
//!
//! The Flutter example uses these as lightweight template thumbnails while
//! its live macOS preview embeds the real native view. GPUI cannot host
//! platform views, so here the drawn sign is the in-window preview as well —
//! same geometry, same colours, with rectangles on a canvas and the strings
//! as measured, absolutely positioned text.

use gpui::{div, point, prelude::*, px, size as px_size, App, Bounds, Hsla, SharedString, Window};
use gpui::{FontWeight, ParentElement, RenderOnce, Styled};

use super::sign_style::{SignContent, SignStyle};

fn light() -> Hsla {
    gpui::rgb(0xFAFAF5).into()
}
fn ink() -> Hsla {
    gpui::rgb(0x141E25).into()
}
fn blue() -> Hsla {
    gpui::rgb(0x184170).into()
}
fn road_green() -> Hsla {
    gpui::rgb(0x00694E).into()
}
fn brown() -> Hsla {
    gpui::rgb(0x683E24).into()
}
fn ochre() -> Hsla {
    gpui::rgb(0xE0B766).into()
}

/// The art is drawn 100 units tall and scaled to `height`.
const ART_HEIGHT: f32 = 100.;

/// A drawn sign at `height` pixels tall, as wide as the sign asks.
#[derive(IntoElement)]
pub struct SignArt {
    style: SignStyle,
    content: SignContent,
    green: bool,
    english: bool,
    right: bool,
    height: f32,
}

impl SignArt {
    pub fn new(style: SignStyle) -> Self {
        SignArt {
            style,
            content: style.defaults(),
            green: false,
            english: true,
            right: true,
            height: 24.,
        }
    }

    pub fn content(mut self, content: &SignContent) -> Self {
        self.content = content.clone();
        self
    }

    #[allow(dead_code)]
    pub fn green(mut self, green: bool) -> Self {
        self.green = green;
        self
    }

    #[allow(dead_code)]
    pub fn english(mut self, english: bool) -> Self {
        self.english = english;
        self
    }

    #[allow(dead_code)]
    pub fn right(mut self, right: bool) -> Self {
        self.right = right;
        self
    }

    pub fn height(mut self, height: f32) -> Self {
        self.height = height;
        self
    }

    fn bilingual(&self) -> bool {
        matches!(self.style, SignStyle::Missing | SignStyle::Travel) || self.english
    }

    /// The caption line under the heading.
    fn caption(&self) -> String {
        match self.style {
            SignStyle::Missing => format!(
                "WoZai{}HenXiangNi",
                self.secondary()
                    .split([' ', '-'])
                    .filter(|word| !word.is_empty())
                    .map(|word| {
                        let mut chars = word.chars();
                        match chars.next() {
                            Some(first) => {
                                first.to_uppercase().collect::<String>()
                                    + &chars.as_str().to_lowercase()
                            }
                            None => String::new(),
                        }
                    })
                    .collect::<String>()
            ),
            SignStyle::Welcome => format!("WELCOME TO {}", self.secondary().to_uppercase()),
            SignStyle::Guide => format!(
                "{}  /  {}",
                self.secondary().to_uppercase(),
                self.content.value
            ),
            SignStyle::Travel => self.secondary().to_string(),
        }
    }
}

// Field accessors that read through to the content.
impl SignArt {
    fn primary(&self) -> &str {
        &self.content.primary
    }

    fn secondary(&self) -> &str {
        &self.content.secondary
    }

    fn heading(&self) -> String {
        self.content.heading(self.style)
    }
}

impl RenderOnce for SignArt {
    fn render(self, window: &mut Window, _cx: &mut App) -> impl IntoElement {
        let scale = self.height / ART_HEIGHT;
        let weight = FontWeight::SEMIBOLD;
        let style = window.text_style();
        let measure = |text: &str, font: f32| -> f32 {
            let run = gpui::TextRun {
                len: text.len(),
                font: style.font(),
                color: gpui::black(),
                background_color: None,
                underline: Default::default(),
                strikethrough: Default::default(),
            };
            f32::from(
                window
                    .text_system()
                    .layout_line(text, px(font), &[run], None)
                    .width,
            )
        };

        let main =
            35.0 * match self.style {
                SignStyle::Welcome => 1.12,
                SignStyle::Guide => 1.1,
                _ => 1.,
            } * if self.bilingual() { 1. } else { 46. / 35. };
        let caption = self.caption();
        let side = match self.style {
            SignStyle::Missing => 35.,
            SignStyle::Travel => 60.,
            SignStyle::Guide => 70.,
            _ => 0.,
        };
        let badge = if self.style == SignStyle::Guide {
            82.
        } else {
            0.
        };
        let body = if self.style == SignStyle::Travel {
            measure(self.primary(), 28.).max(measure(&caption, 26.)) + 30.
        } else {
            measure(&self.heading(), main).max(if self.bilingual() {
                measure(&caption, 18.5)
            } else {
                0.
            }) + if self.style == SignStyle::Missing {
                12.
            } else {
                44.
            }
        };
        let width = body
            + if self.style == SignStyle::Missing {
                side * 2.
            } else {
                side + badge
                    + if self.style == SignStyle::Travel {
                        0.
                    } else {
                        8.
                    }
            };

        let color = match self.style {
            SignStyle::Guide => brown(),
            SignStyle::Travel | SignStyle::Missing if self.green => road_green(),
            SignStyle::Travel => road_green(),
            _ => blue(),
        };

        // The rectangles, in art units.
        let mut rects: Vec<(f32, f32, f32, f32, Hsla)> = vec![(0., 0., width, 100., light())];
        match self.style {
            SignStyle::Missing => {
                rects.push((0., 0., width, 68., color));
            }
            SignStyle::Travel => {
                rects.push((width * 0.47, 0., 12., 100., brown()));
                rects.push((0., 0., width, 47., color));
                rects.push((0., 53., width, 47., ochre()));
            }
            SignStyle::Welcome => {
                rects.push((4., 4., width - 8., 92., color));
            }
            SignStyle::Guide => {
                rects.push((4., 4., width - 8., 92., color));
                rects.push((14., 14., 72., 72., light()));
            }
        }

        // The strings, centred on their anchor points.
        let mut labels: Vec<(String, f32, f32, f32, Hsla)> = Vec::new();
        match self.style {
            SignStyle::Missing => {
                labels.push((self.heading(), side + body / 2., 34., main, light()));
                labels.push((
                    if self.right { "东" } else { "西" }.into(),
                    side / 2.,
                    34.,
                    main * 0.66,
                    light(),
                ));
                labels.push((
                    if self.right { "西" } else { "东" }.into(),
                    width - side / 2.,
                    34.,
                    main * 0.66,
                    light(),
                ));
                if self.bilingual() {
                    labels.push((caption, side + body / 2., 84., 18.5, ink()));
                    labels.push((
                        if self.right { "E" } else { "W" }.into(),
                        side / 2.,
                        84.,
                        18.5,
                        ink(),
                    ));
                    labels.push((
                        if self.right { "W" } else { "E" }.into(),
                        width - side / 2.,
                        84.,
                        18.5,
                        ink(),
                    ));
                }
            }
            SignStyle::Travel => {
                labels.push((self.primary().to_string(), body / 2., 23.5, 28., light()));
                labels.push((
                    if self.right { "➜" } else { "←" }.into(),
                    body + side / 2.,
                    23.5,
                    31.,
                    light(),
                ));
                labels.push((
                    if self.right { "←" } else { "➜" }.into(),
                    side / 2.,
                    76.5,
                    31.,
                    ink(),
                ));
                labels.push((
                    self.secondary().to_string(),
                    side + body / 2.,
                    76.5,
                    26.,
                    ink(),
                ));
            }
            SignStyle::Welcome | SignStyle::Guide => {
                let start = badge + if !self.right { side } else { 0. } + 4.;
                labels.push((
                    self.heading(),
                    start + body / 2.,
                    if self.bilingual() { 35. } else { 50. },
                    main,
                    light(),
                ));
                if self.bilingual() {
                    labels.push((caption, start + body / 2., 79.5, 18.5, light()));
                }
                if side > 0. {
                    labels.push((
                        if self.right { "➜" } else { "←" }.into(),
                        if self.right {
                            width - 4. - side / 2.
                        } else {
                            4. + badge + side / 2.
                        },
                        46.,
                        50.,
                        light(),
                    ));
                }
                if badge > 0. {
                    labels.push(("▲".into(), 50., 50., 34., color));
                }
            }
        }

        let rects = rects
            .into_iter()
            .map(|(x, y, w, h, fill)| (x * scale, y * scale, w * scale, h * scale, fill))
            .collect::<Vec<_>>();

        let mut sign = div()
            .relative()
            .flex_none()
            .w(px(width * scale))
            .h(px(self.height))
            .overflow_hidden()
            .child(gpui::canvas(
                move |_, _, _| (),
                move |bounds, _, window, _| {
                    for (x, y, w, h, fill) in &rects {
                        let origin = bounds.origin + point(px(*x), px(*y));
                        let quad_bounds = Bounds::new(origin, px_size(px(*w), px(*h)));
                        window.paint_quad(gpui::quad(
                            quad_bounds,
                            gpui::Corners::default(),
                            gpui::Background::from(*fill),
                            gpui::Edges::default(),
                            gpui::transparent_black(),
                            gpui::BorderStyle::default(),
                        ));
                    }
                },
            ));
        for (text, x, y, font, fill) in labels {
            let width = measure(&text, font) * scale;
            let line = font * scale * 1.3;
            sign = sign.child(
                div()
                    .absolute()
                    .left(px(x * scale - width / 2.))
                    .top(px(y * scale - line / 2.))
                    .w(px(width))
                    .flex()
                    .items_center()
                    .justify_center()
                    .text_size(px(font * scale))
                    .line_height(px(line))
                    .font_weight(weight)
                    .text_color(fill)
                    .child(SharedString::from(text)),
            );
        }
        sign
    }
}
