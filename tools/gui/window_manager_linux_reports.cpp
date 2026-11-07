// GTK/Wayland integration fixture for window_manager #594/#595. No input sent.
// Compile against core and GTK3; WAYLAND_DEBUG=client records compositor
// regions.
#include <cstdint>
#include <gdk/gdkwayland.h>
#include <gtk/gtk.h>
#include <iostream>
#include <wayland-client.h>
#include <window.h>

static void Settle() {
  auto *loop = g_main_loop_new(nullptr, FALSE);
  g_timeout_add(
      600,
      +[](gpointer data) -> gboolean {
        g_main_loop_quit(static_cast<GMainLoop *>(data));
        return G_SOURCE_REMOVE;
      },
      loop);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
}
static void Report(GtkWidget *widget, GtkWidget *content, const char *phase) {
  Settle();
  int x = 0, y = 0;
  gtk_widget_translate_coordinates(content, widget, 0, 0, &x, &y);
  const int width = gtk_widget_get_allocated_width(widget);
  const int height = gtk_widget_get_allocated_height(widget);
  const int cw = gtk_widget_get_allocated_width(content);
  const int ch = gtk_widget_get_allocated_height(content);
  auto *image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  auto *cr = cairo_create(image);
  gtk_widget_draw(widget, cr);
  cairo_destroy(cr);
  cairo_surface_flush(image);
  auto alpha = [&](int px, int py) {
    if (px < 0 || py < 0 || px >= width || py >= height)
      return -1;
    auto *row = reinterpret_cast<uint32_t *>(
        cairo_image_surface_get_data(image) +
        py * cairo_image_surface_get_stride(image));
    return int(row[px] >> 24);
  };
  auto *surface =
      gdk_wayland_window_get_wl_surface(gtk_widget_get_window(widget));
  GdkRGBA background{};
  gtk_style_context_get_background_color(gtk_widget_get_style_context(widget),
                                         gtk_widget_get_state_flags(widget),
                                         &background);
  std::cout << "REPORT {\"phase\":\"" << phase << "\",\"surface\":"
            << wl_proxy_get_id(reinterpret_cast<wl_proxy *>(surface))
            << ",\"content\":[" << x << ',' << y << ',' << cw << ',' << ch
            << ']' << ",\"size\":[" << width << ',' << height << ']'
            << ",\"gdkSize\":["
            << gdk_window_get_width(gtk_widget_get_window(widget)) << ','
            << gdk_window_get_height(gtk_widget_get_window(widget)) << ']'
            << ",\"themeAlpha\":" << background.alpha << ",\"appPaintable\":"
            << (gtk_widget_get_app_paintable(widget) ? "true" : "false")
            << ",\"gutterAlpha\":[" << alpha(x - 20, y + ch / 2) << ','
            << alpha(x + cw + 20, y + ch / 2) << ','
            << alpha(x + cw / 2, y - 20) << ','
            << alpha(x + cw / 2, y + ch + 20) << ']' << ",\"rgbaVisual\":"
            << (gtk_widget_get_visual(widget) ==
                        gdk_screen_get_rgba_visual(
                            gtk_widget_get_screen(widget))
                    ? "true"
                    : "false")
            << '}' << std::endl;
  cairo_surface_destroy(image);
  // A phase boundary in the same stream as the submitted Wayland requests.
  std::cerr << "REGION_PHASE " << phase << std::endl;
}
int main(int argc, char **argv) {
  if (!gtk_init_check(&argc, &argv))
    return 77;
  if (g_getenv("NATIVEAPI_REPORT_HIGH_PRIORITY_THEME")) {
    auto *theme = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        theme, "window, window:backdrop { background: #eff0f1; }", -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(theme),
        GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
    g_object_unref(theme);
  }
  auto *widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(widget), "NativeAPI Linux report regression");
  if (g_getenv("NATIVEAPI_REPORT_SSD")) {
    // A host that did not request client decorations (KWin can prefer SSD) may
    // realize a plain RGB window before nativeapi is loaded by Dart.
    gtk_window_set_decorated(GTK_WINDOW(widget), FALSE);
  } else {
    gtk_window_set_titlebar(GTK_WINDOW(widget), gtk_header_bar_new());
  }
  gtk_window_set_default_size(GTK_WINDOW(widget), 480, 320);
  auto *content = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(widget), content);
  g_signal_connect(
      content, "draw",
      G_CALLBACK(+[](GtkWidget *, cairo_t *cr, gpointer) -> gboolean {
        cairo_set_source_rgb(cr, 0.1, 0.5, 0.2);
        cairo_paint(cr);
        return TRUE;
      }),
      nullptr);
  gtk_widget_show_all(widget);
  Settle();
  if (g_getenv("NATIVEAPI_REPORT_HOST_BACKGROUND")) {
    // A host's renderer can clear an opaque backing independently of GTK CSS.
    // Simulate that backing before nativeapi's own shadow pass.
    g_signal_connect_after(
        widget, "draw",
        G_CALLBACK(+[](GtkWidget *, cairo_t *cr, gpointer) -> gboolean {
          cairo_save(cr);
          cairo_reset_clip(cr);
          cairo_set_source_rgb(cr, 1, 1, 1);
          cairo_paint(cr);
          cairo_restore(cr);
          return FALSE;
        }),
        nullptr);
  }
  nativeapi::Window window(widget);
  if (g_getenv("NATIVEAPI_REPORT_BACKGROUND"))
    window.SetBackgroundColor({255, 255, 255, 255});
  window.SetTitleBarStyle(nativeapi::TitleBarStyle::Hidden);
  window.SetContentSize({480, 320});
  Report(widget, content, "hidden");
  window.SetContentSize({560, 360});
  Report(widget, content, "resized");
  window.SetHasShadow(false);
  Report(widget, content, "shadow-off");
  window.SetHasShadow(true);
  gtk_widget_hide(widget);
  Settle();
  gtk_widget_show(widget);
  Report(widget, content, "remapped");
  window.Maximize();
  Report(widget, content, "maximized");
  window.Unmaximize();
  Report(widget, content, "restored");
  window.SetTitleBarStyle(nativeapi::TitleBarStyle::Normal);
  Report(widget, content, "normal");
  gtk_widget_destroy(widget);
  Settle();
}
