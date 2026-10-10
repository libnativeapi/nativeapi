//! What each binding generates for every symbol, for the API reference.
//!
//! Every generator exposes `reference(api, origins) -> Reference`, filed under
//! the keys of `codegen_shared::symbols` — the same keys the parser files doc
//! comments under. A symbol the binding does not expose is simply absent, and
//! the reference shows it as unavailable in that language.
//!
//! The signatures must be the ones the generator writes into the binding:
//! render code and `reference` call the same signature helpers, so the two
//! cannot drift apart.

use std::collections::BTreeMap;

use serde::Serialize;

#[derive(Debug, Clone, Default, Serialize)]
pub struct Reference {
    pub entries: BTreeMap<String, Entry>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize)]
pub struct Entry {
    /// The identifier as this language spells it: `setTitle`, `set_title`,
    /// `title` (a Dart / Python property), `TitleBarStyle.hidden`.
    pub name: String,
    /// The declaration as generated, without its body or a trailing `{`, and
    /// without leading indentation: `void setTitle(String title)`,
    /// `pub fn set_title(&self, title: &str)`, `class Window extends …`.
    /// Several lines are allowed (a property getter plus its setter).
    pub signature: String,
}

impl Reference {
    pub fn insert(&mut self, key: impl Into<String>, name: impl Into<String>, signature: impl Into<String>) {
        self.entries.insert(
            key.into(),
            Entry {
                name: name.into(),
                signature: signature.into(),
            },
        );
    }
}
