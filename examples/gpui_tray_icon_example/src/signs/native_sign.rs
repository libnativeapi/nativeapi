//! Every host owns native views and labels, including badges and
//! fingerboards — the counterpart of the Flutter example's
//! `signs/native_sign.dart`. No GPUI painting or bitmap capture is involved.
//!
//! The Flutter example additionally asks its macOS runner (through a method
//! channel) to round the sign's face and retune its label fonts; nativeapi's
//! own views and labels are used as they are here.

use nativeapi as na;
use nativeapi::view::{Label, View};

use super::sign_style::{SignContent, SignStyle};

pub const SIGN_BLUE: na::color::Color = na::color::Color {
    r: 24,
    g: 65,
    b: 112,
    a: 255,
};
pub const SIGN_GREEN: na::color::Color = na::color::Color {
    r: 0,
    g: 105,
    b: 78,
    a: 255,
};
const WHITE: na::color::Color = na::color::Color {
    r: 250,
    g: 250,
    b: 245,
    a: 255,
};
const INK: na::color::Color = na::color::Color {
    r: 20,
    g: 30,
    b: 37,
    a: 255,
};
const BROWN: na::color::Color = na::color::Color {
    r: 104,
    g: 62,
    b: 36,
    a: 255,
};
const OCHRE: na::color::Color = na::color::Color {
    r: 224,
    g: 183,
    b: 102,
    a: 255,
};

pub struct NativeSign {
    pub view: View,
    pub scale: f64,
    on_layout: Option<Box<dyn Fn()>>,
    post: View,
    face: View,
    badge: View,
    title: Label,
    caption: Label,
    west: Label,
    east: Label,
    w: Label,
    e: Label,
    badge_number: Label,
    detail: Label,
    width: f64,
    base_height: f64,
    bilingual: bool,
    right: bool,
    style: SignStyle,
}

impl NativeSign {
    pub fn new(scale: f64, on_layout: Option<Box<dyn Fn()>>) -> Option<Self> {
        let view = View::new()?;
        let sign = NativeSign {
            view,
            scale,
            on_layout,
            post: View::new()?,
            face: View::new()?,
            badge: View::new()?,
            title: Label::new("")?,
            caption: Label::new("")?,
            west: Label::new("")?,
            east: Label::new("")?,
            w: Label::new("")?,
            e: Label::new("")?,
            badge_number: Label::new("")?,
            detail: Label::new("")?,
            width: 148.,
            base_height: 32.,
            bilingual: true,
            right: true,
            style: SignStyle::Missing,
        };
        // Keep every handle alive with the sign; on failure the part-built
        // sign is dropped, releasing what it already owns.
        let sign = sign;
        sign.view.add_subview(Some(&sign.post));
        sign.view.add_subview(Some(&sign.face));
        sign.face.add_subview(Some(&sign.title));
        sign.face.add_subview(Some(&sign.west));
        sign.face.add_subview(Some(&sign.east));
        sign.view.add_subview(Some(&sign.badge));
        sign.badge.add_subview(Some(&sign.badge_number));
        sign.view.add_subview(Some(&sign.caption));
        sign.view.add_subview(Some(&sign.w));
        sign.view.add_subview(Some(&sign.e));
        sign.view.add_subview(Some(&sign.detail));
        for label in sign.labels() {
            label.set_text_alignment(na::view::TextAlignment::Center);
        }
        Some(sign)
    }

    fn labels(&self) -> [&Label; 8] {
        [
            &self.title,
            &self.caption,
            &self.west,
            &self.east,
            &self.w,
            &self.e,
            &self.badge_number,
            &self.detail,
        ]
    }

    /// TrayIcon centers its content view in the menu bar. Reduce the view
    /// itself so the 2 pt top/bottom margin applies to every template,
    /// outside its face.
    pub fn height(&self) -> f64 {
        (self.base_height - 4.).max(1.) * self.scale
    }

    pub fn width(&self) -> f64 {
        self.width
    }

    pub fn update(
        &mut self,
        style: SignStyle,
        content: &SignContent,
        green: bool,
        english: bool,
        right: bool,
        bar_height: f64,
    ) {
        self.style = style;
        self.base_height = bar_height;
        self.bilingual = matches!(style, SignStyle::Missing | SignStyle::Travel) || english;
        self.right = right;
        for label in self.labels() {
            label.set_visible(false);
        }
        self.post.set_visible(false);
        self.badge.set_visible(false);
        self.view.set_background_color(&WHITE);

        let h = self.height();
        let main_size = h * if self.bilingual { 0.35 } else { 0.46 };
        let sub_size = h * 0.185;
        let face_color = match style {
            SignStyle::Guide => BROWN,
            SignStyle::Travel => SIGN_GREEN,
            SignStyle::Welcome => SIGN_BLUE,
            SignStyle::Missing if green => SIGN_GREEN,
            _ => SIGN_BLUE,
        };
        self.face.set_background_color(&face_color);
        match style {
            SignStyle::Missing => {
                self.text(&self.title, &content.heading(style), main_size, WHITE);
                let pinyin: String = content
                    .secondary
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
                    .collect();
                self.text(
                    &self.caption,
                    &format!("WoZai{pinyin}HenXiangNi"),
                    sub_size,
                    INK,
                );
                self.text(
                    &self.west,
                    if right { "东" } else { "西" },
                    main_size * 0.66,
                    WHITE,
                );
                self.text(
                    &self.east,
                    if right { "西" } else { "东" },
                    main_size * 0.66,
                    WHITE,
                );
                self.text(&self.w, if right { "E" } else { "W" }, sub_size, INK);
                self.text(&self.e, if right { "W" } else { "E" }, sub_size, INK);
                for label in [&self.caption, &self.w, &self.e] {
                    label.set_visible(self.bilingual);
                }
            }
            SignStyle::Welcome => {
                self.text(
                    &self.title,
                    &content.heading(style),
                    main_size * 1.12,
                    WHITE,
                );
                self.text(
                    &self.caption,
                    &format!("WELCOME TO {}", content.secondary.to_uppercase()),
                    sub_size,
                    WHITE,
                );
            }
            SignStyle::Travel => {
                self.text(&self.title, &content.primary, h * 0.28, WHITE);
                self.text(&self.caption, &content.secondary, h * 0.26, INK);
                self.text(&self.east, if right { "➜" } else { "←" }, h * 0.31, WHITE);
                self.text(&self.detail, if right { "←" } else { "➜" }, h * 0.31, INK);
                self.badge.set_visible(true);
                self.badge.set_background_color(&OCHRE);
                self.post.set_visible(true);
                self.post.set_background_color(&BROWN);
            }
            SignStyle::Guide => {
                self.text(&self.title, &content.primary, main_size * 1.1, WHITE);
                self.text(
                    &self.caption,
                    &format!("{}  /  {}", content.secondary.to_uppercase(), content.value),
                    sub_size,
                    WHITE,
                );
                self.badge.set_visible(true);
                self.badge.set_background_color(&WHITE);
                self.text(&self.badge_number, "▲", h * 0.34, BROWN);
                self.arrow(h);
            }
        }
        if style.has_subtitle_toggle() {
            self.caption.set_visible(self.bilingual);
        }
        self.layout();
    }

    fn text(&self, label: &Label, text: &str, size: f64, color: na::color::Color) {
        label.set_text(text);
        label.set_font_size(size);
        label.set_text_color(&color);
        label.set_visible(true);
    }

    fn arrow(&self, h: f64) {
        let (label, glyph) = if self.right {
            (&self.east, "➜")
        } else {
            (&self.west, "←")
        };
        self.text(label, glyph, h * 0.50, WHITE);
    }

    fn frame(&self, target: &View, x: f64, y: f64, w: f64, h: f64) {
        target.set_frame(&na::geometry::Rectangle {
            x,
            y,
            width: w,
            height: h,
        });
    }

    /// Centers a label's text block vertically inside its box.
    fn center(&self, label: &Label, x: f64, y: f64, w: f64, h: f64) {
        let text_height = label.intrinsic_size().height;
        self.frame(label, x, y + (h - text_height) / 2., w, text_height);
    }

    fn layout(&mut self) {
        match self.style {
            SignStyle::Missing => self.layout_missing(),
            SignStyle::Travel => self.layout_travel(),
            _ => self.layout_panel(),
        }
        self.view.set_preferred_size(&na::geometry::Size {
            width: self.width,
            height: self.height(),
        });
        if let Some(on_layout) = &self.on_layout {
            on_layout();
        }
    }

    fn layout_missing(&mut self) {
        let h = self.height();
        let side = (h * 0.35).max(
            [
                self.west.intrinsic_size().width,
                self.east.intrinsic_size().width,
                self.w.intrinsic_size().width,
                self.e.intrinsic_size().width,
            ]
            .into_iter()
            .fold(0., f64::max)
                + self.scale,
        );
        let text_width = self.title.intrinsic_size().width.max(if self.bilingual {
            self.caption.intrinsic_size().width
        } else {
            0.
        }) + h * 0.12;
        self.width = (text_width + side * 2.).ceil();
        let top = if self.bilingual { h * 0.68 } else { h };
        self.frame(&self.face, 0., 0., self.width, top);
        self.center(&self.title, side, 0., text_width, top);
        self.center(&self.west, 0., 0., side, top);
        self.center(&self.east, self.width - side, 0., side, top);
        self.center(&self.caption, side, top, text_width, h - top);
        self.center(&self.w, 0., top, side, h - top);
        self.center(&self.e, self.width - side, top, side, h - top);
    }

    fn layout_panel(&mut self) {
        let h = self.height();
        let edge = self.scale;
        let pad = h * 0.22;
        let face_height = h - edge * 2.;
        let badge_height = h * 0.72;
        let badge_inset = (face_height - badge_height) / 2.;
        let badge_body_width =
            badge_height.max(self.badge_number.intrinsic_size().width + self.scale * 4.);
        let badge_width = if self.badge.is_visible() {
            badge_body_width + badge_inset
        } else {
            0.
        };
        let left_arrow = if self.west.is_visible() {
            (h * 0.7).max(self.west.intrinsic_size().width + self.scale * 2.)
        } else {
            0.
        };
        let right_arrow = if self.east.is_visible() {
            (h * 0.7).max(self.east.intrinsic_size().width + self.scale * 2.)
        } else {
            0.
        };
        let body_width = self
            .title
            .intrinsic_size()
            .width
            .max(if self.caption.is_visible() {
                self.caption.intrinsic_size().width
            } else {
                0.
            })
            + pad * 2.;
        self.width = (badge_width + left_arrow + body_width + right_arrow + edge * 2.).ceil();
        self.frame(&self.face, edge, edge, self.width - edge * 2., face_height);
        let start = badge_width + left_arrow;
        let title_top = if self.bilingual {
            face_height * 0.04
        } else {
            0.
        };
        let title_height = if self.bilingual {
            face_height * 0.64
        } else {
            face_height
        };
        self.center(&self.title, start, title_top, body_width, title_height);
        self.center(&self.caption, edge + start, h * 0.64, body_width, h * 0.31);
        self.center(&self.west, badge_width, 0., left_arrow, face_height);
        self.center(
            &self.east,
            self.width - edge * 2. - right_arrow,
            0.,
            right_arrow,
            face_height,
        );
        if self.badge.is_visible() {
            let (bh, bw) = (badge_height, badge_body_width);
            self.frame(&self.badge, edge + badge_inset, edge + badge_inset, bw, bh);
            self.center(&self.badge_number, 0., 0., bw, bh);
        }
    }

    fn layout_travel(&mut self) {
        let h = self.height();
        let side = (h * 0.60).max(
            self.east
                .intrinsic_size()
                .width
                .max(self.detail.intrinsic_size().width)
                + self.scale * 2.,
        );
        let body = self
            .title
            .intrinsic_size()
            .width
            .max(self.caption.intrinsic_size().width)
            + h * 0.3;
        self.width = (body + side).ceil();
        self.frame(&self.post, self.width * 0.47, 0., h * 0.12, h);
        self.frame(&self.face, 0., 0., self.width, h * 0.47);
        self.frame(&self.badge, 0., h * 0.53, self.width, h * 0.47);
        self.center(&self.title, 0., 0., body, h * 0.47);
        self.center(&self.east, body, 0., side, h * 0.47);
        self.center(&self.detail, 0., h * 0.53, side, h * 0.47);
        self.center(&self.caption, side, h * 0.53, body, h * 0.47);
    }

    /// Dropping releases every handle the sign owns, each exactly once.
    #[allow(dead_code)]
    pub fn dispose(self) {}
}
