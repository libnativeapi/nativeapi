//! The sign styles and their content, the counterpart of the Flutter
//! example's `signs/sign_style.dart`.

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub enum SignStyle {
    Missing,
    Welcome,
    Travel,
    Guide,
}

pub const ALL: [SignStyle; 4] = [
    SignStyle::Missing,
    SignStyle::Welcome,
    SignStyle::Travel,
    SignStyle::Guide,
];

impl SignStyle {
    pub fn label(self) -> &'static str {
        match self {
            SignStyle::Missing => "Missing You Sign",
            SignStyle::Welcome => "City Welcome Sign",
            SignStyle::Travel => "Travel Sign",
            SignStyle::Guide => "Scenic Guide Sign",
        }
    }

    pub fn short_label(self) -> &'static str {
        match self {
            SignStyle::Missing => "Missing You",
            SignStyle::Welcome => "Welcome",
            SignStyle::Travel => "Travel",
            SignStyle::Guide => "Guide",
        }
    }

    pub fn has_direction(self) -> bool {
        matches!(
            self,
            SignStyle::Missing | SignStyle::Travel | SignStyle::Guide
        )
    }

    pub fn has_subtitle_toggle(self) -> bool {
        matches!(self, SignStyle::Welcome | SignStyle::Guide)
    }

    pub fn fields(self) -> &'static [&'static str] {
        match self {
            SignStyle::Missing => &["Place", "Pinyin · Separate syllables with spaces"],
            SignStyle::Welcome => &["City", "English name"],
            SignStyle::Guide => &["Attraction", "English name", "Distance · e.g. 2 km / 300 m"],
            SignStyle::Travel => &["Upper text", "Lower text"],
        }
    }

    pub fn defaults(self) -> SignContent {
        match self {
            SignStyle::Missing => SignContent::new("上海", "SHANG HAI"),
            SignStyle::Welcome => SignContent::new("大理", "DALI"),
            SignStyle::Travel => SignContent::new("下一站，山海", "去有风的地方"),
            SignStyle::Guide => SignContent::with_value("观景台", "VIEWPOINT", "300 m"),
        }
    }
}

#[derive(Clone, PartialEq, Eq, Debug, Default)]
pub struct SignContent {
    pub primary: String,
    pub secondary: String,
    pub value: String,
    pub detail: String,
}

impl SignContent {
    pub fn new(primary: &str, secondary: &str) -> Self {
        SignContent {
            primary: primary.to_string(),
            secondary: secondary.to_string(),
            value: String::new(),
            detail: String::new(),
        }
    }

    pub fn with_value(primary: &str, secondary: &str, value: &str) -> Self {
        SignContent {
            value: value.to_string(),
            ..Self::new(primary, secondary)
        }
    }

    pub fn values(&self) -> [&str; 4] {
        [&self.primary, &self.secondary, &self.value, &self.detail]
    }

    pub fn heading(&self, style: SignStyle) -> String {
        match style {
            SignStyle::Missing => format!("我在{}很想你", self.primary),
            SignStyle::Welcome => format!("{}欢迎您", self.primary),
            _ => self.primary.clone(),
        }
    }

    /// Invalid input leaves the current sign intact: the error message, or
    /// `None` when the content is acceptable.
    pub fn validate(&self, style: SignStyle) -> Option<String> {
        let labels = style.fields();
        let entries = self.values();
        for (i, label) in labels.iter().enumerate() {
            let limit = if i == 1 { 36 } else { 16 };
            let entry = entries[i];
            let too_long = entry.chars().count() > limit;
            let control = entry.chars().any(|c| c == '\r' || c == '\n' || c == '\t');
            if entry.is_empty() || too_long || control {
                return Some(format!(
                    "{} must contain 1–{limit} characters and no line breaks or tabs.",
                    label.split(" · ").next().unwrap_or(label)
                ));
            }
        }
        if style == SignStyle::Missing {
            if self.primary.chars().count() > 10 {
                return Some("Place must contain at most 10 characters.".to_string());
            }
            let pinyin_ok = !self.secondary.is_empty()
                && self.secondary.chars().count() <= 24
                && self
                    .secondary
                    .chars()
                    .all(|c| c.is_ascii_alphanumeric() || c == ' ' || c == '-');
            if !pinyin_ok {
                return Some(
                    "Pinyin must contain 1–24 letters, digits, spaces or hyphens.".to_string(),
                );
            }
        }
        None
    }
}
