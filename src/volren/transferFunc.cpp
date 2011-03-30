/*****************************************************************************


    Copyright 2009,2010 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <iostream>
#include <algorithm>

#include <fbo.h>
#include <pmisc.h>
#include <draw_routines.h>

#include "renderscreen.h"
#include "transferFunc.h"


//-----------------------------------------------------------------------------
//Constructor and Destructor
//-----------------------------------------------------------------------------
CTransferFunc::CTransferFunc():
    m_TFsize(256),
    m_TF(0),
    m_transferFunc(0),
    m_preInt(0)
{
}


CTransferFunc::~CTransferFunc()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_TF) { delete[] m_TF; m_TF = 0; }
    if(m_transferFunc) {delete m_transferFunc; m_transferFunc=0; }
    if(m_preInt) {delete m_preInt; m_preInt = 0;}
}

//-----------------------------------------------------------------------------
//Create Pre-integration Table
//-----------------------------------------------------------------------------
unsigned char* CTransferFunc::create_Preintegration_Table( unsigned char* Table )
{
//std::cout << __FILE__ << ":" << __func__ << std::endl; 

    //unsigned char* Table = vol->TF;
    
    double rInt[256]; rInt[0] = 0.;
    double gInt[256]; gInt[0] = 0.;
    double bInt[256]; bInt[0] = 0.;
    double aInt[256]; aInt[0] = 0.;

    for( int i=1; i<256; i++ )
    {
        const double tauc = ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2.;

        rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc/255.;
        gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc/255.;
        bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc/255.;
        aInt[i] = aInt[i-1] + tauc;
    }


    unsigned char *lookupImg = new unsigned char[ 256*256*4]; // Preint Texture

    int lookupindex=0;

    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }

            int rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                const double factor = 1. / (double)(smax-smin);
                rcol = static_cast<int>( (rInt[smax]-rInt[smin])*factor );
                gcol = static_cast<int>( (gInt[smax]-gInt[smin])*factor );
                bcol = static_cast<int>( (bInt[smax]-bInt[smin])*factor );
                //acol = static_cast<int>( 255.*( exp(-(aInt[smax]-aInt[smin])*factor/255.)));
                acol = static_cast<int>( (aInt[smax]-aInt[smin])*factor );
            } else
            {
                const int    index  = smin*4;
                const double factor = 1. * Table[index+3] / 255.;
                rcol = static_cast<int>( Table[index+0] * factor );
                gcol = static_cast<int>( Table[index+1] * factor );
                bcol = static_cast<int>( Table[index+2] * factor );
                //acol = static_cast<int>( 255.*( exp(-Table[index+3]/255.)) );
                acol = static_cast<int>( Table[index+3] );
                //printf("acol=%d\n", acol);
            }
            lookupImg[lookupindex++] = clip( rcol, 0, 255 );//MIN( rcol, 255 );
            lookupImg[lookupindex++] = clip( gcol, 0, 255 );//MIN( gcol, 255 );
            lookupImg[lookupindex++] = clip( bcol, 0, 255 );//MIN( bcol, 255 );
            lookupImg[lookupindex++] = clip( acol, 0, 255 );//MIN( acol, 255 );
        }
    }

    return   lookupImg;
}

void CTransferFunc::init_TF_texture()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
     
    m_transferFunc = new tex_unit;
    m_transferFunc->setformat(GL_TEXTURE_1D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    m_transferFunc->setfilter(GL_NEAREST, GL_NEAREST);
    m_transferFunc->create(m_TFsize, 0, (GLvoid*)NULL);
    m_transferFunc->update_subimage(0, m_TFsize, (GLvoid*)m_TF);

}

void CTransferFunc::init_preInt_texture()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_preInt = new tex_unit;
    m_preInt->setformat(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    m_preInt->setfilter(GL_NEAREST, GL_NEAREST);
    m_preInt->create(256, 256, 0, (GLvoid*)NULL);
    unsigned char* preInt = create_Preintegration_Table(m_TF);
    m_preInt->update_subimage(0, 0, 256, 256, (GLvoid*)preInt);
    delete[] preInt;
}


void CTransferFunc::update_textures()
{
    fprintf(stderr, "%s:%s()\n", __FILE__, __func__);

    if(m_transferFunc)
    {
        m_transferFunc->update_subimage(0, m_TFsize, (GLvoid*)m_TF);
        //printf("transferFucn texture: %d\n", TFsize);
    }
    if(m_preInt)
    {
        unsigned char* preInt = create_Preintegration_Table(m_TF);
        m_preInt->update_subimage(0, 0, 256, 256, (GLvoid*)preInt);
        delete[] preInt;
        //printf("preInt texture: 256, 256\n");
    }
}


//-----------------------------------------------------------------------------
//given point pair (px, py), (x, y) in the (m_TFsize, m_TF) value pair, 
//update the TF smoothly between px and x. for tf_channel (RGBA)
//-----------------------------------------------------------------------------
void CTransferFunc::update_TF(int px, int py, int x, int y, int tf_channel)
{
    fprintf(stderr, "%s:%s(): (%d,%d) to (%d,%d)\n", __FILE__, __func__,
            px, py, x, y);

    for(int i=px; i<=x; i++)
    {
        float t = 1.0*(i-px)/(x-px);
        float f = (1.0-t)*py + t*y;
            
        m_TF[4*i + tf_channel] = (unsigned char)(f);
    }
}



void CTransferFunc::draw_transfer_function_component(int comp)
{
    int i;
    float deltax = 2.0/m_TFsize;
    float deltay = 2.0/256;

    switch(comp)
    {
    case 0:
        glColor3f(1.0, 0., 0.); break;
    case 1: 
        glColor3f(0.0, 1., 0.); break;
    case 2:
        glColor3f(0., 0., 1.); break;
    case 3:
        glColor3f(1., 1., 1.);break;
    default:
        assert(0); break;
    }

    glBegin(GL_LINE_STRIP);
    for(i = 0; i<m_TFsize; i++)
    {
        glVertex2f(-1.0 + i*deltax, -1.0+m_TF[i*4+comp]*deltay);
    }
    glEnd();

}



void CTransferFunc::draw_transfer_function(int w, int h, int m, int c )
{
    int vp[4] = {50, 50, m_TFsize, m_TFsize};

    push_all_matrix();
    toOrtho(vp);

    if(m)
    {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1., 1., 1., 0.2);
    draw_quad();
    glDisable(GL_BLEND);
    }

    draw_transfer_function_component(c);
       
    pop_all_matrix();
}

void CTransferFunc::draw_alphamap(int w, int h)
{

    push_all_matrix();
    toOrtho(0);

    glViewport(0, 0, w, 50);

    draw_transfer_function_component(3);
       
    pop_all_matrix();
}


void CTransferFunc::draw_colormap(int w, int h)
{
    push_all_matrix();
    toOrtho(0); //frustum -1 to +1

    //draw a vertical bar near rendering
    //glViewport(w-100, 200, 50, h/2);

    //draw a horizontal bar at the bottom
    glViewport(0, 0, w, 50);

    glBegin(GL_QUADS);

    glColor3ub(m_TF[0], m_TF[1], m_TF[2]);        
    glVertex2f(-1.0, -1.0);
    glVertex2f(-1.0, 1.0);
    for(int i=1; i<m_TFsize-1; i++)
    {
        glColor3ub(m_TF[i*4], m_TF[i*4+1], m_TF[i*4+2]);        
        glVertex2f(2*i/255.0-1.0, 1.0);
        glVertex2f(2*i/255.0-1.0, -1.0);
        glVertex2f(2*i/255.0-1.0, -1.0);
        glVertex2f(2*i/255.0-1.0, 1.0);
    }
    glColor3ub(m_TF[255*4], m_TF[255*4+1], m_TF[255*4+2]);        
    glVertex2f(1.0, 1.0);
    glVertex2f(1.0, -1.0);

    glEnd();
       
    pop_all_matrix();    
}


//by default bind to texture unit 0
void CTransferFunc::draw_preInt_texture(int w, int h)
{
    push_all_matrix();
    toOrtho(0);

    glViewport(w/2, h/2, w/2, h/2);

    glActiveTexture(GL_TEXTURE0);    
    m_preInt->bind();

    glColor3f(0.5, 0.5, 0.5);
    draw_quad();
        
    m_preInt->unbind();

    pop_all_matrix();
}



void CTransferFunc::init_TF(const char* filename)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    FILE* fp = fopen(filename, "r");
    if(fp == NULL) 
    {
        printf("can't read file %s\n", filename);
        exit(1);
    }

    int ret;
    char dummy[16];
    ret = fscanf(fp, "%s\n", dummy);

    int i;
    ret = fscanf(fp, "size=%d\n", &m_TFsize);
    //printf("TFsize: %d\n", m_TFsize);

    m_TF = new unsigned char[m_TFsize*4];

    for( i=0; i<m_TFsize; i++ )
    {
        int r,g, b, a;
        ret = fscanf(fp, "r=%d\n", &r);
        ret = fscanf(fp, "g=%d\n", &g);
        ret = fscanf(fp, "b=%d\n", &b);
        ret = fscanf(fp, "a=%d\n", &a);
            

//         if( a < 10 )
//         {
//             printf("get rid of this entry(%d, %d, %d, %d)\n", r, g, b, a);
//             r=g=b=a=0;
//         }

        m_TF[4*i+0] = r;
        m_TF[4*i+1] = g;
        m_TF[4*i+2] = b;
        m_TF[4*i+3] = a;
        
    }
    fclose(fp);
}


void CTransferFunc::save_TF(const char* filename)
{

    fprintf(stderr, "%s:%s(): save TF header file: %s\n", 
           __FILE__, __func__, filename);

    FILE* fp = fopen(filename, "w");
    if(fp == NULL) 
    {
        printf("can't open file %s for write\n", filename);
        exit(1);
    }

    fprintf(fp, "TF\n");

    int i;
    fprintf(fp, "size=%d\n", m_TFsize);

    for( i=0; i<m_TFsize; i++ )
    {
        fprintf(fp, "r=%d\n", m_TF[4*i+0]);
        fprintf(fp, "g=%d\n", m_TF[4*i+1]);
        fprintf(fp, "b=%d\n", m_TF[4*i+2]);
        fprintf(fp, "a=%d\n", m_TF[4*i+3]);
    }
    fclose(fp);
}


void CTransferFunc::init_colormap(float sr, float sg, float sb)
{
//     fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

//     fprintf(stderr, "cr=%7.3f, cg=%7.3f, cb=%7.3f\n", sr, sg, sb);
    
    if(m_TF==0){
        m_TF = new unsigned char[256*4];
        memset(m_TF, 0, 256*4);
    }


    double r, g, b, value;

    for(int i=0; i<256; i++)
    {
       value = i/255.0;
 
       if(sr==0 && sg==0 && sb==0)
        {
            if(value<0.5) r = 1.; else r = 0.;
            if(value<0.5) g = 0.; else g = 1.;
            b = 0.;
        }
        else
        {

        r = sin( M_PI * 0.5 * sr * value);
        g = sin( M_PI * sg * value);
        b = cos( M_PI * 0.5 * sb * value);
        

        r = pow(r, 3.);
        g = pow(g, 3.);
        b = pow(b, 3.);
        }

        m_TF[4*i+0] = (unsigned char)(255*r);
        m_TF[4*i+1] = (unsigned char)(255*g);
        m_TF[4*i+2] = (unsigned char)(255*b);
    }

}


void CTransferFunc::init_alphamap(double scale,
                                  double sharpness, 
                                  double shift,
                                  double intensity,                                       
                                  int numPeaks)
{
//     fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

//     fprintf(stderr, "scale=%7.3f, sharp=%7.3f, shift=%7.3f, intensity=%7.3f\n", 
//             scale, sharpness, shift, intensity);

    if(m_TF==0)
    {
        m_TF = new unsigned char[256*4];
        memset(m_TF, 0, 256*4);
    }

    double value, alpha;

    for(int i=0; i<256; i++)
    {
        value = i/255.0;
        alpha = 0.;

        if(numPeaks == 0){
            if(value > (1.-sharpness) && value < sharpness) 
                alpha = intensity;
        }
        else
        {
            double s = value - 0.5;
            s *= scale;

            if(s >= -0.5 && s <= 0.5)
            {
                double pv = fabs(sin( M_PI * s * numPeaks + shift));
                alpha = pv > sharpness ? pv : 0.;
                alpha = clip(intensity * alpha, 0., 1.);
            }
        }
        //std::cout << i << "=" << alpha << ";";
        m_TF[4*i+3] = (unsigned char)(255*alpha);
    }
}
