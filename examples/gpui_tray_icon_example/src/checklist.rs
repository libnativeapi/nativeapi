//! The acceptance checklist, the counterpart of the Flutter example's
//! `checklist.dart`.
//!
//! Auto items are settled by the controller from real events and return
//! values; manual items are marked from the Checklist tab.

use std::collections::{BTreeSet, HashMap};

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum CheckStatus {
    Open,
    Pass,
    Fail,
}

impl CheckStatus {
    pub fn name(self) -> &'static str {
        match self {
            CheckStatus::Open => "open",
            CheckStatus::Pass => "pass",
            CheckStatus::Fail => "fail",
        }
    }
}

pub struct CheckItem {
    pub id: &'static str,
    pub label: &'static str,
    /// Manual items need eyes on the tray; the rest tick themselves.
    pub manual: bool,
    /// Short platform remark shown next to the label.
    pub note: Option<&'static str>,
    pub status: CheckStatus,
    pub detail: String,
}

pub const SUPPORTED: &str = "supported";
pub const CREATE: &str = "create";
pub const MANAGED: &str = "managed";
pub const CLICKED: &str = "clicked";
pub const RIGHT_CLICKED: &str = "rightClicked";
pub const DOUBLE_CLICKED: &str = "doubleClicked";
pub const MENU_OPEN_CLOSE: &str = "menuOpenClose";
pub const MENU_ITEMS: &str = "menuItems";
pub const TRIGGERS: &str = "triggers";
pub const OPEN_MENU: &str = "openMenu";
pub const CLOSE_MENU: &str = "closeMenu";
pub const VISIBLE: &str = "visible";
pub const READ_BACK: &str = "readBack";
pub const BOUNDS: &str = "bounds";
pub const FRAMES: &str = "frames";

pub struct Checklist {
    pub items: Vec<CheckItem>,
    /// Partial progress of the multi-part items.
    parts: HashMap<&'static str, BTreeSet<String>>,
}

impl Checklist {
    pub fn new() -> Self {
        let linux = cfg!(target_os = "linux");
        let windows = cfg!(target_os = "windows");
        let mut items = vec![
            CheckItem::new(SUPPORTED, "TrayManager.isSupported"),
            CheckItem::new(CREATE, "TrayIcon.create returns an icon"),
            CheckItem::new(MANAGED, "TrayManager get and getAll know it"),
            CheckItem::new(CLICKED, "Clicked event"),
            CheckItem::new(RIGHT_CLICKED, "Right clicked event"),
            CheckItem::new(DOUBLE_CLICKED, "Double clicked event"),
            CheckItem::new(MENU_OPEN_CLOSE, "Menu opened and closed"),
            CheckItem::new(MENU_ITEMS, "Menu item, checkbox, submenu"),
            CheckItem::new(
                TRIGGERS,
                if linux {
                    "Left, right and double click open the menu"
                } else {
                    "All four triggers open the menu"
                },
            ),
        ];
        // A StatusNotifierItem (Linux) cannot pop its menu up or be located.
        if !linux {
            items.push(CheckItem::new(OPEN_MENU, "openContextMenu returns true"));
            items.push(CheckItem::new(CLOSE_MENU, "closeContextMenu closes it"));
        }
        items.push(CheckItem::new(VISIBLE, "setVisible round trip"));
        items.push(CheckItem::new(
            READ_BACK,
            if windows {
                "Tooltip reads back"
            } else {
                "Title and tooltip read back"
            },
        ));
        if !linux {
            items.push(CheckItem::new(BOUNDS, "getBounds is not empty"));
        }
        items.push(CheckItem::new(FRAMES, "100 frames at the target rate"));
        items.push(CheckItem::manual(
            "m.still",
            "Icon swaps: asset, drawn, base64",
        ));
        items.push(CheckItem::manual(
            "m.animation",
            "Tray animation matches preview",
        ));
        items.push(CheckItem::manual("m.three", "Three icons animate at once"));
        items.push(CheckItem::manual(
            "m.popup",
            "Popup: a click shows the window, losing focus hides it",
        ));
        if !windows {
            items.push(CheckItem::manual_with_note(
                "m.title",
                "Title next to the icon",
                "macOS",
            ));
        }
        items.push(CheckItem::manual("m.tooltip", "Tooltip on hover"));
        items.push(CheckItem::manual("m.hidden", "Hidden icon leaves the tray"));
        items.push(CheckItem::manual(
            "m.removed",
            "Removed icon leaves the tray",
        ));
        Checklist {
            items,
            parts: HashMap::new(),
        }
    }

    pub fn get(&self, id: &str) -> Option<&CheckItem> {
        self.items.iter().find(|item| item.id == id)
    }

    pub fn passed(&self) -> usize {
        self.items
            .iter()
            .filter(|item| item.status == CheckStatus::Pass)
            .count()
    }

    pub fn failed(&self) -> usize {
        self.items
            .iter()
            .filter(|item| item.status == CheckStatus::Fail)
            .count()
    }

    pub fn pass(&mut self, id: &str, detail: &str) {
        self.set(id, CheckStatus::Pass, detail);
    }

    pub fn fail(&mut self, id: &str, detail: &str) {
        self.set(id, CheckStatus::Fail, detail);
    }

    /// Updates the detail of an item that is still open.
    pub fn note(&mut self, id: &str, detail: &str) {
        let Some(item) = self.find_mut(id) else {
            return;
        };
        if item.status != CheckStatus::Open || item.detail == detail {
            return;
        }
        item.detail = detail.to_string();
    }

    /// Records `part` of a multi-part item; passes once `total` are in.
    pub fn part(&mut self, id: &'static str, part: &str, total: usize) {
        let parts = self.parts.entry(id).or_default();
        parts.insert(part.to_string());
        let have = parts.len();
        if have >= total {
            self.pass(id, &format!("{total}/{total}"));
        } else {
            self.note(id, &format!("{have}/{total}"));
        }
    }

    /// Counts an occurrence of an event item (detail becomes "×n").
    pub fn count(&mut self, id: &'static str) {
        let parts = self.parts.entry(id).or_default();
        parts.insert(parts.len().to_string());
        let n = parts.len();
        self.set(id, CheckStatus::Pass, &format!("×{n}"));
    }

    pub fn mark(&mut self, id: &str, status: CheckStatus) {
        let Some(item) = self.find_mut(id) else {
            return;
        };
        let next = if item.status == status {
            CheckStatus::Open
        } else {
            status
        };
        item.status = next;
        item.detail.clear();
    }

    pub fn reset(&mut self) {
        self.parts.clear();
        for item in &mut self.items {
            item.status = CheckStatus::Open;
            item.detail.clear();
        }
    }

    pub fn summary(&self) -> String {
        let open = self.items.len() - self.passed() - self.failed();
        format!(
            "{} pass · {} fail · {} open",
            self.passed(),
            self.failed(),
            open
        )
    }

    /// Plain-text report for pasting into an issue or a PR.
    pub fn report(&self) -> String {
        let mut buffer = String::new();
        buffer.push_str("tray_icon_example checklist\n");
        buffer.push_str(&format!("{} {}\n", std::env::consts::OS, os_version()));
        buffer.push_str(&self.summary());
        buffer.push_str("\n\n");
        for item in &self.items {
            let mark = match item.status {
                CheckStatus::Pass => "PASS",
                CheckStatus::Fail => "FAIL",
                CheckStatus::Open => "open",
            };
            let kind = if item.manual { "manual" } else { "auto" };
            let detail = if item.detail.is_empty() {
                String::new()
            } else {
                format!("  ({})", item.detail)
            };
            buffer.push_str(&format!("[{mark}] {}  <{kind}>{detail}\n", item.label));
        }
        buffer
    }

    // Items a platform cannot offer are left out of `items`; settling one is
    // then simply ignored.
    fn find_mut(&mut self, id: &str) -> Option<&mut CheckItem> {
        self.items.iter_mut().find(|item| item.id == id)
    }

    fn set(&mut self, id: &str, status: CheckStatus, detail: &str) {
        let Some(item) = self.find_mut(id) else {
            return;
        };
        if item.status == status && item.detail == detail {
            return;
        }
        item.status = status;
        item.detail = detail.to_string();
        // One line per change on stdout, so a GUI test can assert on the list
        // without having to read ticks off the screen.
        println!("[checklist] {} {} {}", item.id, status.name(), detail);
    }
}

impl CheckItem {
    fn new(id: &'static str, label: &'static str) -> Self {
        CheckItem {
            id,
            label,
            manual: false,
            note: None,
            status: CheckStatus::Open,
            detail: String::new(),
        }
    }

    fn manual(id: &'static str, label: &'static str) -> Self {
        CheckItem {
            manual: true,
            ..Self::new(id, label)
        }
    }

    fn manual_with_note(id: &'static str, label: &'static str, note: &'static str) -> Self {
        CheckItem {
            note: Some(note),
            ..Self::manual(id, label)
        }
    }
}

fn os_version() -> String {
    #[cfg(target_os = "macos")]
    {
        std::process::Command::new("sw_vers")
            .arg("-productVersion")
            .output()
            .ok()
            .and_then(|output| String::from_utf8(output.stdout).ok())
            .map(|version| version.trim().to_string())
            .unwrap_or_default()
    }
    #[cfg(not(target_os = "macos"))]
    {
        String::new()
    }
}
