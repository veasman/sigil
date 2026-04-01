#include "capture.h"
#include "clipboard.h"
#include "cli.h"
#include "png_write.h"
#include "select.h"
#include "sigil.h"
#include "util.h"

#include <X11/Xlib.h>

static int emit_image(const SigilOptions *opts, const SigilImage *img) {
    bool did_output = false;

    if (opts->output_file) {
        if (!sigil_write_png_file(opts->output_file, img)) {
            sigil_die("failed to write PNG file");
        }
        did_output = true;
    }

    if (opts->write_stdout) {
        if (!sigil_write_png_stream(stdout, img)) {
            sigil_die("failed to write PNG to stdout");
        }
        did_output = true;
    }

    if (opts->use_clipboard) {
        if (!sigil_copy_png_to_clipboard(img)) {
            sigil_warn("clipboard copy failed (is xclip installed?)");
            if (!did_output) {
                return 1;
            }
        } else {
            did_output = true;
        }
    }

    if (!did_output) {
        sigil_warn("nothing was done; use clipboard, --file, or --stdout");
        return 1;
    }

    return 0;
}

static int run_full(Display *dpy, const SigilOptions *opts) {
    SigilImage img = {0};

    if (!sigil_capture_full(dpy, opts->capture_all, &img)) {
        sigil_die("failed to capture full screen");
    }

    int rc = emit_image(opts, &img);
    sigil_image_free(&img);
    return rc;
}

static int run_select(Display *dpy, const SigilOptions *opts) {
    SigilSelection sel = {0};
    if (!sigil_select(dpy, &sel)) {
        sigil_warn("selection cancelled");
        return 1;
    }

    SigilImage img = {0};
    bool ok = false;

    switch (sel.kind) {
        case SIGIL_SELECT_AREA:
            ok = sigil_capture_rect(dpy, &sel.rect, &img);
            break;
        case SIGIL_SELECT_WINDOW:
            ok = sigil_capture_window(dpy, sel.window, &img);
            break;
        default:
            ok = false;
            break;
    }

    if (!ok) {
        sigil_die("failed to capture selection");
    }

    int rc = emit_image(opts, &img);
    sigil_image_free(&img);
    return rc;
}

int main(int argc, char **argv) {
    SigilOptions opts;
    if (!sigil_parse_cli(argc, argv, &opts)) {
        sigil_print_usage(stderr, argv[0]);
        return 1;
    }

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        sigil_die("failed to open X display");
    }

    int rc = 1;

    switch (opts.mode) {
        case SIGIL_MODE_FULL:
            rc = run_full(dpy, &opts);
            break;
        case SIGIL_MODE_SELECT:
            rc = run_select(dpy, &opts);
            break;
        default:
            sigil_print_usage(stderr, argv[0]);
            rc = 1;
            break;
    }

    XCloseDisplay(dpy);
    return rc;
}
