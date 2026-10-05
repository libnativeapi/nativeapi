// wlpointer — press, release or click a mouse button, or move the pointer, through the
// compositor's zwlr_virtual_pointer_manager_v1 (Hyprland, sway and other wlroots-based
// compositors; GNOME has no such global, use the Mutter RemoteDesktop driver there).
// The event lands wherever the real cursor is: position it first (Hyprland:
// `hyprctl dispatch movecursor X Y`) or with `move`.
//
//   wlpointer click [left|right|middle]     press + release, 60 ms apart
//   wlpointer press|release [button]
//   wlpointer move X Y EXTENT_W EXTENT_H    absolute motion, X/Y in 0..EXTENT
//
// Build (next to wlr-virtual-pointer-unstable-v1.xml):
//   wayland-scanner client-header wlr-virtual-pointer-unstable-v1.xml wlr-virtual-pointer-client.h
//   wayland-scanner private-code  wlr-virtual-pointer-unstable-v1.xml wlr-virtual-pointer.c
//   cc -O1 -o wlpointer wlpointer.c wlr-virtual-pointer.c -lwayland-client
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <wayland-client.h>

#include "wlr-virtual-pointer-client.h"

static struct wl_seat* seat;
static struct zwlr_virtual_pointer_manager_v1* manager;

static void on_global(void* data, struct wl_registry* registry, uint32_t name,
                      const char* interface, uint32_t version) {
  (void)data;
  (void)version;
  if (!seat && strcmp(interface, wl_seat_interface.name) == 0) {
    seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
  } else if (strcmp(interface, zwlr_virtual_pointer_manager_v1_interface.name) == 0) {
    manager = wl_registry_bind(registry, name, &zwlr_virtual_pointer_manager_v1_interface, 1);
  }
}

static void on_global_remove(void* data, struct wl_registry* registry, uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static uint32_t now_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static int usage(void) {
  fprintf(stderr,
          "usage: wlpointer click|press|release [left|right|middle]\n"
          "       wlpointer move X Y EXTENT_W EXTENT_H\n");
  return 64;
}

int main(int argc, char** argv) {
  const char* cmd = argc > 1 ? argv[1] : "";
  int is_click = strcmp(cmd, "click") == 0;
  int is_press = strcmp(cmd, "press") == 0;
  int is_release = strcmp(cmd, "release") == 0;
  int is_move = strcmp(cmd, "move") == 0;
  if (!(is_click || is_press || is_release || (is_move && argc >= 6))) return usage();

  struct wl_display* display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "wlpointer: cannot connect to the Wayland display\n");
    return 2;
  }
  struct wl_registry* registry = wl_display_get_registry(display);
  static const struct wl_registry_listener listener = {on_global, on_global_remove};
  wl_registry_add_listener(registry, &listener, NULL);
  wl_display_roundtrip(display);
  if (!manager) {
    fprintf(stderr, "wlpointer: the compositor has no zwlr_virtual_pointer_manager_v1\n");
    return 3;
  }
  struct zwlr_virtual_pointer_v1* pointer =
      zwlr_virtual_pointer_manager_v1_create_virtual_pointer(manager, seat);

  if (is_move) {
    zwlr_virtual_pointer_v1_motion_absolute(pointer, now_ms(), (uint32_t)atoi(argv[2]),
                                            (uint32_t)atoi(argv[3]), (uint32_t)atoi(argv[4]),
                                            (uint32_t)atoi(argv[5]));
    zwlr_virtual_pointer_v1_frame(pointer);
  } else {
    uint32_t button = BTN_LEFT;
    if (argc > 2) {
      if (strcmp(argv[2], "right") == 0) button = BTN_RIGHT;
      else if (strcmp(argv[2], "middle") == 0) button = BTN_MIDDLE;
      else if (strcmp(argv[2], "left") != 0) return usage();
    }
    if (is_click || is_press) {
      zwlr_virtual_pointer_v1_button(pointer, now_ms(), button, WL_POINTER_BUTTON_STATE_PRESSED);
      zwlr_virtual_pointer_v1_frame(pointer);
      wl_display_flush(display);
      if (is_click) usleep(60 * 1000);
    }
    if (is_click || is_release) {
      zwlr_virtual_pointer_v1_button(pointer, now_ms(), button, WL_POINTER_BUTTON_STATE_RELEASED);
      zwlr_virtual_pointer_v1_frame(pointer);
    }
  }
  wl_display_roundtrip(display);
  zwlr_virtual_pointer_v1_destroy(pointer);
  wl_display_disconnect(display);
  return 0;
}
