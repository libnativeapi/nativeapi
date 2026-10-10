//! Stable keys for every documented symbol of the IR.
//!
//! The parser files doc comments under these keys, and each binding generator
//! files the signature it emits for the same symbol under the same key, so the
//! API reference can join the two without either side knowing about the other.
//!
//! | Symbol                          | Key                         |
//! | ------------------------------- | --------------------------- |
//! | class / struct / enum / alias   | `Window`                    |
//! | event group or event variant    | `WindowEvent`, `WindowMovedEvent` |
//! | constructor                     | `Window::new`               |
//! | method (class or struct)        | `Window::SetTitle`          |
//! | listener registration           | `Window::listener`          |
//! | struct field / event field      | `Size::width`, `WindowMovedEvent::NewPosition` |
//! | struct constant                 | `Color::Black`              |
//! | enum variant                    | `TitleBarStyle::Hidden`     |
//!
//! Overloads repeat a name; the second and later ones get `/1`, `/2`, … in
//! declaration order (`Application::Run`, `Application::Run/1`).

use crate::ir::{Class, Enum, EventGroup, EventVariant, Struct};

/// Keys for a list of names in declaration order, numbering repeats.
pub fn numbered<'a>(owner: &str, names: impl IntoIterator<Item = &'a str>) -> Vec<String> {
    let mut seen = std::collections::HashMap::<&str, usize>::new();
    names
        .into_iter()
        .map(|name| {
            let count = seen.entry(name).or_insert(0);
            let key = match *count {
                0 => format!("{owner}::{name}"),
                n => format!("{owner}::{name}/{n}"),
            };
            *count += 1;
            key
        })
        .collect()
}

pub fn member(owner: &str, name: &str) -> String {
    format!("{owner}::{name}")
}

/// One key per `class.constructors`, in order.
pub fn constructors(class: &Class) -> Vec<String> {
    numbered(&class.name, class.constructors.iter().map(|_| "new"))
}

/// One key per `class.methods`, in order.
pub fn class_methods(class: &Class) -> Vec<String> {
    numbered(&class.name, class.methods.iter().map(|method| method.name.as_str()))
}

/// Listener registration of a class that emits events.
pub fn listener(class: &Class) -> String {
    member(&class.name, "listener")
}

/// One key per `item.methods`, in order.
pub fn struct_methods(item: &Struct) -> Vec<String> {
    numbered(&item.name, item.methods.iter().map(|method| method.name.as_str()))
}

pub fn struct_fields(item: &Struct) -> Vec<String> {
    item.fields
        .iter()
        .map(|field| member(&item.name, &field.name))
        .collect()
}

pub fn struct_constants(item: &Struct) -> Vec<String> {
    item.constants
        .iter()
        .map(|constant| member(&item.name, constant))
        .collect()
}

pub fn enum_variants(item: &Enum) -> Vec<String> {
    item.variants
        .iter()
        .map(|variant| member(&item.name, &variant.name))
        .collect()
}

/// Fields every variant of the group carries.
pub fn event_common_fields(group: &EventGroup) -> Vec<String> {
    group
        .common
        .iter()
        .map(|field| member(&group.name, &field.name))
        .collect()
}

pub fn event_variant_fields(variant: &EventVariant) -> Vec<String> {
    variant
        .fields
        .iter()
        .map(|field| member(&variant.name, &field.name))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn numbers_overloads_in_order() {
        assert_eq!(
            numbered("Application", ["Run", "Quit", "Run", "Run"]),
            vec![
                "Application::Run",
                "Application::Quit",
                "Application::Run/1",
                "Application::Run/2",
            ]
        );
    }
}
