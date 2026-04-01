#ifndef SIGIL_H
#define SIGIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef enum {
    SIGIL_MODE_NONE = 0,
    SIGIL_MODE_FULL,
    SIGIL_MODE_SELECT,
} SigilMode;

typedef struct {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
} SigilRect;

typedef struct {
    unsigned char *data;
    unsigned int width;
    unsigned int height;
} SigilImage;

typedef struct {
    SigilMode mode;
    bool use_clipboard;
    bool write_stdout;
    bool capture_all;
    const char *output_file;
} SigilOptions;

#endif
