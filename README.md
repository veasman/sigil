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

## Build dependencies

Install on Arch/Artix:

```bash
doas pacman -S base-devel pkgconf libx11 libxrandr libxrender libpng
```

## Runtime dependency

Clipboard support uses `xclip`:

```bash
doas pacman -S xclip
```

If `xclip` is missing, `--file` and `--stdout` still work.

## Build

```bash
make
```

## Install

Local install:

```bash
./install.sh
```

System install:

```bash
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

## Exit codes

- `0` success
- `1` usage/runtime error or cancelled selection
