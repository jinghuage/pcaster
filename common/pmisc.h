#ifndef MISC_H_
#define MISC_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <vector>

#ifdef _WIN32

# ifdef __FUNCTION__
#  define __func__  __FUNCTION__
# else
#  define __func__  __FILE__
# endif

#endif

#ifndef M_PI
#define M_PI 3.1415926
#endif

//time
double getTimeInSecs();
//eye
//void caculate_eyepos(float* headPos, float* headOri, float* leftEye, float* rightEye);

//compute
unsigned char* create_Preintegration_Table(unsigned char*);
void gaussian_filter(int, unsigned char*, int);
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst);
void add(unsigned char* src0, unsigned char* src1, int width, int height, unsigned char* dst);


//#define max(a, b) (a) > (b) ? (a) : (b)
//#define min(a, b) (a) < (b) ? (a) : (b)


template <class T>
T clip( T val, T min, T max )
{
    return ( val<min ? min : ( val>max ? max : val ) );
}

template <class T>
T swap(T n, T a, T b)
{
    assert(n == a || n == b);
    if(n == a) return b;
    else if(n == b) return a;
    else return 0;
}

template <class T>
void print_vector(std::vector<T>& V, const char* D)
{
    typename std::vector<T>::iterator it;
    std::cout << D;
    for(it=V.begin(); it<V.end(); it++)
    {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}


template<class T>
float* calculateDerivatives( void *volume,
                                  unsigned w, unsigned h, unsigned d  )
{

    printf("--------------------------------------------------------\n");
    printf("DEBUG: %s @ %d: calculateDerivatives\n", __FILE__, __LINE__);
    printf("--------------------------------------------------------\n");

    printf("w=%d, h=%d, d=%d\n", w, h, d);

    int wh = w*h;

    float* GxGyGz = (float*)malloc( wh*d*3 * sizeof(float) );
    memset(GxGyGz, 0, wh*d*3*sizeof(float));

    for( unsigned z=1; z<d-1; z++ )
    {
	printf("z=%d\n", z);

        int zwh = z*wh;

        const T *curPz = (T*)volume + zwh;

//	printf("volume[0]=%d, zwh=%d, volume[zwh]=%d\n", volume[0], zwh, volume[zwh]);

        for( unsigned y=1; y<h-1; y++ )
        {
            int zwh_y = zwh + y*w;
            const T *curPy = curPz + y*w ;
            for( unsigned x=1; x<w-1; x++ )
            {
                const T *curP = curPy +  x;
                const T *prvP = curP  - wh;
                const T *nxtP = curP  + wh;
	
		//printf("*curPy=%d, *curP=%d, *prvP=%d, *nxtP=%d\n", *curPy, *curP, *prvP, *nxtP);

		//printf("prvP[-1-w]=%d\n", *(prvP -1-w ));
		
/*	
                int32_t gx = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[  1   ]+ 6*curP[  1   ]+ 3*prvP[  1   ]+
                      nxtP[  1-w ]+ 3*curP[  1-w ]+   prvP[  1-w ]-

                      nxtP[ -1+w ]- 3*curP[ -1+w ]-   prvP[ -1+w ]-
                    3*nxtP[ -1   ]- 6*curP[ -1   ]- 3*prvP[ -1   ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];

                int32_t gy = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[    w ]+ 6*curP[    w ]+ 3*prvP[    w ]+
                      nxtP[ -1+w ]+ 3*curP[ -1+w ]+   prvP[ -1+w ]-

                      nxtP[  1-w ]- 3*curP[  1-w ]-   prvP[  1-w ]-
                    3*nxtP[   -w ]- 6*curP[   -w ]- 3*prvP[   -w ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];

                int32_t gz = 
                      nxtP[  1+w ]+ 3*nxtP[  1   ]+   nxtP[  1-w ]+
                    3*nxtP[    w ]+ 6*nxtP[  0   ]+ 3*nxtP[   -w ]+
                      nxtP[ -1+w ]+ 3*nxtP[ -1   ]+   nxtP[ -1-w ]-

                      prvP[  1+w ]- 3*prvP[  1   ]-   prvP[  1-w ]-
                    3*prvP[   +w ]- 6*prvP[  0   ]- 3*prvP[   -w ]-
                      prvP[ -1+w ]- 3*prvP[ -1   ]-   prvP[ -1-w ];
*/
//rewite stuff for negative index:
//
                double gx = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[  1   ]+ 6*curP[  1   ]+ 3*prvP[  1   ]+
                      (*(nxtP+1-w) )+ 3*(*(curP+ 1-w) )+  (*(prvP+1-w) )-

                      nxtP[ -1+w ]- 3*curP[ -1+w ]-   prvP[ -1+w ]-
                    3*(*(nxtP -1))- 6*(*(curP -1))- 3*(*(prvP -1 ))-
                      (*(nxtP-1-w))- 3*(*(curP-1-w))-  (*(prvP-1-w));

                double gy = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[    w ]+ 6*curP[    w ]+ 3*prvP[    w ]+
                      nxtP[ -1+w ]+ 3*curP[ -1+w ]+   prvP[ -1+w ]-

                      (*(nxtP+ 1-w))- 3*(*(curP+ 1-w))-(*(prvP+1-w))-
                    3*(*(nxtP -w ))- 6*(*(curP-w))- 3*(*(prvP-w))-
                      (*(nxtP-1-w))- 3*(*(curP-1-w))-  (*(prvP-1-w));

                double gz = 
                      nxtP[  1+w ]+ 3*nxtP[  1   ]+   (*(nxtP+1-w ))+
                    3*nxtP[    w ]+ 6*nxtP[  0   ]+ 3*(*(nxtP-w))+
                      nxtP[ -1+w ]+ 3*(*(nxtP-1)) +   (*(nxtP-1-w))-

                      prvP[  1+w ]- 3*prvP[  1   ]-   (*(prvP+1-w))-
                    3*prvP[   +w ]- 6*prvP[  0   ]- 3*(*(prvP-w ))-
                      (*(prvP-1+w ))- 3*(*(prvP-1 ))-  (*( prvP-1-w));
    

                GxGyGz[(zwh_y + x)*3   ] = static_cast<float>( gx );
                GxGyGz[(zwh_y + x)*3 +1] = static_cast<float>( gy );
                GxGyGz[(zwh_y + x)*3 +2] = static_cast<float>( gz );

            }
        }
    }


    return GxGyGz;

}



#endif /*MISC_H_*/
