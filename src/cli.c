#include "cli.h"

#include <string.h>

void sigil_print_usage(FILE *stream, const char *argv0) {
    fprintf(stream,
        "Usage:\n"
        "  %s full [--all] [--file PATH] [--stdout] [--no-clipboard]\n"
        "  %s select [--file PATH] [--stdout] [--no-clipboard]\n"
        "\n"
        "Modes:\n"
        "  full    capture monitor under pointer\n"
        "  select  click a window or drag an area\n"
        "\n"
        "Flags:\n"
        "  --all            capture full desktop in full mode\n"
        "  --file PATH      save PNG to file\n"
        "  --stdout         write PNG to stdout\n"
        "  --no-clipboard   skip clipboard copy\n",
        argv0, argv0);
}

bool sigil_parse_cli(int argc, char **argv, SigilOptions *out) {
    if (!out || argc < 2) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    out->use_clipboard = true;

    if (strcmp(argv[1], "full") == 0) {
        out->mode = SIGIL_MODE_FULL;
    } else if (strcmp(argv[1], "select") == 0) {
        out->mode = SIGIL_MODE_SELECT;
    } else {
        return false;
    }

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            out->capture_all = true;
            continue;
        }

        if (strcmp(argv[i], "--file") == 0) {
            if (i + 1 >= argc) {
                return false;
            }
            out->output_file = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--stdout") == 0) {
            out->write_stdout = true;
            continue;
        }

        if (strcmp(argv[i], "--no-clipboard") == 0) {
            out->use_clipboard = false;
            continue;
        }

        return false;
    }

    return true;
}
