#include "png_write.h"

#include <png.h>

static bool sigil_write_png_internal(FILE *fp, const SigilImage *img) {
    if (!fp || !img || !img->data || img->width == 0 || img->height == 0) {
        return false;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        return false;
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        img->width,
        img->height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png, info);

    for (unsigned int y = 0; y < img->height; y++) {
        png_bytep row = (png_bytep)(img->data + ((size_t)y * img->width * 4u));
        png_write_row(png, row);
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    return true;
}

bool sigil_write_png_file(const char *path, const SigilImage *img) {
    if (!path) {
        return false;
    }

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        return false;
    }

    bool ok = sigil_write_png_internal(fp, img);
    fclose(fp);
    return ok;
}

bool sigil_write_png_stream(FILE *fp, const SigilImage *img) {
    return sigil_write_png_internal(fp, img);
}
