#ifndef SIGIL_PNG_WRITE_H
#define SIGIL_PNG_WRITE_H

#include "sigil.h"

bool sigil_write_png_file(const char *path, const SigilImage *img);
bool sigil_write_png_stream(FILE *fp, const SigilImage *img);

#endif
