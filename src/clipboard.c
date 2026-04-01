#include "clipboard.h"
#include "png_write.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

bool sigil_copy_png_to_clipboard(const SigilImage *img) {
    if (!img || !img->data) {
        return false;
    }

    if (!sigil_command_exists("xclip")) {
        return false;
    }

    FILE *pipe = popen("xclip -selection clipboard -t image/png -i", "w");
    if (!pipe) {
        return false;
    }

    bool ok = sigil_write_png_stream(pipe, img);
    int rc = pclose(pipe);

    return ok && rc == 0;
}
