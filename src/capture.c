#include "capture.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static unsigned char extract_channel(unsigned long pixel, unsigned long mask) {
    if (mask == 0) {
        return 0;
    }

    int shift = 0;
    while (((mask >> shift) & 1UL) == 0UL) {
        shift++;
    }

    unsigned long value = (pixel & mask) >> shift;
    unsigned long max = mask >> shift;

    if (max == 0UL) {
        return 0;
    }

    return (unsigned char)((value * 255UL) / max);
}

static bool sigil_monitor_under_pointer(Display *dpy, SigilRect *out) {
    if (!dpy || !out) {
        return false;
    }

    Window root = DefaultRootWindow(dpy);

    Window root_ret = None;
    Window child_ret = None;
    int root_x = 0;
    int root_y = 0;
    int win_x = 0;
    int win_y = 0;
    unsigned int mask = 0;

    if (!XQueryPointer(
            dpy,
            root,
            &root_ret,
            &child_ret,
            &root_x,
            &root_y,
            &win_x,
            &win_y,
            &mask)) {
        return false;
    }

    int monitor_count = 0;
    XRRMonitorInfo *monitors = XRRGetMonitors(dpy, root, True, &monitor_count);
    if (!monitors || monitor_count <= 0) {
        if (monitors) {
            XRRFreeMonitors(monitors);
        }
        return false;
    }

    bool found = false;

    for (int i = 0; i < monitor_count; i++) {
        XRRMonitorInfo *m = &monitors[i];
        if (root_x >= m->x &&
            root_x < (m->x + m->width) &&
            root_y >= m->y &&
            root_y < (m->y + m->height)) {
            out->x = m->x;
            out->y = m->y;
            out->width = (unsigned int)m->width;
            out->height = (unsigned int)m->height;
            found = true;
            break;
        }
    }

    XRRFreeMonitors(monitors);
    return found;
}

bool sigil_capture_rect(Display *dpy, const SigilRect *rect, SigilImage *out) {
    if (!dpy || !rect || !out || rect->width == 0 || rect->height == 0) {
        return false;
    }

    Screen *screen = DefaultScreenOfDisplay(dpy);
    if (!screen) {
        return false;
    }

    Window root = RootWindowOfScreen(screen);

    XImage *ximg = XGetImage(
        dpy,
        root,
        rect->x,
        rect->y,
        rect->width,
        rect->height,
        AllPlanes,
        ZPixmap
    );
    if (!ximg) {
        return false;
    }

    size_t size = (size_t)rect->width * (size_t)rect->height * 4u;
    unsigned char *rgba = calloc(1, size);
    if (!rgba) {
        XDestroyImage(ximg);
        return false;
    }

    for (unsigned int yy = 0; yy < rect->height; yy++) {
        for (unsigned int xx = 0; xx < rect->width; xx++) {
            unsigned long pixel = XGetPixel(ximg, (int)xx, (int)yy);

            unsigned char r = extract_channel(pixel, ximg->red_mask);
            unsigned char g = extract_channel(pixel, ximg->green_mask);
            unsigned char b = extract_channel(pixel, ximg->blue_mask);

            size_t idx = ((size_t)yy * rect->width + xx) * 4u;
            rgba[idx + 0] = r;
            rgba[idx + 1] = g;
            rgba[idx + 2] = b;
            rgba[idx + 3] = 255;
        }
    }

    XDestroyImage(ximg);

    out->data = rgba;
    out->width = rect->width;
    out->height = rect->height;
    return true;
}

bool sigil_capture_full(Display *dpy, bool capture_all, SigilImage *out) {
    if (!dpy || !out) {
        return false;
    }

    Screen *screen = DefaultScreenOfDisplay(dpy);
    if (!screen) {
        return false;
    }

    SigilRect rect = {0};

    if (!capture_all && sigil_monitor_under_pointer(dpy, &rect)) {
        return sigil_capture_rect(dpy, &rect, out);
    }

    rect.x = 0;
    rect.y = 0;
    rect.width = (unsigned int)WidthOfScreen(screen);
    rect.height = (unsigned int)HeightOfScreen(screen);

    return sigil_capture_rect(dpy, &rect, out);
}

bool sigil_capture_window(Display *dpy, Window win, SigilImage *out) {
    if (!dpy || !win || !out) {
        return false;
    }

    XWindowAttributes attr;
    if (!XGetWindowAttributes(dpy, win, &attr)) {
        return false;
    }

    if (attr.width <= 0 || attr.height <= 0) {
        return false;
    }

    Window root = DefaultRootWindow(dpy);
    int root_x = 0;
    int root_y = 0;
    Window child = None;

    if (!XTranslateCoordinates(dpy, win, root, 0, 0, &root_x, &root_y, &child)) {
        return false;
    }

    SigilRect rect = {
        .x = root_x,
        .y = root_y,
        .width = (unsigned int)attr.width,
        .height = (unsigned int)attr.height,
    };

    return sigil_capture_rect(dpy, &rect, out);
}

void sigil_image_free(SigilImage *img) {
    if (!img) {
        return;
    }

    free(img->data);
    img->data = NULL;
    img->width = 0;
    img->height = 0;
}
