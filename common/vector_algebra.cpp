/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
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
#include <string.h>
#include <math.h>
#include <assert.h>

#include "glheaders.h"
#include "vector_algebra.h"


#define dtor(a) (3.1416*a/180.0)


//-----------------------------------------------------------------------------
// In homogeneous coord system: 
// 3D point: (x, y, z, 1)
// 3D vector: (x, y, z, 0) -- translation doesn't apply to vectors
// 
// Orthogonal transformation: 
// only glTranslate and glRotate are used, no glScale
//
// Normally, transform in this order: Scale, Rotate, Translate
// All relative to world coord system.
// gl function Calls should be issued backwards. 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Transform normal vector : gl_NormalMatrix
// for orthogonal transformation, gl_NormalMatrix = gl_ModelViewMatrix;
// for non-uniform scale and shear
// gl_NormalMatrix = gl_ModelViewMatrixTransposeInverse; 
//
// proof:
// We know that, prior to the matrix transformation T.N = 0, 
// since the vectors are by definition perpendicular. 
// We also know that after the transformation N'.T' must remain equal to zero, 
// since they must remain perpendicular to each other. 
// Let's assume that the matrix G is the correct matrix to transform 
// the normal vector. 
// T can be multiplied safely by the upper left 3x3 submatrix of 
// the modelview (T is a vector, hence the w component is zero). 
// This is because T can be computed as the difference between two vertices, 
// therefore the same matrix that is used to transform the vertices can be 
// used to transform T.
// N.T = Transpose(N) * T = 0
// N'.T' = (G*N) . (M*T) = 0
// The dot product can be transformed into a product of vectors, therefore:
// Transpose(G*N) * (M*T) = 0
// the transpose of a multiplication is the multiplication of the transposes
// Transpose(N) * Transpose(G) * M * T = 0
// if Transpose(G) * M = I, then the formula holds true
// thus: G = Transpose(Inverse(M)) #
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//input : vector V1, V2
//output: dot 
//-----------------------------------------------------------------------------

float dot(float V1[3], float V2[3])
{
    return ( V1[0]*V2[0] + V1[1]*V2[1] + V1[2]*V2[2] ); 
}


//-----------------------------------------------------------------------------
//input : vector T, N
//output: cross vector B
//-----------------------------------------------------------------------------

void cross(float V1[3], float V2[3], float B[3])
{
    B[0] = V1[1]*V2[2] - V1[2]*V2[1];
    B[1] = V1[2]*V2[0] - V1[0]*V2[2];
    B[2] = V1[0]*V2[1] - V1[1]*V2[0];
}

//-----------------------------------------------------------------------------
//input : vector V
//output: normalized vector, return len before normalization
//-----------------------------------------------------------------------------

float normalize(float V[3])
{
    float l = sqrt(V[0]*V[0] + V[1]*V[1] + V[2]*V[2]);
    V[0] /= l;
    V[1] /= l;
    V[2] /= l;

    return l;
}


//-----------------------------------------------------------------------------
//input : ori_angle[3] : rotation around x, y ,z
//output: vector in xyz coord system
//-----------------------------------------------------------------------------

void  getVectorFromAngle(float ori_angle[3], float ori_vec[3])
{
    float azim,elev,roll,cos_azim,sin_azim,sin_elev,cos_roll,sin_roll;
    azim = dtor(ori_angle[1]);
    elev = dtor(ori_angle[0]);
    roll = dtor(ori_angle[2]);
    cos_azim = cosf(azim);
    sin_azim = sinf(azim);
    sin_elev = sinf(elev);
    cos_roll = cosf(roll);
    sin_roll = sinf(roll);
	
    //float vec[3];
    ori_vec[0] = -cos_azim * cos_roll - sin_azim * sin_elev * sin_roll;
    ori_vec[1] = -cosf(elev) * sin_roll;
    ori_vec[2] = sin_azim * cos_roll - cos_azim * sin_elev * sin_roll;
}

void caculate_eyepos(float* headPos, float* headOri, float* leftEye, float* rightEye)
{

    float vec[3];
	
    getVectorFromAngle(headOri, vec);
	
    float HalfInterocularDistance = 2.5/24;
    for(int i=0; i<3; i++)
    {
        leftEye[i] = headPos[i] + vec[i]*HalfInterocularDistance;
        rightEye[i] = headPos[i] - vec[i]*HalfInterocularDistance;
    }   	
}

//-----------------------------------------------------------------------------
//input : vector V, and plane normal N
//output: projected vector pv from V to plane N
//-----------------------------------------------------------------------------

void  projectVectorToPlane(float V[3], float N[3], float pv[3])
{
    float d = dot(V, N);

    for(int i=0; i<3; i++) pv[i] = V[i] - d * N[i];

}

//-----------------------------------------------------------------------------
//input : normal N
//output: other two vectors in tangent space T and B
//-----------------------------------------------------------------------------

void  setTangentSpace(float N[3], float T[3], float B[3])
{
    float up[3] = {0., 1., 0.};

    float d = dot(up, N);
    if(d == 1) // N == up
    {
        T[0] = 0.; T[1] = 0.; T[2] = 1.;
    }
    else 
    {
        cross(up, N, T);
    }
    
    cross(N, T, B);
}




//-----------------------------------------------------------------------------
//input : vector T, B, N to make up the tangent space
//output: transformation matrix m (columns) from tangent space to world space
//
// relation:
// tangent space (1, 0, 0) to world space T
// tangent space (0, 1, 0) to world space B
// tangent space (0, 0, 1) to world space N
//
// usage scenario: 
// For attributes defined in local object space, e.g. local normal vectors, 
// need to transform to global world space for homogeneous transformation
// such as modelview and projection.  
//-----------------------------------------------------------------------------

void  transformToWorldSpace(float T[3], float B[3], float N[3], float m[16])
{

    m[0] = T[0]; m[4] = B[0]; m[8]  = N[0]; m[12] = 0.;
    m[1] = T[1]; m[5] = B[1]; m[9]  = N[1]; m[13] = 0.;
    m[2] = T[2]; m[6] = B[2]; m[10] = N[2]; m[14] = 0.;
    m[3] = 0.;   m[7] = 0.;   m[11] = 0.;   m[15] = 1.;
}


void  transformToWorldSpace(float N[3], float m[16])
{
    float T[3], B[3];

    setTangentSpace(N, T, B);
    transformToWorldSpace(T, B, N, m);
}


//-----------------------------------------------------------------------------
//input : vector T, B, N to make up the tangent space
//output: transformation matrix m (columns) from world space to tangent space 
//
// relation:
// world space X(1, 0, 0) to tangent space 
// (dot(X,T), dot(X,B), dot(X,N)) =  (T[0], B[0], N[0])
//
// tangent space Y(0, 1, 0) to world space 
// (dot(Y,T), dot(Y,B), dot(Y,N)) =  (T[1], B[1], N[1])
//
// tangent space Z(0, 0, 1) to world space 
// (dot(Z,T), dot(Z,B), dot(Z,N)) =  (T[2], B[2], N[2])
//
// note: 
// transpose of the matrix from tangent space to world space
//
// usage scenario: 
// Given specific eye position and screen plane in world space definition 
// frustum is setup according to eye + screen bounding rectangle, 
// world space object needs to transform first. 
//-----------------------------------------------------------------------------

void  transformToTangentSpace(float T[3], float B[3], float N[3], float m[16])
{

    m[0] = T[0]; m[4] = T[1]; m[8]  = T[2]; m[12] = 0.;
    m[1] = B[0]; m[5] = B[1]; m[9]  = B[2]; m[13] = 0.;
    m[2] = N[0]; m[6] = N[1]; m[10] = N[2]; m[14] = 0.;
    m[3] = 0.;   m[7] = 0.;   m[11] = 0.;   m[15] = 1.;
}


//-----------------------------------------------------------------------------
//input: eye position, screen corners, and near/far distance
//output: setup frustum
//-----------------------------------------------------------------------------

void setupFrustum(float eye[3], 
                  float screenBL[3],
                  float screenBR[3],
                  float screenTL[3], 
                  float n, float f)
{
    //setup screen space as tangent space
    float T[3], B[3], N[3], m[16];
    
    for(int i=0; i<3; i++) 
    {
        T[i] = screenBR[i] - screenBL[i];
        B[i] = screenTL[i] - screenBL[i];
    }
    cross(T, B, N);
    normalize(T);
    normalize(B);
    normalize(N);



    //vector from eye to screen corners
    float ebl[3], ebr[3], etl[3]; 
    for(int i=0; i<3; i++)
    {
        ebl[i] = eye[i] - screenBL[i];
        ebr[i] = eye[i] - screenBR[i];
        etl[i] = eye[i] - screenTL[i];
    }

    //see how dot is used here to get screen space metrics
    //vector project to vector
    float l = dot(T, ebl);
    float r = dot(T, ebr);
    float b = dot(B, ebl);
    float t = dot(B, etl);


    float k = -dot(N, ebl);
    k = n/k;

    l *= k;
    r *= k;
    b *= k;
    t *= k;


    //set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // printf("eye(%f, %f, %f), frustum(%f, %f, %f, %f, %f, %f)\n", 
    //        eye[0], eye[1], eye[2],
    //        l, r, b, t, n, f);
    glFrustum(l, r, b, t, n, f);

    transformToTangentSpace(T, B, N, m);
    glMultMatrixf(m);

    //set up modelview
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-eye[0], -eye[1], -eye[2]);
}


/*
//-----------------------------------------------------------------------------
//input : Homogeneous matrix m
//output: transpose mt
//-----------------------------------------------------------------------------

void  transposeHmg(float m[16], float mt[16])
{
    
}

//-----------------------------------------------------------------------------
//input : 3D matrix m
//output: transpose mt
//-----------------------------------------------------------------------------

void  transpose(float m[9], float mt[9])
{
    
}

//-----------------------------------------------------------------------------
//input : rotation angle[3] around x, y ,z
//output: rotation matrix m in homogeneous coord system
//-----------------------------------------------------------------------------

void  rotateHmg(float angle[3], float m[16])
{
    
}


//-----------------------------------------------------------------------------
//input : rotation angle[3] around x, y ,z
//output: rotation matrix m in 3D coord system
//-----------------------------------------------------------------------------

void  rotate(float angle[3], float m[9])
{
    
}


//-----------------------------------------------------------------------------
//input : translate trl[3] in x, y , z direction
//output: homogeneous matrix m
//-----------------------------------------------------------------------------

void  translateHmg(float trl[3], float m[16])
{
    
}


//-----------------------------------------------------------------------------
//input : scale scl[3] in x, y ,z direction
//output: scale matrix m in homogeneous coord system
//-----------------------------------------------------------------------------

void  scaleHmg(float scl[3], float m[16])
{
    
}


//-----------------------------------------------------------------------------
//input : scale scl[3] in x, y ,z direction
//output: scale matrix m in 3D coord system
//-----------------------------------------------------------------------------

void  scale(float scl[3], float m[9])
{
    
}



//-----------------------------------------------------------------------------
//input : scale scl[2] in x, y direction, rotation rot, translation trl[2]
//output: matrix m to scale, then rotate, then translate, in this order
//-----------------------------------------------------------------------------

void transform2DOrtho(float scl[2], float rot, float trl[2], float m[9])
{
    //matrix columns

    m[0] = sx * cos(rot); 
    m[1] = sx * sin(rot);
    m[2] = 0.0;

    m[3] = -sy * sin(rot);
    m[4] = sy * cos(rot);
    m[5] = 0.0;
    
    m[6] = cx;
    m[7] = cy;
    m[8] = 1.0
}
*/

//-----------------------------------------------------------------------------
//input:
//output:
//-----------------------------------------------------------------------------
// void func()
// {

// }

//-----------------------------------------------------------------------------
//input: triangle vertices V0, V1, V2
//output: a point V inside triangle, parameterize by s, t, (0<=s+t<=1)
//-----------------------------------------------------------------------------
void parameterizeTriangle(float V0[3], float V1[3], float V2[3], float V[3],
                          float s, float t)
{
    //V(s,t) = V0 + s*(V1-V0) + t*(V2-V0);
    //t=0, s<=1, point on vector V1-V0
    //s=0, t<=1, point on vector V2-V0
    //s+t=1, point on vector V2-V1
    //s+t<1, point inside triangle

    float k = s+t;
    assert(k >= 0 && k <= 1);

    for(int i=0; i<3; i++) 
        V[i] = V0[i] + s * (V1[i]-V0[i]) + t * (V2[i] - V0[i]);

}

