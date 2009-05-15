#ifndef DRAW_ROUTINE_H_
#define DRAW_ROUTINE_H_



//draw
void drawBox(float size, float cx, float cy, float cz, float[6]);
void drawBox(float size, float cx, float cy, float cz);
void draw_grid(int, int, int);
void draw_quad();
void draw_fullscreen_quad();
void toOrtho(int vp[4]);
void toPerspective(int vp[4]);



#endif /*DRAW_ROUTINE_H_*/
