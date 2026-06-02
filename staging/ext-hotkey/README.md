# ext-hotkey

A small Wayland protocol that lets an application bind its own global hotkeys:
pick a key combination, get told when it's pressed. The same thing apps already
do on Windows, macOS and X11.

## The problem

Everywhere else this is a non-issue. The app says "let me know when Super+Space is
pressed" and the OS does it. Wayland took that away and never gave back an
equivalent, so a lot of apps that rely on it (launchers, clipboard managers,
push-to-talk, media controls) either can't offer the feature or have to fake it
with ugly workarounds. Vicinae runs into it constantly: you can pop the launcher
open, but you can't put a hotkey directly on one of its commands.

## Why the existing options aren't enough

Wayland does have a couple of ways to register "global shortcuts" today, and they
all make the same call, which is the one that breaks things: the user picks the
trigger, not the app.

### The desktop portal

xdg-desktop-portal's [GlobalShortcuts interface](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html)
has the app register a named action and then lets the user (or the compositor)
decide which key fires it. So the app
can't ship a sane default, can't offer its own "change this shortcut" setting, and
can't really remove a binding once it exists, since it outlives the app. Rebinding
usually throws up a system dialog, and support across compositors is spotty
anyway.

### Compositor protocols

Hyprland's [global-shortcuts protocol](https://wayland.app/protocols/hyprland-global-shortcuts-v1)
and the long-running [action-binder RFC](https://gitlab.freedesktop.org/wayland/wayland-protocols/-/merge_requests/216)
are the same idea: the app exposes anonymous actions and the user binds them to
keys in compositor config. The app still doesn't own its keys, so it's stuck with the
same limitations.

The real cost of all this is on the app's UX. The user has to leave the app to
set a shortcut up (edit a compositor config file, or click through a system
dialog), and the app can't present the simple "click here, press your keys" box it
shows on every other platform. The feature ends up feeling broken or second-class,
which is exactly the kind of thing that makes people write Linux off.

## What this does instead

The goal is to keep all of that inside the app and out of the user's way: setting
a global shortcut should be a normal part of the app's own UI, and the protocol
itself should be invisible.

So the app asks for a specific key combination and the compositor either accepts
it or says no, with a reason. The app owns the binding and can change it whenever,
straight from its own settings, with nothing left behind and no dialog in the
user's face. The compositor still calls the shots (it can refuse, revoke, or apply
whatever policy it wants), it just no longer forces the user to be the middleman.
Same model as Windows, macOS and X11, fit to Wayland's compositor-is-in-charge
world.

The actual protocol (wire format, security, matching, activation) is in
[`ext-hotkey-v1.xml`](ext-hotkey-v1.xml), with a small example client in
[`../examples`](../examples).
