PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

CC ?= cc
PKG_CONFIG ?= pkg-config

TARGET := sigil
BUILD_DIR := build

SRC := \
	src/main.c \
	src/cli.c \
	src/capture.c \
	src/select.c \
	src/png_write.c \
	src/clipboard.c \
	src/util.c

OBJ := $(SRC:src/%.c=$(BUILD_DIR)/%.o)

WARN_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
OPT_CFLAGS ?= -O2
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L -Iinclude
CFLAGS ?=
LDFLAGS ?=

PKGS := x11 xrandr xrender libpng
PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PKGS) 2>/dev/null)
PKG_LIBS := $(shell $(PKG_CONFIG) --libs $(PKGS) 2>/dev/null)

.PHONY: all clean check deps-check runtime-check install uninstall pkg srcinfo

all: $(TARGET)

$(TARGET): check $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) $(PKG_LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(WARN_CFLAGS) $(OPT_CFLAGS) $(CFLAGS) $(PKG_CFLAGS) -c $< -o $@

check:
	@$(PKG_CONFIG) --exists $(PKGS) || { \
		echo "Missing required build dependencies."; \
		echo "On Arch/Artix install:"; \
		echo "  doas pacman -S base-devel pkgconf libx11 libxrandr libxrender libpng"; \
		exit 1; \
	}

deps-check: check
	@echo "build dependencies OK"

runtime-check:
	@command -v xclip >/dev/null 2>&1 || { \
		echo "Missing runtime dependency: xclip"; \
		echo "Install with:"; \
		echo "  doas pacman -S xclip"; \
		exit 1; \
	}
	@echo "runtime dependencies OK"

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 0755 "$(TARGET)" "$(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"

pkg:
	makepkg -fs

srcinfo:
	makepkg --printsrcinfo > .SRCINFO

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
