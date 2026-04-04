# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Sigil is a minimal X11 screenshot utility written in C11. It supports full-screen capture, interactive region/window selection, and outputs PNG to file, stdout, or clipboard (via xclip).

## Build Commands

```bash
make                # Build the binary (./sigil)
make clean          # Remove build artifacts
make deps-check     # Verify build dependencies are installed
make runtime-check  # Verify runtime dependencies (xclip)
make pkg            # Build Arch package via makepkg
make install        # Install to PREFIX (~/.local by default)
```

Build dependencies: `base-devel pkgconf libx11 libxrandr libxrender libpng`
Runtime dependency: `xclip`

There is no test suite or separate linter. Compiler warnings (`-Wall -Wextra -Wpedantic`) serve as the primary static analysis.

## Architecture

All source is in `src/` with headers in `include/`. The central data types live in `include/sigil.h` (`SigilMode`, `SigilRect`, `SigilImage`, `SigilOptions`).

**Module responsibilities:**

- **main.c** — Entry point. Parses CLI, opens X11 display, dispatches to capture mode, emits output.
- **cli.c** — Argument parsing and validation. Modes: `full`, `select`. Flags: `--all`, `--file`, `--stdout`, `--no-clipboard`.
- **capture.c** — Screenshot capture via X11/XRandr. Functions for full-screen, rectangle, and window capture. Converts X pixel data to RGBA.
- **select.c** — Interactive selection overlay. Creates a transparent X window with crosshair cursor, handles mouse/keyboard events. Uses a 6px drag threshold to distinguish click (window capture) from drag (region capture).
- **png_write.c** — PNG encoding via libpng. Supports file and stream (stdout) output.
- **clipboard.c** — Pipes PNG data to xclip subprocess.
- **util.c** — Error handling (`sigil_die`, `sigil_warn`) and command existence checks.

**Call flow:** `main` → `sigil_parse_cli` → `XOpenDisplay` → `run_full`/`run_select` → `sigil_capture_*` → `emit_image` (file/stdout/clipboard).

## Conventions

- All public functions and types are prefixed with `sigil_`.
- All structs are typedef'd.
- Error handling uses boolean returns and early exits; fatal errors go through `sigil_die()`.
- Memory is manually managed with explicit malloc/free pairing.
