# ext-hotkey

A Wayland protocol that lets an application bind its own global hotkeys: it picks a key combination, requests to bind to it, and if successful, gets notified when it is pressed/released.

## At a glance

The whole API is one request and a handful of callbacks:

```c
static const uint32_t super_ctrl_b_mods =
    EXT_HOTKEY_MANAGER_V1_MODIFIERS_SUPER | EXT_HOTKEY_MANAGER_V1_MODIFIERS_CTRL;

static struct ext_hotkey_v1 *register_launcher_hotkey(
    struct ext_hotkey_manager_v1 *manager) {
    struct ext_hotkey_v1 *hotkey = ext_hotkey_manager_v1_bind(
        manager, XKB_KEY_b, super_ctrl_b_mods, NULL,
        "com.example.app", "Toggle the launcher");
    ext_hotkey_v1_add_listener(hotkey, &launcher_listener, NULL);
    return hotkey;
}

static void on_bound(void *data, struct ext_hotkey_v1 *hotkey) {
    set_hotkey_active(true);
}

static void on_pressed(void *data, struct ext_hotkey_v1 *hotkey,
                       uint32_t serial, uint32_t time) {
    toggle_launcher(serial);
}

static void on_released(void *data, struct ext_hotkey_v1 *hotkey,
                        uint32_t serial, uint32_t time) {
}

static void on_denied(void *data, struct ext_hotkey_v1 *hotkey,
                      uint32_t reason, const char *message) {
    printf("hotkey denied (reason %u): %s\n", reason, message);
    ext_hotkey_v1_destroy(hotkey);
}

static void on_revoked(void *data, struct ext_hotkey_v1 *hotkey,
                       uint32_t reason, const char *message) {
    set_hotkey_active(false);
    printf("hotkey revoked (reason %u): %s\n", reason, message);
    ext_hotkey_v1_destroy(hotkey);
}
```

To rebind, destroy the object and bind the new combination. There is no separate
reconfigure step. The full runnable client is in [`../../examples`](../../examples).

## The problem

On Windows (`RegisterHotKey`), macOS (`RegisterEventHotKey`) and X11 (`XGrabKey`)
an application registers a key combination with the OS and is notified when it is
pressed. Wayland has no general equivalent.
Applications that depend on global hotkeys (launchers, clipboard managers,
push-to-talk, media controls) either cannot offer the feature or resort to
workarounds.

## Why the existing options aren't enough

Wayland already has a couple of ways to register global shortcuts. They share one
design decision that does not fit this use case: the user picks the trigger, not
the application.

This is the opposite of what most applications expect, and forces on application developers an entirely new way to manage shortcuts that does
not necessarily feel intuitive to the user. Applications that are not designed with this in mind need to adapt their UX in order to support
this way of doing things. It is our belief that this is not a reasonable thing to expect from app developers in a world where the Linux desktop has so little market share.

### The desktop portal

xdg-desktop-portal's [GlobalShortcuts interface](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html)
has the application register a named action; the user or compositor then decides which key fires it.

This has several problems:
- The application can only set a preferred trigger, and has no guarantee it will ever be respected. The application doesn't know about it unless it queries the list of named shortcuts again.
- Upon querying the list of named shortcuts, the portal doesn't return a stable, parsable representation of the current keybind, which means applications cannot even reliably parse it and display it to the user. Only a human-readable description is provided.
- The application cannot initiate rebinds/removal of shortcuts from its own UX.
- Configuration has to be done in a compositor-specific UI, which a lot of standalone compositors don't even provide. Even for those that do, it breaks the application UX and developers have to fully rely on the compositor's way of handling shortcuts.
- Due to all these reasons, a global shortcut saved in an application's config file and reapplied on startup easily falls out of sync, with no reliable way to fix it.

### Compositor protocols

Hyprland's [global-shortcuts protocol](https://wayland.app/protocols/hyprland-global-shortcuts-v1)
and the long-running [action-binder RFC](https://gitlab.freedesktop.org/wayland/wayland-protocols/-/merge_requests/216) do the same thing as the portal and have about the same downsides.

## What this does instead

This protocol keeps shortcut configuration inside the application. Setting a
global shortcut is part of the application's own UI, and the protocol stays out
of the way.

The application requests a specific key combination, and the compositor either
accepts it or denies it with a reason. The application owns the binding and can
change it at any time from its own settings; nothing is persisted and no dialog is
shown. The compositor remains in control (it can deny a request, revoke a
binding, or apply any policy it wants), but the user is no longer required to
configure the trigger out-of-band. This matches the model used on Windows, macOS
and X11, adapted to Wayland's compositor-driven design.

The actual protocol (wire format, security, matching, activation) is in
[`ext-hotkey-v1.xml`](ext-hotkey-v1.xml), with a small example client in
[`../../examples`](../../examples).
