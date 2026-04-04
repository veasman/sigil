# Sigil

Sigil is a lean X11 screenshot tool for Arch-family systems.

## Current status

Implemented:

- `sigil full`
- `sigil select`
- copy PNG to clipboard by default
- optional file output via `--file`
- optional stdout output via `--stdout`

## UX

### `sigil full`
Capture the **monitor under the pointer** immediately.

Use `--all` to capture the **entire desktop** instead.

### `sigil select`
Interactive mode:

- **click** a window to capture it
- **drag** to capture an area
- **right click** to cancel
- **Escape** to cancel

## Platform

Supported target for now:

- Arch Linux
- Artix Linux

X11 only.

## Install

### Arch/Artix (makepkg)

```bash
git clone https://github.com/veasman/sigil.git
cd sigil
makepkg -si
```

This handles all dependencies automatically.

### Manual

Install build dependencies:

```bash
doas pacman -S base-devel pkgconf libx11 libxrandr libxrender libpng
```

Install runtime dependency for clipboard support:

```bash
doas pacman -S xclip
```

If `xclip` is missing, `--file` and `--stdout` still work.

Build and install:

```bash
make
./install.sh
```

Or for a system-wide install:

```bash
make
PREFIX=/usr/local ./install.sh
```

## Usage

Capture current monitor to clipboard:

```bash
sigil full
```

Capture full desktop:

```bash
sigil full --all
```

Select a window or area:

```bash
sigil select
```

Save too:

```bash
sigil select --file /tmp/shot.png
```

Write PNG to stdout:

```bash
sigil full --stdout > /tmp/shot.png
```

## Compositor notes (picom)

Sigil works well with picom. The selection overlay and selection box are
separate windows with distinct classes, so you can target them in your picom
config:

- `sigil-select` — the dark overlay
- `sigil-select-box` — the selection border (picom's `corner-radius` applies here)

If picom's blur is enabled, exclude both sigil windows to prevent blur from
appearing in your screenshots:

```conf
blur-background-exclude = [
    "class_g = 'sigil-select'",
    "class_g = 'sigil-select-box'"
];
```

## Exit codes

- `0` success
- `1` usage/runtime error or cancelled selection
