#ifndef SIGIL_CAPTURE_H
#define SIGIL_CAPTURE_H

#include "sigil.h"

bool sigil_capture_full(Display *dpy, bool capture_all, SigilImage *out);
bool sigil_capture_rect(Display *dpy, const SigilRect *rect, SigilImage *out);
bool sigil_capture_window(Display *dpy, Window win, SigilImage *out);

void sigil_image_free(SigilImage *img);

#endif
