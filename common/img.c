#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <regex.h>
#include <png.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#define MAXSTR 256

//-----------------------------------------------------------------------------

static void fail(const char *error)
{
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

GLenum internal_form(int c, int b)
{
    switch (b)
    {
    case 2:
        switch (c)
        {
        case  1: return GL_LUMINANCE16;
        case  2: return GL_LUMINANCE16_ALPHA16;
        case  3: return GL_RGB16;
        default: return GL_RGBA16;
        }

    case 4:
    case 8:
        switch (c)
        {
        case  1: return GL_LUMINANCE32F_ARB;
        case  2: return GL_LUMINANCE_ALPHA32F_ARB;
        case  3: return GL_RGB32F_ARB;
        default: return GL_RGBA32F_ARB;
        }

    default:
        switch (c)
        {
        case  1: return GL_LUMINANCE;
        case  2: return GL_LUMINANCE_ALPHA;
        case  3: return GL_RGB;
        default: return GL_RGBA;
        }
    }
}

GLenum external_form(int c)
{
    switch (c)
    {
    case  1: return GL_LUMINANCE;
    case  2: return GL_LUMINANCE_ALPHA;
    case  3: return GL_RGB;
    default: return GL_RGBA;
    }
}

GLenum external_type(int b)
{
    switch (b)
    {
    case  2: return GL_UNSIGNED_SHORT;
    case  4: return GL_FLOAT;
    case  8: return GL_DOUBLE;
    default: return GL_UNSIGNED_BYTE;
    }
}

//-----------------------------------------------------------------------------

void *read_png(const char *name, int *w, int *h, int *c, int *b)
{
    png_structp rp = NULL;
    png_infop   ip = NULL;
    png_bytep  *bp = NULL;
    FILE       *fp = NULL;
    void        *p = NULL;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(name, "rb")))
        fail(strerror(errno));

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        fail("Failure to allocate PNG read structure");

    if (!(ip = png_create_info_struct(rp)))
        fail("Failure to allocate PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        // Read the PNG header.

        png_init_io (rp, fp);
        png_read_png(rp, ip, PNG_TRANSFORM_PACKING |
                             PNG_TRANSFORM_SWAP_ENDIAN, NULL);
        
        *w = (int) png_get_image_width (rp, ip);
        *h = (int) png_get_image_height(rp, ip);
        *c = (int) png_get_channels    (rp, ip);
        *b = (int) png_get_bit_depth   (rp, ip) / 8;

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Allocate the final buffer and flip-copy the pixels there.

            int i, j, s = (*w) * (*c) * (*b);

            if ((p = malloc((*w) * (*h) * (*c) * (*b))))

                for (i = 0, j = (*h) - 1; j >= 0; ++i, --j)
                    memcpy((png_bytep) p + s * i, bp[j], s);

            else fail("Failure to allocate image buffer");
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, NULL);
    fclose(fp);

    return p;
}

void write_png(const char *name, int w, int h, int c, int b, const void *p)
{
    png_structp wp = NULL;
    png_infop   ip = NULL;
    png_bytep  *bp = NULL;
    FILE       *fp = NULL;

    // Initialize all PNG export data structures.

    if (!(fp = fopen(name, "wb")))
        fail(strerror(errno));

    if (!(wp = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        fail("Failure to allocate PNG write structure");

    if (!(ip = png_create_info_struct(wp)))
        fail("Failure to allocate PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(wp)) == 0)
    {
        static const int color[] = {
            0,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_GRAY_ALPHA,
            PNG_COLOR_TYPE_RGB,
            PNG_COLOR_TYPE_RGB_ALPHA
        };

        // Initialize the PNG header.

        png_init_io (wp, fp);
        png_set_compression_level(wp, 9);
        png_set_IHDR(wp, ip, w, h, b*8, color[c], PNG_INTERLACE_NONE,
                                                  PNG_COMPRESSION_TYPE_DEFAULT,
                                                  PNG_FILTER_TYPE_DEFAULT);

        // Allocate and initialize the row pointers.

        if ((bp = (png_bytep *) png_malloc(wp, h * sizeof (png_bytep))))
        {
            int i, j;

            for (i = 0, j = h - 1; j >= 0; ++i, --j)
                bp[j] = (png_bytep) p + i * w * c * b;

            // Write the PNG image file.

            png_set_rows  (wp, ip, bp);
            png_write_info(wp, ip);
            png_write_png (wp, ip, PNG_TRANSFORM_SWAP_ENDIAN, NULL);

            free(bp);
        }
        else fail("Failure to allocate PNG row array");
    }

    // Release all resources.

    png_destroy_write_struct(&wp, &ip);
    fclose(fp);
}

//-----------------------------------------------------------------------------

static size_t size_raw(const char *name, int *w, int *h, int *c, int *b)
{
    const char *pattern = "([0-9]+)x([0-9]+)x([0-9]+)([bdfs])";

    regmatch_t match[5];
    regex_t    reg;
    size_t     siz = 0;

    assert(regcomp(&reg, pattern, REG_EXTENDED) == 0);

    if (regexec(&reg, name, 5, match, 0) == 0)
    {
        *w = (int) strtol(name + match[1].rm_so, NULL, 0);
        *h = (int) strtol(name + match[2].rm_so, NULL, 0);
        *c = (int) strtol(name + match[3].rm_so, NULL, 0);

        switch (name[match[4].rm_so])
        {
        case 'b': *b = 1; break;
        case 's': *b = 2; break;
        case 'f': *b = 4; break;
        case 'd': *b = 8; break;
        }

        siz = (*w) * (*h) * (*c) * (*b);
    }

    regfree(&reg);
    return siz;
}

void *read_raw(const char *name, int *w, int *h, int *c, int *b)
{
    void   *p = NULL;
    FILE  *fp;
    size_t sz;

    if ((sz = size_raw(name, w, h, c, b)))
    {
        if ((fp = fopen(name, "rb")))
        {
            int s = (*w) * (*c) * (*b);

            if ((p = malloc(sz)))
                for (int r = *h - 1; r >= 0; --r)
                    fread((uint8_t *) p + r * s, 1, s, fp);

            fclose(fp);
        }
        else fail(strerror(errno));
    }
    return p;
}

void write_raw(const char *name, int w, int h, int c, int b, const void *p)
{
    FILE  *fp;
    size_t sz;

    if ((sz = size_raw(name, &w, &h, &c, &b)))
    {
        if ((fp = fopen(name, "wb")))
        {
            int s = w * c * b;

            for (int r = h - 1; r >= 0; --r)
                fwrite((uint8_t *) p + r * s, 1, s, fp);

            fclose(fp);
        }
        else fail(strerror(errno));
    }
}

//-----------------------------------------------------------------------------

void *read_img(const char *name, int *w, int *h, int *c, int *b)
{
    const char *ext = name + strlen(name) - 4;

    if      (strcmp(ext, ".png") == 0) return read_png(name, w, h, c, b);
    else if (strcmp(ext, ".raw") == 0) return read_raw(name, w, h, c, b);

    return NULL;
}

void write_img(const char *name, int w, int h, int c, int b, const void *p)
{
    const char *ext  = name + strlen(name) - 4;

    if      (strcmp(ext, ".png") == 0) write_png(name, w, h, c, b, p);
    else if (strcmp(ext, ".raw") == 0) write_raw(name, w, h, c, b, p);
}

//-----------------------------------------------------------------------------
