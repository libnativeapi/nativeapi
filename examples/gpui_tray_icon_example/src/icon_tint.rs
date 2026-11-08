//! Recolour a monochrome image without losing its dimensions or alpha mask,
//! the counterpart of the Flutter example's `icon_tint.dart`.

use crate::icon_animations::Rgb;

/// Takes PNG bytes, returns PNG bytes with every pixel's colour replaced by
/// `ink` and its alpha kept.
pub fn tint_png(png: &[u8], ink: Rgb) -> Option<Vec<u8>> {
    let rgba = image::load_from_memory(png).ok()?.into_rgba8();
    let mut tinted = rgba;
    for pixel in tinted.pixels_mut() {
        pixel.0 = [ink[0], ink[1], ink[2], pixel.0[3]];
    }
    let mut out = Vec::new();
    image::DynamicImage::ImageRgba8(tinted)
        .write_to(&mut std::io::Cursor::new(&mut out), image::ImageFormat::Png)
        .ok()?;
    Some(out)
}
