#include "select.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrender.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIGIL_DRAG_THRESHOLD 6
#define SIGIL_OVERLAY_ALPHA  0x44
#define SIGIL_SELECT_BORDER  2

static void normalize_rect(int x1, int y1, int x2, int y2, SigilRect *out) {
    int left = (x1 < x2) ? x1 : x2;
    int top = (y1 < y2) ? y1 : y2;
    int right = (x1 > x2) ? x1 : x2;
    int bottom = (y1 > y2) ? y1 : y2;

    out->x = left;
    out->y = top;
    out->width = (unsigned int)(right - left);
    out->height = (unsigned int)(bottom - top);
}

static bool movement_exceeds_threshold(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) >= SIGIL_DRAG_THRESHOLD || abs(y2 - y1) >= SIGIL_DRAG_THRESHOLD;
}

static Visual *find_argb_visual(Display *dpy, int screen, int *depth_out) {
    XVisualInfo tpl;
    memset(&tpl, 0, sizeof(tpl));
    tpl.screen = screen;
    tpl.depth = 32;
    tpl.class = TrueColor;

    int nitems = 0;
    XVisualInfo *infos = XGetVisualInfo(
        dpy,
        VisualScreenMask | VisualDepthMask | VisualClassMask,
        &tpl,
        &nitems
    );
    if (!infos) {
        return NULL;
    }

    Visual *result = NULL;
    int result_depth = 0;

    for (int i = 0; i < nitems; i++) {
        XRenderPictFormat *fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
        if (!fmt) {
            continue;
        }

        if (fmt->type == PictTypeDirect && fmt->direct.alphaMask != 0) {
            result = infos[i].visual;
            result_depth = infos[i].depth;
            break;
        }
    }

    XFree(infos);

    if (result && depth_out) {
        *depth_out = result_depth;
    }

    return result;
}

static void set_overlay_metadata(Display *dpy, Window win) {
    XStoreName(dpy, win, "sigil-select");

    XClassHint hint;
    hint.res_name = (char *)"sigil-select";
    hint.res_class = (char *)"sigil-select";
    XSetClassHint(dpy, win, &hint);
}

static Window create_overlay(Display *dpy, int screen, int width, int height) {
    int depth = 0;
    Visual *visual = find_argb_visual(dpy, screen, &depth);
    if (!visual || depth != 32) {
        return None;
    }

    Window root = RootWindow(dpy, screen);
    Colormap cmap = XCreateColormap(dpy, root, visual, AllocNone);

    XSetWindowAttributes attrs;
    memset(&attrs, 0, sizeof(attrs));
    attrs.override_redirect = True;
    attrs.colormap = cmap;
    attrs.border_pixel = 0;
    attrs.background_pixel = 0;
    attrs.event_mask =
        ExposureMask |
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask |
        KeyPressMask;

    unsigned long mask =
        CWOverrideRedirect |
        CWColormap |
        CWBorderPixel |
        CWBackPixel |
        CWEventMask;

    Window win = XCreateWindow(
        dpy,
        root,
        0,
        0,
        (unsigned int)width,
        (unsigned int)height,
        0,
        depth,
        InputOutput,
        visual,
        mask,
        &attrs
    );

    if (!win) {
        XFreeColormap(dpy, cmap);
        return None;
    }

    set_overlay_metadata(dpy, win);
    XMapRaised(dpy, win);
    XFlush(dpy);
    return win;
}

static void paint_overlay(Display *dpy, Picture pic,
                          int screen_w, int screen_h,
                          int x1, int y1, int x2, int y2) {
    XRenderColor clear = {0, 0, 0, 0};
    XRenderColor tint = {0, 0, 0, SIGIL_OVERLAY_ALPHA * 257};

    /* clear entire overlay to transparent */
    XRenderFillRectangle(dpy, PictOpSrc, pic, &clear,
                         0, 0, (unsigned int)screen_w, (unsigned int)screen_h);

    int left = (x1 < x2) ? x1 : x2;
    int top = (y1 < y2) ? y1 : y2;
    unsigned int width = (unsigned int)abs(x2 - x1);
    unsigned int height = (unsigned int)abs(y2 - y1);

    if (width == 0 || height == 0) {
        /* no selection yet, tint the whole screen */
        XRenderFillRectangle(dpy, PictOpSrc, pic, &tint,
                             0, 0, (unsigned int)screen_w, (unsigned int)screen_h);
        return;
    }

    /* top */
    if (top > 0) {
        XRenderFillRectangle(dpy, PictOpSrc, pic, &tint,
                             0, 0, (unsigned int)screen_w, (unsigned int)top);
    }
    /* bottom */
    if ((int)(top + height) < screen_h) {
        XRenderFillRectangle(dpy, PictOpSrc, pic, &tint,
                             0, (int)(top + height),
                             (unsigned int)screen_w,
                             (unsigned int)(screen_h - top - (int)height));
    }
    /* left */
    if (left > 0) {
        XRenderFillRectangle(dpy, PictOpSrc, pic, &tint,
                             0, top, (unsigned int)left, height);
    }
    /* right */
    if ((int)(left + (int)width) < screen_w) {
        XRenderFillRectangle(dpy, PictOpSrc, pic, &tint,
                             (int)(left + (int)width), top,
                             (unsigned int)(screen_w - left - (int)width), height);
    }
}

static Window create_select_box(Display *dpy, int screen) {
    int depth = 0;
    Visual *visual = find_argb_visual(dpy, screen, &depth);
    if (!visual || depth != 32) {
        return None;
    }

    Window root = RootWindow(dpy, screen);
    Colormap cmap = XCreateColormap(dpy, root, visual, AllocNone);

    XSetWindowAttributes attrs;
    memset(&attrs, 0, sizeof(attrs));
    attrs.override_redirect = True;
    attrs.colormap = cmap;
    attrs.border_pixel = 0xFFFFFFFFUL;
    attrs.background_pixel = 0;
    attrs.event_mask = 0;

    unsigned long mask =
        CWOverrideRedirect |
        CWColormap |
        CWBorderPixel |
        CWBackPixel |
        CWEventMask;

    Window win = XCreateWindow(
        dpy,
        root,
        0, 0, 1, 1,
        SIGIL_SELECT_BORDER,
        depth,
        InputOutput,
        visual,
        mask,
        &attrs
    );

    if (win) {
        XClassHint hint;
        hint.res_name = (char *)"sigil-select-box";
        hint.res_class = (char *)"sigil-select-box";
        XSetClassHint(dpy, win, &hint);
    }

    return win;
}

static void update_select_box(Display *dpy, Window box,
                              int x1, int y1, int x2, int y2, bool *mapped) {
    int left = (x1 < x2) ? x1 : x2;
    int top = (y1 < y2) ? y1 : y2;
    unsigned int width = (unsigned int)abs(x2 - x1);
    unsigned int height = (unsigned int)abs(y2 - y1);

    if (width == 0 || height == 0) {
        if (*mapped) {
            XUnmapWindow(dpy, box);
            *mapped = false;
        }
        return;
    }

    XMoveResizeWindow(dpy, box, left, top, width, height);

    if (!*mapped) {
        XMapRaised(dpy, box);
        *mapped = true;
    }
}

static Window resolve_click_target(Display *dpy, Window root, Window target) {
    if (!dpy || target == None || target == root) {
        return None;
    }

    Window current = target;

    while (1) {
        Window root_ret = None;
        Window parent_ret = None;
        Window *children = NULL;
        unsigned int nchildren = 0;

        if (!XQueryTree(dpy, current, &root_ret, &parent_ret, &children, &nchildren)) {
            if (children) {
                XFree(children);
            }
            break;
        }

        if (children) {
            XFree(children);
        }

        if (parent_ret == None || parent_ret == root) {
            return current;
        }

        current = parent_ret;
    }

    return target;
}

static void settle_after_overlay(Display *dpy) {
    XSync(dpy, False);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 150 * 1000 * 1000;
    nanosleep(&ts, NULL);

    XSync(dpy, False);
}

bool sigil_select(Display *dpy, SigilSelection *out) {
    if (!dpy || !out) {
        return false;
    }

    memset(out, 0, sizeof(*out));

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int screen_w = DisplayWidth(dpy, screen);
    int screen_h = DisplayHeight(dpy, screen);

    Window overlay = create_overlay(dpy, screen, screen_w, screen_h);
    if (overlay == None) {
        return false;
    }

    XRenderPictFormat *fmt = XRenderFindVisualFormat(dpy, find_argb_visual(dpy, screen, NULL));
    XRenderPictureAttributes pa;
    memset(&pa, 0, sizeof(pa));
    Picture overlay_pic = XRenderCreatePicture(dpy, overlay, fmt, 0, &pa);

    /* paint initial full-screen tint */
    paint_overlay(dpy, overlay_pic, screen_w, screen_h, 0, 0, 0, 0);
    XFlush(dpy);

    Window select_box = create_select_box(dpy, screen);
    bool box_mapped = false;

    Cursor cursor = XCreateFontCursor(dpy, XC_crosshair);

    int pgrab = XGrabPointer(
        dpy,
        overlay,
        False,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        cursor,
        CurrentTime
    );

    if (pgrab != GrabSuccess) {
        if (select_box != None) {
            XDestroyWindow(dpy, select_box);
        }
        XDestroyWindow(dpy, overlay);
        if (cursor != None) {
            XFreeCursor(dpy, cursor);
        }
        return false;
    }

    bool keyboard_grabbed = false;
    int kgrab = XGrabKeyboard(
        dpy,
        overlay,
        False,
        GrabModeAsync,
        GrabModeAsync,
        CurrentTime
    );
    if (kgrab == GrabSuccess) {
        keyboard_grabbed = true;
    }

    bool pressed = false;
    bool dragging = false;
    bool ok = false;

    int start_x = 0;
    int start_y = 0;
    int last_x = 0;
    int last_y = 0;
    Window press_window = None;

    XEvent ev;
    while (1) {
        XNextEvent(dpy, &ev);

        if (ev.type == Expose) {
            if (dragging) {
                paint_overlay(dpy, overlay_pic, screen_w, screen_h,
                              start_x, start_y, last_x, last_y);
            } else {
                paint_overlay(dpy, overlay_pic, screen_w, screen_h,
                              0, 0, 0, 0);
            }
            XFlush(dpy);
            continue;
        }

        if (ev.type == KeyPress && keyboard_grabbed) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            if (ks == XK_Escape) {
                break;
            }
            continue;
        }

        if (ev.type == ButtonPress) {
            if (ev.xbutton.button != Button1) {
                break;
            }

            pressed = true;
            dragging = false;

            start_x = ev.xbutton.x_root;
            start_y = ev.xbutton.y_root;
            last_x = start_x;
            last_y = start_y;

            Window clicked = ev.xbutton.subwindow;
            if (clicked == None || clicked == overlay) {
                clicked = root;
            }
            press_window = resolve_click_target(dpy, root, clicked);
            continue;
        }

        if (ev.type == MotionNotify && pressed) {
            while (XCheckTypedEvent(dpy, MotionNotify, &ev)) {
                /* compress motion */
            }

            int cur_x = ev.xmotion.x_root;
            int cur_y = ev.xmotion.y_root;

            if (!dragging && movement_exceeds_threshold(start_x, start_y, cur_x, cur_y)) {
                dragging = true;
            }

            last_x = cur_x;
            last_y = cur_y;

            if (dragging) {
                paint_overlay(dpy, overlay_pic, screen_w, screen_h,
                              start_x, start_y, last_x, last_y);
                if (select_box != None) {
                    update_select_box(dpy, select_box,
                                      start_x, start_y, last_x, last_y,
                                      &box_mapped);
                }
                XFlush(dpy);
            }
            continue;
        }

        if (ev.type == ButtonRelease && pressed) {
            if (ev.xbutton.button != Button1) {
                break;
            }

            if (dragging) {
                normalize_rect(start_x, start_y, ev.xbutton.x_root, ev.xbutton.y_root, &out->rect);
                if (out->rect.width > 0 && out->rect.height > 0) {
                    out->kind = SIGIL_SELECT_AREA;
                    ok = true;
                }
            } else {
                if (press_window != None && press_window != root) {
                    out->kind = SIGIL_SELECT_WINDOW;
                    out->window = press_window;
                    ok = true;
                }
            }
            break;
        }
    }

    if (keyboard_grabbed) {
        XUngrabKeyboard(dpy, CurrentTime);
    }

    XUngrabPointer(dpy, CurrentTime);

    XRenderFreePicture(dpy, overlay_pic);

    if (select_box != None) {
        XDestroyWindow(dpy, select_box);
    }
    XDestroyWindow(dpy, overlay);

    if (cursor != None) {
        XFreeCursor(dpy, cursor);
    }

    settle_after_overlay(dpy);
    return ok;
}
