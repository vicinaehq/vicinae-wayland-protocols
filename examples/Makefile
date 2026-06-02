WAYLAND_SCANNER ?= wayland-scanner
PKG_CFLAGS := $(shell pkg-config --cflags wayland-client)
PKG_LIBS := $(shell pkg-config --libs wayland-client)
CFLAGS ?= -Wall -Wextra -O2
PROTO := ../staging/ext-hotkey/ext-hotkey-v1.xml

GEN := ext-hotkey-v1-client-protocol.h ext-hotkey-v1-protocol.c

ext-hotkey-demo: ext-hotkey-demo.c $(GEN)
	$(CC) $(CFLAGS) $(PKG_CFLAGS) -o $@ ext-hotkey-demo.c ext-hotkey-v1-protocol.c $(PKG_LIBS)

ext-hotkey-v1-client-protocol.h: $(PROTO)
	$(WAYLAND_SCANNER) client-header $< $@

ext-hotkey-v1-protocol.c: $(PROTO)
	$(WAYLAND_SCANNER) private-code $< $@

clean:
	rm -f ext-hotkey-demo $(GEN)

.PHONY: clean
