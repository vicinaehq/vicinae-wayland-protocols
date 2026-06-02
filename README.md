# vicinae-wayland-protocols

Wayland protocol proposals by [Vicinae](https://vicinae.com), laid out
like [wayland-protocols](https://gitlab.freedesktop.org/wayland/wayland-protocols)
(`staging/<name>/`) for easy upstreaming.

> Status: drafts / RFCs. Not yet submitted upstream.

## Protocols

| Protocol | Status | Summary |
|---|---|---|
| [`ext-hotkey`](staging/ext-hotkey/) | Draft | Client-managed global hotkeys: the client chooses & reconfigures its own key combos (the Windows/macOS/X11 model, a drop-in for cross-platform apps), while the compositor stays the arbiter (accept/deny/revoke). |

## Why

Vicinae needs deep compositor integration to be fully useful, and Wayland holds it back more than any other platform does. So rather than just complain, we propose alternatives.

## License

MIT (see [LICENSE](LICENSE)); protocol XML files carry their own header.
