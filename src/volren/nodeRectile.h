#ifndef NODE_RECTILE_H_
#define NODE_RECTILE_H_

#include <string>
#include <vector>

#include "nodeDimension.h"

//------------------------------------------------------------------
//NodeBase for 2D screen configuration as tiled-display
//how to split: divide 2D screen into display tiles
//stop condition: number of tiles total
//NodeItem: 2D tile. m_k=2 
//------------------------------------------------------------------


class NodeRectile : public NodeDimension
{
public:
    NodeRectile(int, int);
    //copy constructor
    //default shallow copy constructor should be ok
/*     NodeRectile(const NodeRectile&); */

    virtual ~NodeRectile();

protected:
    int m_screen_dims[2]; //rows and columns


public:

    friend std::ostringstream& operator<< (std::ostringstream&, 
                                           const NodeRectile&);

    virtual void split_data(int);
    virtual void draw(); //draw the tile boundary
    virtual void draw_cut();
    virtual void print();
    virtual bool apply_condition(); //can be further splited or not
    virtual bool howto_split(); //determine how to split
};


#endif
