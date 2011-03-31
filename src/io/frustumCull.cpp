
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include <draw_routines.h>
#include <pmisc.h>

#include "frustumCull.h"


#define PRINT_FRUSTUM_PLANES 0

CFrustumCull::CFrustumCull()
{
}

CFrustumCull::~CFrustumCull()
{
}

void CFrustumCull::print()
{
	fprintf(stderr, "test print\n");
}

//-----------------------------------------------------------------------------
// Name: calculateMvp()
// Desc: 
//-----------------------------------------------------------------------------
void CFrustumCull::calculateMvpMatrix()                                 
{
    //GLfloat mv[16];
    //GLfloat p[16];

    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    glGetFloatv(GL_PROJECTION_MATRIX, p);

    //
    // Concatenate the projection matrix and the model-view matrix to produce 
    // a combined model-view-projection matrix.
    //
    
    mvp[ 0] = mv[ 0] * p[ 0] + mv[ 1] * p[ 4] + mv[ 2] * p[ 8] + mv[ 3] * p[12];
    mvp[ 1] = mv[ 0] * p[ 1] + mv[ 1] * p[ 5] + mv[ 2] * p[ 9] + mv[ 3] * p[13];
    mvp[ 2] = mv[ 0] * p[ 2] + mv[ 1] * p[ 6] + mv[ 2] * p[10] + mv[ 3] * p[14];
    mvp[ 3] = mv[ 0] * p[ 3] + mv[ 1] * p[ 7] + mv[ 2] * p[11] + mv[ 3] * p[15];

    mvp[ 4] = mv[ 4] * p[ 0] + mv[ 5] * p[ 4] + mv[ 6] * p[ 8] + mv[ 7] * p[12];
    mvp[ 5] = mv[ 4] * p[ 1] + mv[ 5] * p[ 5] + mv[ 6] * p[ 9] + mv[ 7] * p[13];
    mvp[ 6] = mv[ 4] * p[ 2] + mv[ 5] * p[ 6] + mv[ 6] * p[10] + mv[ 7] * p[14];
    mvp[ 7] = mv[ 4] * p[ 3] + mv[ 5] * p[ 7] + mv[ 6] * p[11] + mv[ 7] * p[15];

    mvp[ 8] = mv[ 8] * p[ 0] + mv[ 9] * p[ 4] + mv[10] * p[ 8] + mv[11] * p[12];
    mvp[ 9] = mv[ 8] * p[ 1] + mv[ 9] * p[ 5] + mv[10] * p[ 9] + mv[11] * p[13];
    mvp[10] = mv[ 8] * p[ 2] + mv[ 9] * p[ 6] + mv[10] * p[10] + mv[11] * p[14];
    mvp[11] = mv[ 8] * p[ 3] + mv[ 9] * p[ 7] + mv[10] * p[11] + mv[11] * p[15];

    mvp[12] = mv[12] * p[ 0] + mv[13] * p[ 4] + mv[14] * p[ 8] + mv[15] * p[12];
    mvp[13] = mv[12] * p[ 1] + mv[13] * p[ 5] + mv[14] * p[ 9] + mv[15] * p[13];
    mvp[14] = mv[12] * p[ 2] + mv[13] * p[ 6] + mv[14] * p[10] + mv[15] * p[14];
    mvp[15] = mv[12] * p[ 3] + mv[13] * p[ 7] + mv[14] * p[11] + mv[15] * p[15];
}



//-----------------------------------------------------------------------------
// Name: calculateFrustumPlanes()
// Desc: 
//-----------------------------------------------------------------------------
void CFrustumCull::calculateFrustumPlanes(float minx, float maxx,
                                          float miny, float maxy,
                                          float minz, float maxz)                                 
{
    //
    // Extract the frustum's right clipping plane and normalize it.
    //

    m_frustumPlanes[0][0] = maxx * mvp[ 3] - mvp[ 0];
    m_frustumPlanes[0][1] = maxx * mvp[ 7] - mvp[ 4];
    m_frustumPlanes[0][2] = maxx * mvp[11] - mvp[ 8];
    m_frustumPlanes[0][3] = maxx * mvp[15] - mvp[12];

    float t = (float) sqrt( m_frustumPlanes[0][0] * m_frustumPlanes[0][0] + 
                      m_frustumPlanes[0][1] * m_frustumPlanes[0][1] + 
                      m_frustumPlanes[0][2] * m_frustumPlanes[0][2] );

    m_frustumPlanes[0][0] /= t;
    m_frustumPlanes[0][1] /= t;
    m_frustumPlanes[0][2] /= t;
    m_frustumPlanes[0][3] /= t;

    //
    // Extract the frustum's left clipping plane and normalize it.
    //

    m_frustumPlanes[1][0] = -minx * mvp[ 3] + mvp[ 0];
    m_frustumPlanes[1][1] = -minx * mvp[ 7] + mvp[ 4];
    m_frustumPlanes[1][2] = -minx * mvp[11] + mvp[ 8];
    m_frustumPlanes[1][3] = -minx * mvp[15] + mvp[12];

    t = (float) sqrt( m_frustumPlanes[1][0] * m_frustumPlanes[1][0] + 
                      m_frustumPlanes[1][1] * m_frustumPlanes[1][1] + 
                      m_frustumPlanes[1][2] * m_frustumPlanes[1][2] );

    m_frustumPlanes[1][0] /= t;
    m_frustumPlanes[1][1] /= t;
    m_frustumPlanes[1][2] /= t;
    m_frustumPlanes[1][3] /= t;

    //
    // Extract the frustum's bottom clipping plane and normalize it.
    //

    m_frustumPlanes[2][0] = -miny * mvp[ 3] + mvp[ 1];
    m_frustumPlanes[2][1] = -miny * mvp[ 7] + mvp[ 5];
    m_frustumPlanes[2][2] = -miny * mvp[11] + mvp[ 9];
    m_frustumPlanes[2][3] = -miny * mvp[15] + mvp[13];

    t = (float) sqrt( m_frustumPlanes[2][0] * m_frustumPlanes[2][0] + 
                      m_frustumPlanes[2][1] * m_frustumPlanes[2][1] + 
                      m_frustumPlanes[2][2] * m_frustumPlanes[2][2] );

    m_frustumPlanes[2][0] /= t;
    m_frustumPlanes[2][1] /= t;
    m_frustumPlanes[2][2] /= t;
    m_frustumPlanes[2][3] /= t;

    //
    // Extract the frustum's top clipping plane and normalize it.
    //

    m_frustumPlanes[3][0] = maxy * mvp[ 3] - mvp[ 1];
    m_frustumPlanes[3][1] = maxy * mvp[ 7] - mvp[ 5];
    m_frustumPlanes[3][2] = maxy * mvp[11] - mvp[ 9];
    m_frustumPlanes[3][3] = maxy * mvp[15] - mvp[13];

    t = (float) sqrt( m_frustumPlanes[3][0] * m_frustumPlanes[3][0] + 
                      m_frustumPlanes[3][1] * m_frustumPlanes[3][1] + 
                      m_frustumPlanes[3][2] * m_frustumPlanes[3][2] );

    m_frustumPlanes[3][0] /= t;
    m_frustumPlanes[3][1] /= t;
    m_frustumPlanes[3][2] /= t;
    m_frustumPlanes[3][3] /= t;

    //
    // Extract the frustum's far clipping plane and normalize it.
    //

    m_frustumPlanes[4][0] = maxz * mvp[ 3] - mvp[ 2];
    m_frustumPlanes[4][1] = maxz * mvp[ 7] - mvp[ 6];
    m_frustumPlanes[4][2] = maxz * mvp[11] - mvp[10];
    m_frustumPlanes[4][3] = maxz * mvp[15] - mvp[14];

    t = (float) sqrt( m_frustumPlanes[4][0] * m_frustumPlanes[4][0] +  
                      m_frustumPlanes[4][1] * m_frustumPlanes[4][1] + 
                      m_frustumPlanes[4][2] * m_frustumPlanes[4][2] );

    m_frustumPlanes[4][0] /= t;
    m_frustumPlanes[4][1] /= t;
    m_frustumPlanes[4][2] /= t;
    m_frustumPlanes[4][3] /= t;

    //
    // Extract the frustum's near clipping plane and normalize it.
    //

    m_frustumPlanes[5][0] = -minz * mvp[ 3] + mvp[ 2];
    m_frustumPlanes[5][1] = -minz * mvp[ 7] + mvp[ 6];
    m_frustumPlanes[5][2] = -minz * mvp[11] + mvp[10];
    m_frustumPlanes[5][3] = -minz * mvp[15] + mvp[14];

    t = (float) sqrt( m_frustumPlanes[5][0] * m_frustumPlanes[5][0] + 
                      m_frustumPlanes[5][1] * m_frustumPlanes[5][1] + 
                      m_frustumPlanes[5][2] * m_frustumPlanes[5][2] );

    m_frustumPlanes[5][0] /= t;
    m_frustumPlanes[5][1] /= t;
    m_frustumPlanes[5][2] /= t;
    m_frustumPlanes[5][3] /= t;
    
    
    if(PRINT_FRUSTUM_PLANES)
    {
    	for(int i=0; i<6; i++)
	{
		printf("%f, %f, %f, %f\n", m_frustumPlanes[i][0], m_frustumPlanes[i][1], m_frustumPlanes[i][2],
		m_frustumPlanes[i][3]);
	}
    }
}

//pID: plane ID
float CFrustumCull::distanceToPlane(int pID, float *p)
{
	float ret = m_frustumPlanes[pID][0] * p[0] +
            m_frustumPlanes[pID][1] * p[1] +
            m_frustumPlanes[pID][2] * p[2] +
            m_frustumPlanes[pID][3];
	return ret;
}


/*
//scan convert the projected bounding polygon made of front faces
bool CFrusutumCull::ScanProjectedBoundingPolygon(float scanline,
                                                 float size,
                                                 float c[3],
                                                 float inc[2])
{
    inc[0] = 1.;
    inc[1] = -1.;
    bool ret = false;

    float vCorner[24], wCorner[24];

    getBoxCorners(size, c, vCorner);
    for(int i=0; i<24; i++)
    {
        get_clipCoord(vCorner+3*i, wCorner+3*i);
    }

    GLubyte indice[6][4] = {
        {0, 1, 3 ,2}, //left
        {4, 6, 7, 5}, //right
        {0, 4, 5, 1}, //bottom
        {2, 3, 7, 6}, //up
        {3, 1, 5, 7}, //front
        {0, 2, 6, 4} }; //back

    unsigned char frontfaces[6];
    memset(frontface, 0, 6);

    for(int face=0; face<6; i++)
    {
        float signedarea = 0.;
        for(int v=0; v<4; v++)
        {
            int k = indice[face][v];
            int nv = (v+1)%4;
            int nk = indice[face][nv];
            float x = wCorner[k*3];
            float y = wCorner[k*3+1];
            float nx = wCorner[nk*3];
            float ny = wCorner[nk*3+1];
            signedarea += (x * ny - nx * y);
        }
        if(signedarea > 0.) frontfaces[face] = 1;


        if(signedarea > 0.)
        {
            //divide one face into two triangles
            float incT[2] = {1., -1.};
            int v1 = indice[face][0];
            int v2 = indice[face][1];
            int v3 = indice[face][2];
            bool retT;
            retT = scanTriangle(scanline, 
                         wCorner[3*v1], wCorner[3*v1+1],
                         wCorner[3*v2], wCorner[3*v2+1],
                         wCorner[3*v3], wCorner[3*v3+1],
                         inc1);
            if(incT[0] < inc[0]) inc[0] = incT[0];
            if(incT[1] > inc[1]) inc[1] = incT[1];
            ret = ret || retT;

            v1 = indice[face][2];
            v2 = indice[face][0];
            v3 = indice[face][3];
            retT = scanTriangle(scanline, 
                         wCorner[3*v1], wCorner[3*v1+1],
                         wCorner[3*v2], wCorner[3*v2+1],
                         wCorner[3*v3], wCorner[3*v3+1],
                         inc2);
            if(incT[0] < inc[0]) inc[0] = incT[0];
            if(incT[1] > inc[1]) inc[1] = incT[1];
            ret = ret || retT;

        }
    }

    return ret;
}

bool CFrusutumCull::ScanTriangle(float v[6], float deltay,
                                 float *segments)
{
    inc[0] = 1.;
    inc[1] = -1.;
    bool ret = false;
    float scanline;

    //segment format: 
    //miny, maxy, seg1, seg2, ......

    std::multimap<float, float> m;
    for(int i=0; i<3; i++)
    {
        m.insert(std::pair<float, float>(v[2*i+1], v[2*i]));
    }

    std::multimap<float, float>::iterator it;

    for(int i=0, it = m.being(); it< m.end(); i++, it++)
    {
        std::cout << it->first << "," << it->second << std::endl;
        v[2*i+1] = it->first;
        v[2*i] = it->second;
    }

    //vertex 1 (min):    v[0], v[1]
    //vertex 2 (median): v[2], v[3]
    //vertex 3 (max):    v[4], v[5]
    //if(scanline < v[1] || scanline > v[5]) return false;

    //float deltay = 2.0/1024; //one pixel
    int nlines = (v[5] - v[1])/deltay;
    segments = new float[2 + nlines*2];
    segments[0] = v[1];
    segments[1] = v[5];

    float t12 = (v[2]-v[0])/(v[3]-v[1]);
    float t13 = (v[4]-v[0])/(v[5]-v[1]);
    float t23 = (v[4]-v[2])/(v[5]-v[3]);

    //special cases: 
    //fabs(v[1]-v[3]) < deltay
    //fabs(v[1]-v[5]) < deltay
    //fabs(v[3]-v[5]) < deltay

    int i=1;
    for(scanline = v[1]+deltay; scanline < v[3]; scanline += deltay)
    {
        segments[2*i] = v[0] + t12*deltay;
        segments[2*i+1] = v[0] + t13*deltay;
        i++;
    }
    for(scanline = v[3]; scanline < v[5]; scanline += deltay)
    {
        segments[2*i] = v[2] + t23*deltay;
        segments[2*i+1] = v[0] + t13*deltay;
        i++;
    }    

    return ret;
}
*/

//-----------------------------------------------------------------------------
// Name: isBoundingSphereInFrustum()
// Desc: 
//-----------------------------------------------------------------------------
int CFrustumCull::isBoundingSphereInFrustum( float *c, float fRadius )
{
    int count = 0;

    for( int i = 0; i < 6; ++i )
    {
        float dis = distanceToPlane(i, c);
        if( dis <=-fRadius)
        {
            return OUT1;
        }
        else if(dis > fRadius)
        {
            count++;
        }
    }

    if(count==6)	return IN1;
    else return INTERSECT;
}

// tests if a AaBox is within the frustrum
int CFrustumCull::isBoundingBoxInFrustum(float *c, float *size)
{
    int iTotalIn = 0;
    float vCorner[24];

    getBoxCorners(size, c, vCorner);

    // if all points are behind 1 specific plane, we are out
    // if we are in with all points, then we are fully in
    for(int p = 0; p < 6; ++p)
    {
	
        int iInCount = 8;
        int iPtIn = 1;		
        for(int i = 0; i < 8; ++i)
        {
            if( distanceToPlane(p, vCorner+3*i) < 0 ) 
            {
                iPtIn = 0;
                --iInCount;
            }
        }		
        if(iInCount == 0)
            return(OUT1);		
        iTotalIn += iPtIn;
    }	
    // so if iTotalIn is 6, then all are inside the view
    if(iTotalIn == 6)
        return(IN1);	
    return(INTERSECT);
}


// void CFrustumCull::get_eyeCoord(float *p, float *ecp)
// {
//     ecp[0] = mv[0]*p[0] + mv[4]*p[1] + mv[8]*p[2] + mv[12];
//     ecp[1] = mv[1]*p[0] + mv[5]*p[1] + mv[9]*p[2] + mv[13];
//     ecp[2] = mv[2]*p[0] + mv[6]*p[1] + mv[10]*p[2] + mv[14];
//     float w = mv[3]*p[0] + mv[7]*p[1] + mv[11]*p[2] + mv[15];
//     ecp[0] /= w;
//     ecp[1] /= w;
//     ecp[2] /= w;
// }


void CFrustumCull::get_clipCoord(float *p, float *ccp)
{
    ccp[0] = mvp[0]*p[0] + mvp[4]*p[1] + mvp[8]*p[2] + mvp[12];
    ccp[1] = mvp[1]*p[0] + mvp[5]*p[1] + mvp[9]*p[2] + mvp[13];
    ccp[2] = mvp[2]*p[0] + mvp[6]*p[1] + mvp[10]*p[2] + mvp[14];
    float w = mvp[3]*p[0] + mvp[7]*p[1] + mvp[11]*p[2] + mvp[15];
    ccp[0] /= w;
    ccp[1] /= w;
    ccp[2] /= w;

    //imply using clip<float>
    ccp[0] = clip(ccp[0], -1.0f, 1.0f);
    ccp[1] = clip(ccp[1], -1.0f, 1.0f);
    ccp[2] = clip(ccp[2], -1.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//given box center p, and size sz, 
//return screen bounding rect: minx, maxx, miny, maxy
//-----------------------------------------------------------------------------
bool CFrustumCull::get_boundingbox(const float* p, const float* sz, float* sp)
{    
//     if( isBoundingBoxInFrustum(p, sz) == OUT1)
//     {
//         return false;
//     }


    float vCorner[24];
    getBoxCorners(sz, p, vCorner);

    float ccp[3];
    sp[0] = 1.0;
    sp[1] = -1.0;
    sp[2] = 1.0;
    sp[3] = -1.0;

    for(int i=0; i<8; i++)
    {
        get_clipCoord(vCorner+3*i, ccp);

        if(ccp[0] > sp[1]) sp[1] = ccp[0];
        if(ccp[0] < sp[0]) sp[0] = ccp[0];
        if(ccp[1] > sp[3]) sp[3] = ccp[1];
        if(ccp[1] < sp[2]) sp[2] = ccp[1];    
    }
    return true;
}


bool CFrustumCull::intersect_two_bbox(float *sp1, float* sp2, float* nsp)
{
    nsp[0] = std::max(sp1[0], sp2[0]);
    nsp[1] = std::min(sp1[1], sp2[1]);
    nsp[2] = std::max(sp1[2], sp2[2]);
    nsp[3] = std::min(sp1[3], sp2[3]);

    if( (nsp[0] >= nsp[1]) || (nsp[2] >= nsp[3]) ) return false;
    else return true;
}

//------------------------------------------------------------------
//giving the cutting plane, find out if the left side is front
//left side is at the negative cutting plane normal direction
//right side is at the cutting plane normal direction
//------------------------------------------------------------------
bool CFrustumCull::front(int cut_direction, float cut_position)
{
    bool ret = true;

    //first, find the plane which completely seperates the two cubes 
    //this plane defines two half spaces
    //think of the kdtree cutting. this plane is the cutting plane!
    //object space cutting plane
    float p[4] = {0., 0., 0., 0.};
    p[cut_direction] = 1.0;
    p[3] = -cut_position;

    //object space view position
    //-(mv[12], mv[13], mv[14]) rotated by transpose of roation
    float v[4] = {0., 0., 0., 1.};
//     v[0] = -(mv[0] * mv[12] + mv[4] * mv[13] + mv[8] * mv[14]);
//     v[1] = -(mv[1] * mv[12] + mv[5] * mv[13] + mv[9] * mv[14]);
//     v[2] = -(mv[2] * mv[12] + mv[6] * mv[13] + mv[10] * mv[14]);
    v[0] = -(mv[0] * mv[12] + mv[1] * mv[13] + mv[2] * mv[14]);
    v[1] = -(mv[4] * mv[12] + mv[5] * mv[13] + mv[6] * mv[14]);
    v[2] = -(mv[8] * mv[12] + mv[9] * mv[13] + mv[10] * mv[14]);
    

    //this way, the front cube always has the eye in its side of the half space
    float d = 0.;
    for(int i=0; i<4; i++) d += p[i] * v[i];
    ret = (d <= 0.) ? true: false;
    
//     std::cout << "eye(" << v[0] << "," << v[1] << "," << v[2] << ")\n";
    return ret;
} 
