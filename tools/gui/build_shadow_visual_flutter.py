#!/usr/bin/env python3
"""Build #594 with a stock Flutter runner plus a read-only GTK measurement hook.

WM_RGBA=1 selects the documented pre-realization visual; otherwise the runner's
visual selection is unchanged. No native plugin or CSS workaround is installed.
"""
import json
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
APP = ROOT / 'core/build/shadow-visual-flutter'

HOOK = r'''
static GtkWindow* shadow_test_window = nullptr;
extern "C" G_MODULE_EXPORT int shadow_test_inspect(int fitted) {
  auto* widget = GTK_WIDGET(shadow_test_window);
  auto* content = gtk_bin_get_child(GTK_BIN(widget));
  auto* surface = gtk_widget_get_window(widget);
  auto* screen = gtk_widget_get_screen(widget);
  const bool alpha = gdk_window_get_visual(surface) == gdk_screen_get_rgba_visual(screen);
  int x = 0, y = 0;
  gtk_widget_translate_coordinates(content, widget, 0, 0, &x, &y);
  const int w = gtk_widget_get_allocated_width(content);
  const int h = gtk_widget_get_allocated_height(content);
  const int sw = gtk_widget_get_allocated_width(widget);
  const int sh = gtk_widget_get_allocated_height(widget);
  int errors = 0;
  g_print("SURFACE depth=%d alpha=%d content=%d,%d,%d,%d allocation=%d,%d\n",
          gdk_visual_get_depth(gdk_window_get_visual(surface)), alpha, x,y,w,h,sw,sh);
  if (!alpha) errors += x != 0 || y != 0 || w != sw || h != sh;
  else if (!fitted) errors += x < 132 || y < 132 || sw-x-w < 132 || sh-y-h < 132;
  if (!fitted) errors += w != 800 || h != 600;
  if (alpha && x > 0 && y > 0) {
    auto* image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sw, sh);
    auto* cr = cairo_create(image);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);
    cairo_surface_flush(image);
    const auto pixel = *reinterpret_cast<const guint32*>(cairo_image_surface_get_data(image));
    errors += (pixel >> 24) != 0;
    g_print("GUTTER alpha=%u\n", pixel >> 24);
    cairo_surface_destroy(image);
  }
  g_print("%s native GTK geometry and gutter\n", errors ? "FAIL" : "PASS");
  return errors;
}
'''

def main():
    flutter = shutil.which('flutter')
    assert flutter, 'Flutter must be on PATH'
    if not (APP / 'pubspec.yaml').exists():
        subprocess.run([flutter, 'create', '--empty', '--platforms=linux',
                        '--project-name=shadow_visual', str(APP)], check=True)
    pubspec = '''name: shadow_visual
environment:
  sdk: ^3.14.0
dependencies:
  flutter:
    sdk: flutter
  window_manager: 0.7.0
dependency_overrides:
'''
    for name in ('window_manager', 'nativeapi', 'nativeapi_flutter', 'cnativeapi'):
        pubspec += f'  {name}:\n    path: {json.dumps(str(ROOT / "bindings/dart" / name))}\n'
    (APP / 'pubspec.yaml').write_text(pubspec)
    shutil.copyfile(ROOT / 'tools/gui/window_shadow_visual_flutter.dart', APP / 'lib/main.dart')
    runner = APP / 'linux/runner/my_application.cc'
    original = runner.with_suffix('.cc.stock')
    if not original.exists():
        shutil.copyfile(runner, original)
    source = original.read_text()
    marker = 'static void my_application_activate(GApplication* application) {'
    assert marker in source
    source = source.replace(marker, HOOK + '\n' + marker, 1)
    marker = 'GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(application)));'
    assert marker in source
    source = source.replace(marker, marker + '''
  shadow_test_window = window;
  if (g_getenv("WM_RGBA")) {
    GdkScreen* screen = gtk_window_get_screen(window);
    GdkVisual* visual = gdk_screen_get_rgba_visual(screen);
    if (visual != nullptr && gdk_screen_is_composited(screen))
      gtk_widget_set_visual(GTK_WIDGET(window), visual);
  }
''', 1)
    runner.write_text(source)
    cmake = APP / 'linux/runner/CMakeLists.txt'
    export = 'target_link_options(${BINARY_NAME} PRIVATE "-Wl,--export-dynamic-symbol=shadow_test_inspect")'
    if export not in cmake.read_text():
        cmake.write_text(cmake.read_text() + '\n' + export + '\n')
    subprocess.run([flutter, 'pub', 'get'], cwd=APP, check=True)
    subprocess.run([flutter, 'build', 'linux', '--debug'], cwd=APP, check=True)
    print('BUNDLE', APP / 'build/linux/x64/debug/bundle', flush=True)

if __name__ == '__main__':
    main()
