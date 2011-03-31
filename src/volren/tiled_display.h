#ifndef TILED_DISPLAY_H
#define TILED_DISPLAY_H

#include "binaryTree.h"
#include "nodeDimension.h"
#include "nodeRectile.h"

class Tiled_Display
{
public:
    typedef binaryTree<NodeRectile> screenTree;

    Tiled_Display(int w, int h, int row, int col);
    ~Tiled_Display();

    const std::vector<NodeRectile*>& get_tiles() const
    {
        return tiles;
    }

    NodeRectile* get_tile(int id){ return tiles[id]; }

public:
    void init_screenTree(int, int);

    void cross_tile(int* bbox, std::vector<int>& tileids);
    
private:
    int m_wid;
    int m_hei;
    int m_row, m_col;

    screenTree* m_screen;

    std::vector<NodeRectile*> tiles;
};


#endif
