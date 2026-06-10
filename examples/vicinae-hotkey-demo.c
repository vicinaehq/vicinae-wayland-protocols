/*
 * Minimal vicinae-hotkey demo client.
 *
 * Requests a single global hotkey (Super+Ctrl+B) and prints the lifecycle and
 * press/release events. It requires a compositor that implements
 * vicinae_hotkey_manager_v1; without one it just reports that the global is
 * missing.
 *
 * Build with `make`
 */
#include "vicinae-hotkey-v1-client-protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

static struct vicinae_hotkey_manager_v1 *manager;

static const char *deny_reason_str(uint32_t reason) {
  switch (reason) {
  case VICINAE_HOTKEY_V1_DENY_REASON_ALREADY_BOUND:
    return "already_bound";
  case VICINAE_HOTKEY_V1_DENY_REASON_NOT_PERMITTED:
    return "not_permitted";
  case VICINAE_HOTKEY_V1_DENY_REASON_INVALID:
    return "invalid";
  default:
    return "unknown";
  }
}

static const char *revoke_reason_str(uint32_t reason) {
  switch (reason) {
  case VICINAE_HOTKEY_V1_REVOKE_REASON_REMOVED:
    return "removed";
  case VICINAE_HOTKEY_V1_REVOKE_REASON_SUPERSEDED:
    return "superseded";
  case VICINAE_HOTKEY_V1_REVOKE_REASON_NOT_PERMITTED:
    return "not_permitted";
  default:
    return "unknown";
  }
}

static void hk_bound(void *data, struct vicinae_hotkey_v1 *hk) {
  (void)data;
  (void)hk;
  printf("bound: hotkey is active\n");
}

static void hk_denied(void *data, struct vicinae_hotkey_v1 *hk, uint32_t reason,
                      const char *message) {
  (void)data;
  (void)hk;
  if (message && message[0])
    printf("denied: %s (%s)\n", deny_reason_str(reason), message);
  else
    printf("denied: %s\n", deny_reason_str(reason));
}

static void hk_revoked(void *data, struct vicinae_hotkey_v1 *hk, uint32_t reason,
                       const char *message) {
  (void)data;
  (void)hk;
  if (message && message[0])
    printf("revoked: %s (%s)\n", revoke_reason_str(reason), message);
  else
    printf("revoked: %s\n", revoke_reason_str(reason));
}

static void hk_pressed(void *data, struct vicinae_hotkey_v1 *hk, uint32_t serial,
                       uint32_t time) {
  (void)data;
  (void)hk;
  printf("pressed  (serial=%u, time=%u)\n", serial, time);
}

static void hk_released(void *data, struct vicinae_hotkey_v1 *hk, uint32_t serial,
                        uint32_t time) {
  (void)data;
  (void)hk;
  printf("released (serial=%u, time=%u)\n", serial, time);
}

static const struct vicinae_hotkey_v1_listener hk_listener = {
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
  if (strcmp(interface, vicinae_hotkey_manager_v1_interface.name) == 0)
    manager = wl_registry_bind(reg, name, &vicinae_hotkey_manager_v1_interface, 1);
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
    fprintf(stderr, "compositor does not implement vicinae_hotkey_manager_v1\n");
    wl_display_disconnect(display);
    return 1;
  }

  struct vicinae_hotkey_v1 *hotkey = vicinae_hotkey_manager_v1_bind(
      manager, XKB_KEY_space, VICINAE_HOTKEY_MANAGER_V1_MODIFIERS_SUPER,
      NULL /* all seats */, "com.example.vicinae-hotkey-demo", "Demo hotkey");

  vicinae_hotkey_v1_add_listener(hotkey, &hk_listener, NULL);

  printf("requested Super+Space; waiting for events (Ctrl-C to quit)...\n");

  while (wl_display_dispatch(display) != -1)
    ;

  vicinae_hotkey_v1_destroy(hotkey);
  vicinae_hotkey_manager_v1_destroy(manager);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);
  return 0;
}
