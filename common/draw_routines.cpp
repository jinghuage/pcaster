//some draw routines
#include <stdio.h>
#include <stdlib.h>
//#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "misc.h"
#include "glheaders.h"

#include "draw_routines.h"


#include <string>

/* constants */
#define dtor(a) (3.1416*a/180.0)
#ifndef M_PI
#define M_PI            (3.14159265358979f)
#endif
#define DTOR            (M_PI/180.0)
#define RTOD            (180.0/M_PI)

static void vert(float theta, float phi)
{
  float r = 0.75f;
  float x, y, z, nx, ny, nz;

  nx = sin(DTOR * theta) * cos(DTOR * phi);
  ny = sin(DTOR * phi);
  nz = cos(DTOR * theta) * cos(DTOR * phi);
  glNormal3f(nx, ny, nz);

  x = r * sin(DTOR * theta) * cos(DTOR * phi);
  y = r * sin(DTOR * phi);
  z = r * cos(DTOR * theta) * cos(DTOR * phi);
  glVertex4f(x, y, z, 1.0);
}

void DrawSphere(float del)
{
  float phi, phi2, theta;

  glColor4f(1.0, 1.0, 1.0, 1.0);
  for (phi = -90.0f; phi < 90.0f; phi += del) {
    glBegin(GL_TRIANGLE_STRIP);

    phi2 = phi + del;

    for (theta = -90.0f; theta <= 90.0f; theta += del) {
      vert(theta, phi);
      vert(theta, phi2);
    }
    glEnd();
  }
}


void getBoxCorners(const float* fsize, const float* center, float* vCorner)
{

    vCorner[0] = center[0] - fsize[0]/2; 
    vCorner[1] = center[1] - fsize[1]/2; 	
    vCorner[2] = center[2] - fsize[2]/2;

    vCorner[3] = center[0] - fsize[0]/2; 	
    vCorner[4] = center[1] - fsize[1]/2; 	
    vCorner[5] = center[2] + fsize[2]/2;

    vCorner[6] = center[0] - fsize[0]/2; 
    vCorner[7] = center[1] + fsize[1]/2; 
    vCorner[8] = center[2] - fsize[2]/2;

    vCorner[9] = center[0] - fsize[0]/2;
    vCorner[10] = center[1] + fsize[1]/2;
    vCorner[11] = center[2] + fsize[2]/2;

    vCorner[12] = center[0] + fsize[0]/2;
    vCorner[13] = center[1] - fsize[1]/2;
    vCorner[14] = center[2] - fsize[2]/2;

    vCorner[15] = center[0] + fsize[0]/2;
    vCorner[16] = center[1] - fsize[1]/2;
    vCorner[17] = center[2] + fsize[2]/2;

    vCorner[18] = center[0] + fsize[0]/2;
    vCorner[19] = center[1] + fsize[1]/2;
    vCorner[20] = center[2] - fsize[2]/2;

    vCorner[21] = center[0] + fsize[0]/2;
    vCorner[22] = center[1] + fsize[1]/2;
    vCorner[23] = center[2] + fsize[2]/2;
}

void getBoxCorners(const float* fsize, const float* center, 
                   float* tos,
                   float* vCorner, 
                   float* vColor)
{

    vCorner[0] = center[0] - fsize[0]/2; 
    vCorner[1] = center[1] - fsize[1]/2; 	
    vCorner[2] = center[2] - fsize[2]/2;

    vColor[0] = tos[0];
    vColor[1] = tos[2+0];
    vColor[2] = tos[4+0];

    vCorner[3] = center[0] - fsize[0]/2; 	
    vCorner[4] = center[1] - fsize[1]/2; 	
    vCorner[5] = center[2] + fsize[2]/2;

    vColor[3] = tos[0];
    vColor[4] = tos[2+0];
    vColor[5] = tos[4+1];

    vCorner[6] = center[0] - fsize[0]/2; 
    vCorner[7] = center[1] + fsize[1]/2; 
    vCorner[8] = center[2] - fsize[2]/2;

    vColor[6] = tos[0];
    vColor[7] = tos[2+1];
    vColor[8] = tos[4+0];

    vCorner[9] = center[0] - fsize[0]/2;
    vCorner[10] = center[1] + fsize[1]/2;
    vCorner[11] = center[2] + fsize[2]/2;

    vColor[9] = tos[0];
    vColor[10] = tos[2+1];
    vColor[11] = tos[4+1];

    vCorner[12] = center[0] + fsize[0]/2;
    vCorner[13] = center[1] - fsize[1]/2;
    vCorner[14] = center[2] - fsize[2]/2;

    vColor[12] = tos[0+1];
    vColor[13] = tos[2+0];
    vColor[14] = tos[4+0];

    vCorner[15] = center[0] + fsize[0]/2;
    vCorner[16] = center[1] - fsize[1]/2;
    vCorner[17] = center[2] + fsize[2]/2;

    vColor[15] = tos[0+1];
    vColor[16] = tos[2+0];
    vColor[17] = tos[4+1];

    vCorner[18] = center[0] + fsize[0]/2;
    vCorner[19] = center[1] + fsize[1]/2;
    vCorner[20] = center[2] - fsize[2]/2;

    vColor[18] = tos[0+1];
    vColor[19] = tos[2+1];
    vColor[20] = tos[4+0];

    vCorner[21] = center[0] + fsize[0]/2;
    vCorner[22] = center[1] + fsize[1]/2;
    vCorner[23] = center[2] + fsize[2]/2;

    vColor[21] = tos[0+1];
    vColor[22] = tos[2+1];
    vColor[23] = tos[4+1];

}



void drawBox(float* vCorner, float* vColor)
{
    assert(vCorner != 0);

    //indice sequence make sure each face facing outside.
    GLubyte indice[] = {
        0, 1, 3 ,2, //left
        4, 6, 7, 5, //right
        0, 4, 5, 1, //bottom
        2, 3, 7, 6, //up
        3, 1, 5, 7, //front
        0, 2, 6, 4 }; //back
    

    glEnableClientState(GL_VERTEX_ARRAY);
    if(vColor) glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vCorner);
    if(vColor) glColorPointer(3, GL_FLOAT, 0, vColor);
    //glNormalPointer(3, GL_FLOAT, 0, vNormal);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indice);
    glDisableClientState(GL_VERTEX_ARRAY);
    if(vColor) glDisableClientState(GL_COLOR_ARRAY);
}


//draw face
void drawFace(int id, float* vCorner, float* vColor)
{
    assert(vCorner != 0);

    GLubyte indice[6][4] = {
        {0, 1, 3 ,2}, //left,
        {4, 6, 7, 5}, //right, 
        {0, 4, 5, 1}, //bottom
        {2, 3, 7, 6}, //up
        {0, 2, 6, 4}, //back
        {3, 1, 5, 7}  //front
    }; 
    

    glEnableClientState(GL_VERTEX_ARRAY);
    if(vColor) glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vCorner);
    if(vColor) glColorPointer(3, GL_FLOAT, 0, vColor);
    //glNormalPointer(3, GL_FLOAT, 0, vNormal);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indice[id]);
    glDisableClientState(GL_VERTEX_ARRAY);
    if(vColor) glDisableClientState(GL_COLOR_ARRAY);
}


void drawTosBox(float* size, float* center, float* tos)
{
    float vCorner[24], vColor[24];

    getBoxCorners(size, center, tos, vCorner, vColor);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawBox(vCorner, vColor);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void drawColorBox(const float* size, const float* center, const float* color)
{
    float vCorner[24];
    
    getBoxCorners(size, center, vCorner);


    if(color == 0)
    {
        //float c[3];
        //for(int i=0; i<3; i++) c[i] = center[i] + 0.5;
        //glColor4f(1.0-center[0], 1.0-center[1], 1.0-center[2], 0.5);
        glColor4f(0.5-center[0], 0.5-center[1], 0.5-center[2], 0.5);
    }
    else glColor4fv(color);

    drawBox(vCorner, 0);

    //glColor4f(1.0, 1.0, 1.0, 1.0);
}

void drawPlainBox(const float* size, const float* center)
{
    float vCorner[24];
    
    getBoxCorners(size, center, vCorner);
    drawBox(vCorner, 0);
}

void draw_box()
{
    glBegin(GL_QUADS);
    // negative X
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, 1.0);

    // positive X
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, 1.0);

    // negative Z
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);

    // positive Z
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);

    // negative Y
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);

    // positive Y
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);

    glEnd();
}


void draw_texture_box()
{
    glBegin(GL_QUADS);		// Start Drawing Quads
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// Top Left Of The Texture and Quad
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// Top Left Of The Texture and Quad
    glEnd();								// Done Drawing Quads
}


void draw_quad()
{
    //glPushMatrix();
    glBegin( GL_POLYGON );

    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( -1.0f, -1.0f);

    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f(  1.0f, -1.0f);

    glTexCoord2f( 1.0f, 1.0f );   
    glVertex2f(  1.0f,  1.0f);

    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( -1.0f,  1.0f);

    glEnd();    
    //glPopMatrix();
}



void draw_quad(float minx, float maxx, 
               float miny, float maxy,
               float texx, float texy)
{
    //glPushMatrix();
    glBegin( GL_POLYGON );

    glTexCoord2f( 0.0f, 0.0f ); glVertex2f( minx, miny); glNormal3f(0., 0., 1.);
    glTexCoord2f( texx, 0.0f ); glVertex2f( maxx, miny); glNormal3f(0., 0., 1.);
    glTexCoord2f( texx, texy ); glVertex2f( maxx, maxy); glNormal3f(0., 0., 1.);
    glTexCoord2f( 0.0f, texy ); glVertex2f( minx, maxy); glNormal3f(0., 0., 1.);

    glEnd();    
    //glPopMatrix();
}


void draw_quadCutLine(int direction, float position, float* size, float* center)
{
    float minx = center[0] - size[0]/2.;
    float maxx = center[0] + size[0]/2.;
    float miny = center[1] - size[1]/2.;
    float maxy = center[1] + size[1]/2.;

    glBegin( GL_LINES );

    if(direction == 0) 
    {
        glVertex2f(position, miny);
        glVertex2f(position, maxy);
    }
    else if(direction == 1)
    {
        glVertex2f(minx, position);
        glVertex2f(maxx, position);       
    }

    glEnd();  
}


void draw_boxCutPlane(int direction, float position, float* size, float* center)
{
    GLubyte indice[6][4] = {
        {0, 1, 3 ,2}, //left
        {4, 6, 7, 5}, //right
        {0, 4, 5, 1}, //bottom
        {2, 3, 7, 6}, //up
        {3, 1, 5, 7}, //front
        {0, 2, 6, 4} }; //back
    
    float vCorner[24];    
    getBoxCorners(size, center, vCorner);


    for(int i=0; i<4; i++)
    {
        int k = indice[direction*2][i];
        vCorner[k*3 + direction] = position;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vCorner);
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, indice[direction*2]);
    glDisableClientState(GL_VERTEX_ARRAY);

}



// //-----------------------------------------------------------------------------
// //setup geometry transform
// //-----------------------------------------------------------------------------

// void reshape(int w, int h ) 
// {

//     assert( h > 0);

//     float ratio = 0.1 * w / h;

//     // Reset the coordinate system before modifying
//     glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();
	
//     // Set the viewport to be the entire window
//     glViewport(0, 0, w, h);

//     // Set the correct perspective.
//     glFrustum(-ratio, ratio, -0.1, 0.1, 0.8, 100);
//     //glOrtho(-1, 1, -1, 1, -100, 100);
	
//     glMatrixMode(GL_MODELVIEW);

// }


void push_all_matrix()
{
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

}

void pop_all_matrix()
{
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void toOrtho_projection(int vp[4])
{
    // set viewport to be the entire window
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glScissor(vp[0], vp[1], vp[2], vp[3]);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -10, 10);

    // switch to modelview matrix in order to set scene
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

}

void toOrtho(int vp[4])
{
    if(vp)
    {
    glViewport(vp[0], vp[1], vp[2], vp[3]);
//    glScissor(vp[0], vp[1], vp[2], vp[3]);
    }
    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void toOrtho(int vp[4], float l, float r, float b, float t)
{
    if(vp)
    {
    glViewport(vp[0], vp[1], vp[2], vp[3]);
//    glScissor(vp[0], vp[1], vp[2], vp[3]);
    }
    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(l, r, b, t, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



void toPerspective_projection(int vp[4])
{
    // set viewport to be the entire window
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glScissor(vp[0], vp[1], vp[2], vp[3]);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = 0.1 * (vp[2]-vp[0])/(vp[3] - vp[1]);
    glFrustum(-ratio, ratio, -0.1, 0.1, 0.8, 100);

    // switch to modelview matrix in order to set scene
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

}


void toPerspective(int vp[4])
{
    if(vp)
    {
    // set viewport to be the entire window
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glScissor(vp[0], vp[1], vp[2], vp[3]);
    }
    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio = 1.0;
    glFrustum(-ratio, ratio, -0.1, 0.1, 0.8, 100);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}


void draw_fullscreen_quad(int* vp, bool tex) 
{

    glColor3f(1.0, 1.0, 1.0);

    if(vp)
    {
        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        glViewport(vp[0], vp[1], vp[2], vp[3]);
        glScissor(vp[0], vp[1], vp[2], vp[3]);
    }

    //printOpenGLError();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -10, 10);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_POLYGON );
    if(tex) glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( -1.0f, -1.0f);

    if(tex) glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( 1.0f, -1.0f);

    if(tex) glTexCoord2f( 1.0f, 1.0f );   
    glVertex2f( 1.0f, 1.0f);

    if(tex) glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( -1.0f, 1.0f);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if(vp) glPopAttrib();

}

void draw_fullscreen_quad(bool frontface) 
{

    //glColor3f(1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -10, 10);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_POLYGON );

    if(frontface)
    {
        glVertex2f( -1.0f, -1.0f);
        glVertex2f( 1.0f, -1.0f);
        glVertex2f( 1.0f, 1.0f);
        glVertex2f( -1.0f, 1.0f);
    }
    else
    {
        glVertex2f( -1.0f, -1.0f);
        glVertex2f( -1.0f, 1.0f);
        glVertex2f( 1.0f, 1.0f);
        glVertex2f( 1.0f, -1.0f);
    }

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

//------------------------------------------------------------------------------
// show texture to screen. 
// good for drawing small debug screens  
//------------------------------------------------------------------------------

void draw_texture(tex_unit* t, int vp[4])
{
    glActiveTexture(GL_TEXTURE0);    
    t->bind();

    draw_fullscreen_quad(vp, true);
        
    t->unbind();
}


void draw_rec_texture(tex_unit* t, int vp[4])
{
    std::string RECTEXV = "rec_tex.vert";
    std::string RECTEXF = "rec_tex.frag";
    shader_object *rectex_shader = new shader_object;
    rectex_shader->init_from_file(RECTEXV.c_str(), RECTEXF.c_str());

    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);    
    t->bind();

    float mag[2] = {1.0, 1.0};
    float offset[2] = {0., 0.};
    if(vp)
    {
        mag[0] = 1.0 * t->get_width() / vp[2];
        mag[1] = 1.0 * t->get_height() / vp[3];
        offset[0] = vp[0];
        offset[1] = vp[1];
        toOrtho(vp);
    }
    else
    {
        int v[4];
        v[0] = 0;
        v[1] = 0;
        v[2] = t->get_width();
        v[3] = t->get_height();
        offset[0] = v[0];
        offset[1] = v[1];   
        toOrtho(v);
    }
    rectex_shader->use();
    glUniform2f(rectex_shader->getUniformLocation("vpxy"), offset[0], offset[1]);
    glUniform2f(rectex_shader->getUniformLocation("p"), mag[0], mag[1]);
    draw_quad();
    glUseProgram(0);
        
    t->unbind();
    pop_all_matrix();
    glEnable(GL_DEPTH_TEST);
    delete rectex_shader;
}


void draw_rec_texture(tex_unit* t, int vp[4], shader_object* rectex_shader)
{
    glActiveTexture(GL_TEXTURE0);    
    t->bind();

    float mag[2];
    mag[0] = 1.0 * t->get_width() / vp[2];
    mag[1] = 1.0 * t->get_height() / vp[3];

    rectex_shader->use();
    glUniform2f(rectex_shader->getUniformLocation("vpxy"), vp[0], vp[1]);
    glUniform2f(rectex_shader->getUniformLocation("p"), mag[0], mag[1]);
    draw_fullscreen_quad(vp, false);
    glUseProgram(0);
        
    t->unbind();
}


//draw a volume grid within a (-1, 1) cube
void draw_grid(int w, int h, int d)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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






