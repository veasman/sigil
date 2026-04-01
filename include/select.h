#ifndef SIGIL_SELECT_H
#define SIGIL_SELECT_H

#include "sigil.h"

typedef enum {
    SIGIL_SELECT_NONE = 0,
    SIGIL_SELECT_AREA,
    SIGIL_SELECT_WINDOW,
} SigilSelectKind;

typedef struct {
    SigilSelectKind kind;
    SigilRect rect;
    Window window;
} SigilSelection;

bool sigil_select(Display *dpy, SigilSelection *out);

#endif
