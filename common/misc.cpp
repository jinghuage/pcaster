//misc.cpp
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32

#include <time.h>
#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}

#else
#include <sys/time.h>
#endif


#include <string.h>
#include <math.h>


#include "misc.h"
#include "glheaders.h"

#define dtor(a) (3.1416*a/180.0)



//------------------------
//time
//------------------------
double getTimeInSecs() 
{

    //static struct tms tb;
    //finish = times(&tb);
    const double oneMicroSec = 0.000001;
    struct timeval timeSec;
    gettimeofday(&timeSec, NULL);
    return ((double) timeSec.tv_sec + (double) timeSec.tv_usec * oneMicroSec);


}

//-----------------------------------------------------------------------------
// change the brightness
//-----------------------------------------------------------------------------
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst)
{
    if(!src || !dst)
        return;

    int value;
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            value = *src + shift;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src;
            ++dst;

            ++src;    // skip alpha
            ++dst;
        }
    }
}



//-----------------------------------------------------------------------------
// add two pointers
//-----------------------------------------------------------------------------
void add(unsigned char* src0, unsigned char* src1, 
         int width, int height, unsigned char* dst)
{
    if(!src0 || !src1 || !dst)
        return;

    int value;
    for(int i = 0; i < height; ++i)
    {
        for(int j = 0; j < width; ++j)
        {
            value = *src0 + *src1;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src0;
            ++src1;
            ++dst;

            value = *src0 + *src1;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src0;
            ++src1;
            ++dst;

            value = *src0 + *src1;
            if(value > 255) *dst = (unsigned char)255;
            else            *dst = (unsigned char)value;
            ++src0;
            ++src1;
            ++dst;

            ++src0;    // skip alpha
            ++src1;
            ++dst;
        }
    }
}

//-----------------------------------------------------------------------------
/// 1-D: f(x) = exp( x * x / ( -2 * s * s ) ) / ( s * sqrt( 2 * PI ) )
/// 2-D: f(x, y) = exp( x * x + y * y / ( -2 * s * s ) ) / ( s * s * 2 * PI )  
//-----------------------------------------------------------------------------  
void gaussian_filter(int size, unsigned char* v, int filter_size)
{
    printf("gaussian filter size=%d\n", filter_size);
    float xsigma, sum = 0.0f;
    float sumr, sumg, sumb, suma;
    float *fx = new float[2*filter_size + 1];
    int i, j, index;

    for(i=-filter_size; i<=filter_size; i++)
    {
        //choose window: x/sigma = [-4, 4] 
        xsigma = 4.0 * i/filter_size;
        fx[filter_size + i] = exp( -0.5 * xsigma * xsigma);
        sum += fx[filter_size + i];
    }
    for(i=-filter_size; i<=filter_size; i++) 
    {
        fx[filter_size + i] /= sum;
        printf("%f ", fx[filter_size + i]);
    }
    printf("\n");

    for(int i=0; i<size; i++)
    {
        sumr = sumg = sumb = suma = 0.0f;
        for(j=-filter_size; j<=filter_size; j++)
        {
            index = i+j;
            if(index < 0) index = 0;
            else if(index >= size) index = size-1;
            sumr += v[4*index+0] * fx[filter_size + j];
            sumg += v[4*index+1] * fx[filter_size + j];
            sumb += v[4*index+2] * fx[filter_size + j];
            suma += v[4*index+3] * fx[filter_size + j];
        }
        v[4*i+0] = (unsigned char)sumr;
        v[4*i+1] = (unsigned char)sumg;
        v[4*i+2] = (unsigned char)sumb;
        v[4*i+3] = (unsigned char)suma;
    }
}





static void  getVectorFromAngle(float ori_angle[3], float ori_vec[3])
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

/*
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
*/




