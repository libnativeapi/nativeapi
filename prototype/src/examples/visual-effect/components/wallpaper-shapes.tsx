import './wallpaper-shapes.css'

/**
 * Bold shapes on the wallpaper, under the example's window: a material only
 * reads as one when something with edges and colour is behind it, and the
 * stage's soft wash alone would blur into itself.
 */
export function WallpaperShapes() {
  return (
    <div className="wallpaper-shapes" aria-hidden>
      <span className="wallpaper-shapes__circle" />
      <span className="wallpaper-shapes__stripes" />
      <span className="wallpaper-shapes__square" />
      <span className="wallpaper-shapes__triangle" />
      <span className="wallpaper-shapes__ring" />
    </div>
  )
}
