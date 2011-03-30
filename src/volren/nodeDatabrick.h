#ifndef NODE_DATABRICK_H_
#define NODE_DATABRICK_H_

#include <string>
#include <vector>

#include "nodeDimension.h"

//------------------------------------------------------------------
//NodeBase for 3D data tree
//how to split: divide into databricks
//stop condition: number of databrick threshold
//NodeItem: databrick bounding box. (here same as the dimension bounding box)
//------------------------------------------------------------------
class CFrustumCull;

class NodeDatabrick : public NodeDimension
{
public:
    NodeDatabrick();
    NodeDatabrick(int);
    NodeDatabrick(int, float[3]);
    //copy constructor
    //default shallow copy constructor should be ok
/*     NodeDatabrick(const NodeDatabrick&); */

    virtual ~NodeDatabrick();

protected:
    int m_bricksize;
    float m_physical_scale[3]; //overall data physical scale

public:
    void set_bricksize(int s) {m_bricksize = s;}
    bool compute_projection_bbox(CFrustumCull* fculler, float* bb);
    const float* get_physical_scale(){ return m_physical_scale; }


    friend std::ostringstream& operator<<(std::ostringstream&,
                                           NodeDatabrick&);

    virtual void split_data(int);
    virtual void draw(); //draw the databrick bounding box
    virtual void draw_cut();
    virtual void print();
    virtual bool apply_condition(); //can be further splited or not
    virtual bool howto_split(); //determine how to split
};


#endif
