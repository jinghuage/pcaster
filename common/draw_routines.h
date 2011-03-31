#ifndef DRAW_ROUTINE_H_
#define DRAW_ROUTINE_H_


#include "texture.h"
#include "shader.h"

//draw
void DrawSphere(float);

void getBoxCorners(const float* fsize, const float* center, float* vCorner);
void getBoxCorners(const float* fsize, const float* center, float* tos,
                   float* vCorner, float* vColor);
void drawTosBox(float*, float*, float*);
void drawBox(float*, float*);
void drawFace(int, float*, float*);
void drawColorBox(const float*, const float*, const float* );
void drawPlainBox(const float*, const float*);
void draw_box();
void draw_texture_box();
void draw_grid(int, int, int);
void draw_quad();
void draw_quad(float, float, float, float, float, float);
void draw_fullscreen_quad(int* vp, bool tex=false);
void draw_fullscreen_quad(bool frontface = true);
void draw_texture(tex_unit*, int vp[4]);
void draw_rec_texture(tex_unit*, int vp[4]);
void draw_rec_texture(tex_unit*, int vp[4], shader_object*);

void draw_quadCutLine(int direction, float position, float* size, float* center);
void draw_boxCutPlane(int direction, float position, float* s, float* c);

void push_all_matrix();
void pop_all_matrix();
void toOrtho_projection(int vp[4]);
void toPerspective_projection(int vp[4]);
void toOrtho(int vp[4]);
void toOrtho(int vp[4], float, float, float, float);
void toPerspective(int vp[4]);


#endif /*DRAW_ROUTINE_H_*/
