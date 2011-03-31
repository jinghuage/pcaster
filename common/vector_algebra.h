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
#ifndef VECTOR_ALGEBRA_H_
#define VECTOR_ALGEBRA_H_


float dot(float[3], float[3]);
void cross(float[3], float[3], float[3]);
float normalize(float[3]);
void getVectorFromAngle(float[3], float[3]);
void calculate_eyepos(float*, float*, float*, float*);
void projectVectorToPlane(float[3], float[3], float[3]);
void setTangentSpace(float[3], float[3], float[3]);
void  transformToWorldSpace(float T[3], float B[3], float N[3], float m[16]);
void  transformToWorldSpace(float N[3], float m[16]);
void  transformToTangentSpace(float T[3], float B[3], float N[3], float m[16]);
void setupFrustum(float eye[3], 
                  float screenBL[3],
                  float screenBR[3],
                  float screenTL[3], 
                  float n, float f);
void parameterizeTriangle(float V0[3], float V1[3], float V2[3], float V[3],
                          float s, float t);


//void transform2DOrtho(float scl[2], float rot, float trl[2], float m[9]);



#endif

