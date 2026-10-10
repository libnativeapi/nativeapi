//! Doxygen comments → `Doc` (Markdown text).
//!
//! Covers the subset the headers use: `@brief`, `@param`, `@return(s)`,
//! `@note`, `@warning`, `@see`, `@thread_safety`, `@example` (followed by a
//! fenced block), `@code` / `@endcode`, and the inline `@p` / `@c`.
//! Declaration tags (`@class`, `@struct`, `@enum`, `@typedef`, `@file`) only
//! repeat what the declaration says and are dropped.

use crate::ir::{Doc, NoteDoc, ParamDoc};

/// Parses a raw comment as libclang returns it (markers included). Returns
/// `None` when nothing but markers and declaration tags is left.
pub fn parse(raw: &str) -> Option<Doc> {
    let lines = strip_markers(raw);

    let mut doc = Doc::default();
    let mut paragraphs: Vec<String> = Vec::new();
    let mut current = Section::Body;
    let mut buffer: Vec<String> = Vec::new();
    let mut in_code = false;

    for line in lines {
        let trimmed = line.trim();

        // Code blocks pass through verbatim, whatever they contain.
        if in_code {
            if trimmed == "@endcode" || trimmed == "\\endcode" || trimmed.starts_with("```") {
                buffer.push("```".to_string());
                in_code = false;
            } else {
                buffer.push(line.clone());
            }
            continue;
        }
        if trimmed == "@code" || trimmed.starts_with("@code{") || trimmed == "\\code" {
            flush(&mut doc, &mut paragraphs, current, &mut buffer);
            current = Section::Body;
            buffer.push("```cpp".to_string());
            in_code = true;
            continue;
        }
        if trimmed.starts_with("```") {
            flush(&mut doc, &mut paragraphs, current, &mut buffer);
            current = Section::Body;
            buffer.push(trimmed.to_string());
            in_code = true;
            continue;
        }

        if trimmed.is_empty() {
            flush(&mut doc, &mut paragraphs, current, &mut buffer);
            current = Section::Body;
            continue;
        }

        if let Some((tag, rest)) = split_tag(trimmed) {
            flush(&mut doc, &mut paragraphs, current, &mut buffer);
            current = match tag {
                "brief" | "short" => Section::Summary,
                "param" | "tparam" => {
                    let (name, text) = split_word(rest);
                    doc.params.push(ParamDoc {
                        name: name.to_string(),
                        text: String::new(),
                    });
                    buffer.push(text.to_string());
                    Section::Param
                }
                "return" | "returns" | "retval" => Section::Returns,
                "note" | "warning" | "see" | "sa" | "thread_safety" | "attention"
                | "deprecated" => {
                    let kind = match tag {
                        "sa" => "see",
                        "attention" => "warning",
                        other => other,
                    };
                    doc.notes.push(NoteDoc {
                        kind: kind.to_string(),
                        text: String::new(),
                    });
                    Section::Note
                }
                "example" | "par" => {
                    let title = if rest.is_empty() { "Example" } else { rest };
                    paragraphs.push(format!("**{title}**"));
                    continue;
                }
                "class" | "struct" | "enum" | "typedef" | "file" | "namespace" | "fn" => {
                    Section::Discard
                }
                _ => Section::Body,
            };
            if !matches!(current, Section::Param) && !rest.is_empty() {
                buffer.push(rest.to_string());
            }
            continue;
        }

        buffer.push(trimmed.to_string());
    }
    if in_code {
        buffer.push("```".to_string());
    }
    flush(&mut doc, &mut paragraphs, current, &mut buffer);

    if doc.summary.is_empty() && !paragraphs.is_empty() {
        // Without `@brief`, doxygen's own rule: the first paragraph is the
        // summary — unless it is a code block or an example heading.
        if !paragraphs[0].starts_with("```") && !paragraphs[0].starts_with("**") {
            doc.summary = paragraphs.remove(0);
        }
    }
    doc.details = paragraphs.join("\n\n");
    doc.params.retain(|param| !param.name.is_empty());

    let empty = doc.summary.is_empty()
        && doc.details.is_empty()
        && doc.params.iter().all(|param| param.text.is_empty())
        && doc.returns.is_none()
        && doc.notes.is_empty();
    (!empty).then_some(doc)
}

#[derive(Debug, Clone, Copy)]
enum Section {
    Body,
    Summary,
    Param,
    Returns,
    Note,
    Discard,
}

/// Moves the buffered lines into the section they belong to.
fn flush(doc: &mut Doc, paragraphs: &mut Vec<String>, section: Section, buffer: &mut Vec<String>) {
    if buffer.is_empty() {
        return;
    }
    let is_code = buffer.first().is_some_and(|line| line.starts_with("```"));
    let text = if is_code {
        buffer.join("\n")
    } else {
        inline(&join_prose(buffer))
    };
    buffer.clear();
    let text = text.trim().to_string();
    if text.is_empty() {
        return;
    }
    match section {
        Section::Body => paragraphs.push(text),
        Section::Summary => append(&mut doc.summary, &text),
        Section::Param => {
            if let Some(param) = doc.params.last_mut() {
                append(&mut param.text, &text);
            }
        }
        Section::Returns => {
            let returns = doc.returns.get_or_insert_with(String::new);
            append(returns, &text);
        }
        Section::Note => {
            if let Some(note) = doc.notes.last_mut() {
                append(&mut note.text, &text);
            }
        }
        Section::Discard => {}
    }
}

/// Joins wrapped prose lines with spaces, but starts a new line for each list
/// item so Markdown still sees the list.
fn join_prose(lines: &[String]) -> String {
    let mut out = String::new();
    for line in lines {
        if !out.is_empty() {
            out.push(if is_list_item(line) { '\n' } else { ' ' });
        }
        out.push_str(line);
    }
    out
}

fn is_list_item(line: &str) -> bool {
    let line = line.trim_start();
    if line.starts_with("- ") || line.starts_with("* ") || line.starts_with("+ ") {
        return true;
    }
    let digits = line.chars().take_while(char::is_ascii_digit).count();
    digits > 0 && (line[digits..].starts_with(". ") || line[digits..].starts_with(") "))
}

fn append(target: &mut String, text: &str) {
    if !target.is_empty() {
        target.push(' ');
    }
    target.push_str(text);
}

/// Comment lines with the `/**`, `*/`, `*`, `///`, `///<`, `//!` markers
/// removed. Indentation inside code blocks is kept relative to the marker.
fn strip_markers(raw: &str) -> Vec<String> {
    raw.lines()
        .map(|line| {
            let mut text = line.trim_start();
            for opener in ["/**<", "/*!<", "/**", "/*!", "/*", "///<", "//!<", "///", "//!", "//"] {
                if let Some(rest) = text.strip_prefix(opener) {
                    text = rest;
                    break;
                }
            }
            let text = text.strip_suffix("*/").unwrap_or(text);
            let text = if let Some(rest) = text.trim_start().strip_prefix('*') {
                // A lone `*` continuation marker, not `**bold**`.
                if rest.starts_with('*') { text } else { rest }
            } else {
                text
            };
            // One space after the marker is formatting, the rest is content.
            text.strip_prefix(' ').unwrap_or(text).trim_end().to_string()
        })
        .collect()
}

/// `@param[in] title The title` -> (`param`, `title The title`).
fn split_tag(line: &str) -> Option<(&str, &str)> {
    let rest = line.strip_prefix('@').or_else(|| line.strip_prefix('\\'))?;
    let end = rest
        .find(|c: char| !(c.is_ascii_alphanumeric() || c == '_'))
        .unwrap_or(rest.len());
    if end == 0 {
        return None;
    }
    let tag = &rest[..end];
    // `@p` / `@c` at the start of a sentence are inline markup, not tags.
    if matches!(tag, "p" | "c" | "a" | "ref") {
        return None;
    }
    let after = rest[end..].trim_start();
    // Direction attribute: `@param[in]`.
    let after = if after.starts_with('[') {
        after.split_once(']').map_or(after, |(_, tail)| tail).trim_start()
    } else {
        after
    };
    Some((tag, after))
}

fn split_word(text: &str) -> (&str, &str) {
    let text = text.trim_start();
    match text.split_once(char::is_whitespace) {
        Some((word, rest)) => (word, rest.trim_start()),
        None => (text, ""),
    }
}

/// Inline doxygen markup to Markdown: `@p name` / `@c name` / `@a name` ->
/// `` `name` ``, `@ref Name` -> `Name`.
fn inline(text: &str) -> String {
    let mut out = String::with_capacity(text.len());
    let mut words = text.split(' ').peekable();
    while let Some(word) = words.next() {
        let marker = word.strip_prefix('@').or_else(|| word.strip_prefix('\\'));
        match marker {
            Some("p" | "c" | "a") if words.peek().is_some() => {
                let target = words.next().unwrap_or_default();
                // Keep trailing punctuation outside the code span.
                let split = target
                    .find(|c: char| matches!(c, ',' | '.' | ';' | ':' | ')'))
                    .filter(|&index| index > 0)
                    .unwrap_or(target.len());
                out.push('`');
                out.push_str(&target[..split]);
                out.push('`');
                out.push_str(&target[split..]);
            }
            Some("ref") if words.peek().is_some() => {
                out.push_str(words.next().unwrap_or_default());
            }
            _ => out.push_str(word),
        }
        if words.peek().is_some() {
            out.push(' ');
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_brief_params_and_return() {
        let doc = parse(
            "/**\n   * @brief Run the main loop\n   *\n   * Blocks until Quit() stops it.\n   *\n   * @param window The window to run with\n   * @return The exit code, or -1 when @p window is null\n   */",
        )
        .unwrap();
        assert_eq!(doc.summary, "Run the main loop");
        assert_eq!(doc.details, "Blocks until Quit() stops it.");
        assert_eq!(doc.params[0].name, "window");
        assert_eq!(doc.params[0].text, "The window to run with");
        assert_eq!(
            doc.returns.as_deref(),
            Some("The exit code, or -1 when `window` is null")
        );
    }

    #[test]
    fn keeps_code_blocks_and_drops_declaration_tags() {
        let doc = parse(
            "/**\n * @class Window\n * @brief A window.\n *\n * @code\n * auto w = std::make_shared<Window>();\n *   w->Show();\n * @endcode\n */",
        )
        .unwrap();
        assert_eq!(doc.summary, "A window.");
        assert_eq!(
            doc.details,
            "```cpp\nauto w = std::make_shared<Window>();\n  w->Show();\n```"
        );
    }

    #[test]
    fn first_paragraph_is_the_summary_without_brief() {
        let doc = parse("/** The bottom-left corner; both width and height change. */").unwrap();
        assert_eq!(
            doc.summary,
            "The bottom-left corner; both width and height change."
        );
    }

    #[test]
    fn notes_and_examples() {
        let doc = parse(
            "/**\n * @brief Opens it.\n * @note Main thread only.\n * @see Hide() for platform availability.\n * @example\n * ```cpp\n * dialog->Open();\n * ```\n */",
        )
        .unwrap();
        assert_eq!(doc.notes.len(), 2);
        assert_eq!(doc.notes[0].kind, "note");
        assert_eq!(doc.notes[1].text, "Hide() for platform availability.");
        assert_eq!(doc.details, "**Example**\n\n```cpp\ndialog->Open();\n```");
    }

    #[test]
    fn keeps_list_items_on_their_own_lines() {
        let doc = parse(
            "/**\n * @brief Moved.\n * @note Platform availability:\n * - macOS: supported\n * - Linux: X11 only,\n *   not Wayland\n */",
        )
        .unwrap();
        assert_eq!(
            doc.notes[0].text,
            "Platform availability:\n- macOS: supported\n- Linux: X11 only, not Wayland"
        );
    }

    #[test]
    fn markers_only_is_none() {
        assert!(parse("/**\n * @class Foo\n */").is_none());
    }
}
