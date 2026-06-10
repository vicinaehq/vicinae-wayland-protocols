WAYLAND_SCANNER ?= wayland-scanner
PKG_CFLAGS := $(shell pkg-config --cflags wayland-client)
PKG_LIBS := $(shell pkg-config --libs wayland-client)
CFLAGS ?= -Wall -Wextra -O2
PROTO := ../staging/vicinae-hotkey/vicinae-hotkey-v1.xml

GEN := vicinae-hotkey-v1-client-protocol.h vicinae-hotkey-v1-protocol.c

vicinae-hotkey-demo: vicinae-hotkey-demo.c $(GEN)
	$(CC) $(CFLAGS) $(PKG_CFLAGS) -o $@ vicinae-hotkey-demo.c vicinae-hotkey-v1-protocol.c $(PKG_LIBS)

vicinae-hotkey-v1-client-protocol.h: $(PROTO)
	$(WAYLAND_SCANNER) client-header $< $@

vicinae-hotkey-v1-protocol.c: $(PROTO)
	$(WAYLAND_SCANNER) private-code $< $@

clean:
	rm -f vicinae-hotkey-demo $(GEN)

.PHONY: clean
