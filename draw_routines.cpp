//some draw routines
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include "misc.h"
#include "glheaders.h"

#define dtor(a) (3.1416*a/180.0)

template <class T>
T clip( T val, T min, T max )
{
    return ( val<min ? min : ( val>max ? max : val ) );
}

static void getBoxCorners(float x, float y, float z, float fsize, 
                          float* vCorner, float* vColor, float tos[6])
{

    vCorner[0] = x - fsize/2; 
    vCorner[1] = y - fsize/2; 	
    vCorner[2] = z - fsize/2;

    vColor[0] = tos[0];
    vColor[1] = tos[2+0];
    vColor[2] = tos[4+0];

    vCorner[3] = x - fsize/2; 	
    vCorner[4] = y - fsize/2; 	
    vCorner[5] = z + fsize/2;

    vColor[3] = tos[0];
    vColor[4] = tos[2+0];
    vColor[5] = tos[4+1];

    vCorner[6] = x - fsize/2; 
    vCorner[7] = y + fsize/2; 
    vCorner[8] = z - fsize/2;

    vColor[6] = tos[0];
    vColor[7] = tos[2+1];
    vColor[8] = tos[4+0];

    vCorner[9] = x - fsize/2;
    vCorner[10] = y + fsize/2;
    vCorner[11] = z + fsize/2;

    vColor[9] = tos[0];
    vColor[10] = tos[2+1];
    vColor[11] = tos[4+1];

    vCorner[12] = x + fsize/2;
    vCorner[13] = y - fsize/2;
    vCorner[14] = z - fsize/2;

    vColor[12] = tos[0+1];
    vColor[13] = tos[2+0];
    vColor[14] = tos[4+0];

    vCorner[15] = x + fsize/2;
    vCorner[16] = y - fsize/2;
    vCorner[17] = z + fsize/2;

    vColor[15] = tos[0+1];
    vColor[16] = tos[2+0];
    vColor[17] = tos[4+1];

    vCorner[18] = x + fsize/2;
    vCorner[19] = y + fsize/2;
    vCorner[20] = z - fsize/2;

    vColor[18] = tos[0+1];
    vColor[19] = tos[2+1];
    vColor[20] = tos[4+0];

    vCorner[21] = x + fsize/2;
    vCorner[22] = y + fsize/2;
    vCorner[23] = z + fsize/2;

    vColor[21] = tos[0+1];
    vColor[22] = tos[2+1];
    vColor[23] = tos[4+1];

}

void drawBox(float size, float cx, float cy, float cz, float tos[6])
{
    float vCorner[24], vColor[24];

    getBoxCorners(cx, cy, cz, size, vCorner, vColor, tos);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLubyte indice[] = {
      0, 1, 3 ,2,
      4, 6, 7, 5,
      0, 4, 5, 1,
      2, 3, 7, 6,
      3, 1, 5, 7,
      0, 2, 6, 4 };
    
//GLubyte indice[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 2, 1, 3, 4, 6, 5, 7, 0, 4, 2, 6, 1, 5, 3, 7};

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vCorner);
    glColorPointer(3, GL_FLOAT, 0, vColor);
    //glNormalPointer(3, GL_FLOAT, 0, vNormal);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indice);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

//     glBegin(GL_LINES);
//     for(int i=0; i<24; i++)
//     {
//         glVertex3fv(vCorner+3*indice[i]);
//     }
//     glEnd();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void drawBox(float size, float cx, float cy, float cz)
{
    float vCorner[24], vColor[24];
    
    float tos[6] = {0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};
    getBoxCorners(cx, cy, cz, size, vCorner, vColor, tos);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLubyte indice[] = {
      0, 1, 3 ,2,
      4, 6, 7, 5,
      0, 4, 5, 1,
      2, 3, 7, 6,
      3, 1, 5, 7,
      0, 2, 6, 4 };
    

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vCorner);
    glColorPointer(3, GL_FLOAT, 0, vColor);
    //glNormalPointer(3, GL_FLOAT, 0, vNormal);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indice);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}


void draw_quad()
{
    glPushMatrix();
    glBegin( GL_POLYGON );
    //glTexCoord2f( 0.0f, 0.0f );
    glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
    glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
    glVertex3f( -1.0f, -1.0f, 0.0f);

    //glTexCoord2f( 1.0f, 0.0f );
    glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
    glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
    glVertex3f(  1.0f, -1.0f, 0.0f);

    //glTexCoord2f( 1.0f, 1.0f );   
    glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
    glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);     
    glVertex3f(  1.0f,  1.0f, 0.0f);

    //glTexCoord2f( 0.0f, 1.0f );
    glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
    glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);     
    glVertex3f( -1.0f,  1.0f, 0.0f);
    glEnd();    
    glPopMatrix();
}





void toOrtho(int vp[4])
{
    // set viewport to be the entire window
    glViewport(vp[0], vp[1], vp[2], vp[3]);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(vp[0], vp[2]-vp[0], vp[1], vp[3]-vp[1], -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



void toPerspective(int vp[4])
{
    // set viewport to be the entire window
    glViewport(vp[0], vp[1], vp[2], vp[3]);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(60.0f, (float)(SCREEN_WIDTH)/SCREEN_HEIGHT, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip
    float ratio = 0.1 * (vp[2]-vp[0])/(vp[3] - vp[1]);
    glFrustum(-ratio, ratio, -0.1, 0.1, 0.1, 100);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    gluLookAt(0, 0, 8, 0, 0, 0, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
    //glTranslatef(0, 0, -8);
}


void draw_fullscreen_quad() 
{

	glColor3f(0.0, 1.0, 1.0);
	//printOpenGLError();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -10, 10);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_POLYGON );
	//glTexCoord2f( 0.0f, 0.0f );
	//glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
	//glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
	glVertex2f( -1.0f, -1.0f);

	//glTexCoord2f( 1.0f, 0.0f );
	//glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
	//glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
	glVertex2f( 1.0f, -1.0f);

	//glTexCoord2f( 1.0f, 1.0f );   
	//glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
	//glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);
	glVertex2f( 1.0f, 1.0f);

	//glTexCoord2f( 0.0f, 1.0f );
	//glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
	//glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);
	glVertex2f( -1.0f, 1.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

//draw a volume grid within a (-1, 1) cube
void draw_grid(int w, int h, int d)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glScalef(0.5, 0.5, 0.5);
    //glScalef(1.0, 1.0, 1.0);
    //glTranslatef(0.0, 0.0, -2.0);
    //glRotatef(angleY, 0.0, 1.0, 0.0);
    //glRotatef(angleX, 1.0, 0.0, 0.0);
    
    glBegin( GL_QUADS );
    
    glColor4f(0., 0., 1., 0.05);
    for(int i=0; i<d; i++)
    {
        glVertex3f( -1.0f, -1.0f, -1. + 2.0f * i/d);
        glVertex3f(  1.0f, -1.0f, -1. + 2.0f * i/d);
        glVertex3f(  1.0f,  1.0f, -1. + 2.0f * i/d);
        glVertex3f( -1.0f,  1.0f, -1. + 2.0f * i/d);
    }

    //glColor4f(0., 1., 0., 0.08);
    for(int i=0; i<h; i++)
    {
        glVertex3f( -1.0f, -1. + 2.0f * i/d, -1.0f);
        glVertex3f(  1.0f, -1. + 2.0f * i/d, -1.0f);
        glVertex3f(  1.0f, -1. + 2.0f * i/d,  1.0f);
        glVertex3f( -1.0f, -1. + 2.0f * i/d,  1.0f);
    }

    //glColor4f(1., 0., 0., 0.08);
    for(int i=0; i<w; i++)
    {
        glVertex3f( -1. + 2.0f * i/d, -1.0f, -1.0f);
        glVertex3f( -1. + 2.0f * i/d,  1.0f, -1.0f);
        glVertex3f( -1. + 2.0f * i/d,  1.0f,  1.0f);
        glVertex3f( -1. + 2.0f * i/d, -1.0f,  1.0f);
    }

    glEnd();
    glDisable(GL_BLEND);
}






