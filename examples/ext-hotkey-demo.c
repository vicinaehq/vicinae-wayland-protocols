/*
 * Minimal ext-hotkey demo client.
 *
 * Requests a single global hotkey (Super+Ctrl+B) and prints the lifecycle and
 * press/release events. It requires a compositor that implements
 * ext_hotkey_manager_v1; without one it just reports that the global is
 * missing.
 *
 * Build with `make`
 */
#include "ext-hotkey-v1-client-protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

/* Keysyms use the X11/keysym numbering; for printable ASCII the value is the
 * ASCII code, so 0x062 is 'b' (XKB_KEY_b under the xkb_v1 keymap format). */
#define TRIGGER_KEYSYM 0x062 /* 'b' */
#define TRIGGER_MODS                                                           \
  (EXT_HOTKEY_MANAGER_V1_MODIFIERS_SUPER | EXT_HOTKEY_MANAGER_V1_MODIFIERS_CTRL)

static struct ext_hotkey_manager_v1 *manager;

static const char *reason_str(uint32_t reason) {
  switch (reason) {
  case EXT_HOTKEY_V1_REASON_ALREADY_BOUND:
    return "already_bound";
  case EXT_HOTKEY_V1_REASON_NOT_PERMITTED:
    return "not_permitted";
  case EXT_HOTKEY_V1_REASON_INVALID:
    return "invalid";
  default:
    return "unknown";
  }
}

static void hk_bound(void *data, struct ext_hotkey_v1 *hk) {
  (void)data;
  (void)hk;
  printf("bound: hotkey is active\n");
}

static void hk_denied(void *data, struct ext_hotkey_v1 *hk, uint32_t reason) {
  (void)data;
  (void)hk;
  printf("denied: %s\n", reason_str(reason));
}

static void hk_revoked(void *data, struct ext_hotkey_v1 *hk, uint32_t reason) {
  (void)data;
  (void)hk;
  printf("revoked: %s\n", reason_str(reason));
}

static void hk_pressed(void *data, struct ext_hotkey_v1 *hk, uint32_t serial,
                       uint32_t time) {
  (void)data;
  (void)hk;
  printf("pressed  (serial=%u, time=%u)\n", serial, time);
}

static void hk_released(void *data, struct ext_hotkey_v1 *hk, uint32_t serial,
                        uint32_t time) {
  (void)data;
  (void)hk;
  printf("released (serial=%u, time=%u)\n", serial, time);
}

static const struct ext_hotkey_v1_listener hk_listener = {
    .bound = hk_bound,
    .denied = hk_denied,
    .revoked = hk_revoked,
    .pressed = hk_pressed,
    .released = hk_released,
};

static void reg_global(void *data, struct wl_registry *reg, uint32_t name,
                       const char *interface, uint32_t version) {
  (void)data;
  (void)version;
  if (strcmp(interface, ext_hotkey_manager_v1_interface.name) == 0)
    manager = wl_registry_bind(reg, name, &ext_hotkey_manager_v1_interface, 1);
}

static void reg_global_remove(void *data, struct wl_registry *reg,
                              uint32_t name) {
  (void)data;
  (void)reg;
  (void)name;
}

static const struct wl_registry_listener reg_listener = {
    .global = reg_global,
    .global_remove = reg_global_remove,
};

int main(void) {
  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "failed to connect to a wayland display\n");
    return 1;
  }

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &reg_listener, NULL);
  wl_display_roundtrip(display);

  if (!manager) {
    fprintf(stderr, "compositor does not implement ext_hotkey_manager_v1\n");
    wl_display_disconnect(display);
    return 1;
  }

  struct ext_hotkey_v1 *hotkey = ext_hotkey_manager_v1_bind(
      manager, TRIGGER_KEYSYM, TRIGGER_MODS, NULL /* all seats */,
      "com.example.ext-hotkey-demo", "Demo hotkey");
  ext_hotkey_v1_add_listener(hotkey, &hk_listener, NULL);

  printf("requested Super+Ctrl+B; waiting for events (Ctrl-C to quit)...\n");

  while (wl_display_dispatch(display) != -1)
    ;

  ext_hotkey_v1_destroy(hotkey);
  ext_hotkey_manager_v1_destroy(manager);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);
  return 0;
}
