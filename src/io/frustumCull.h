#ifndef FRUSTUM_CULL_H_
#define FRUSTUM_CULL_H_


#include "glheaders.h"


class CFrustumCull
{
private:
	float m_frustumPlanes[6][4];

        float mv[16];
        float p[16];
        float mvp[16];
public:

        enum PosRelation{IN1=0, OUT1, INTERSECT};

	CFrustumCull();
	~CFrustumCull();

        void calculateMvpMatrix();
	void calculateFrustumPlanes(float minx = -1, float maxx = 1, 
                                    float miny = -1, float maxy = 1, 
                                    float minz = -1, float maxz = 1) ;        
	float distanceToPlane(int pID, float* p);

	int isBoundingSphereInFrustum( float* c, float fRadius );
	int isBoundingBoxInFrustum( float* c, float* size );
        //void get_eyeCoord(float *p, float *ecp);
        void get_clipCoord(float *p, float *ccp);
        bool get_boundingbox(const float* p, const float* size, float *sp);
        bool intersect_two_bbox(float *sp1, float *sp2, float *nsp);

        bool front(int, float);

	float* getFrustumPlane(int p) {return m_frustumPlanes[p];}
	
	void print();
};

#endif

