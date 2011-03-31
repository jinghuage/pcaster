#ifndef IMG_H
#define IMG_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

//-----------------------------------------------------------------------------

GLenum internal_form(int, int);
GLenum external_form(int);
GLenum external_type(int);

//-----------------------------------------------------------------------------

void *read_png(const char *, int *, int *, int *, int *);
void write_png(const char *, int,   int,   int,   int, const void *);

void *read_raw(const char *, int *, int *, int *, int *);
void write_raw(const char *, int,   int,   int,   int, const void *);

void *read_img(const char *, int *, int *, int *, int *);
void write_img(const char *, int,   int,   int,   int, const void *);

//-----------------------------------------------------------------------------

#endif
