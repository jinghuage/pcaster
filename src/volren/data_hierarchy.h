#ifndef DATA_HIERARCHY_H
#define DATA_HIERARCHY_H

#include "binaryTree.h"
#include "nodeDimension.h"
#include "nodeDatabrick.h"
#include "frustumCull.h"

class Data_Hierarchy
{
public:
    typedef binaryTree<NodeDatabrick> dataTree;

    Data_Hierarchy();
    ~Data_Hierarchy();


    const std::vector<NodeDatabrick*>& get_data_nodes() const
    {
        return dnodes;
    }

    NodeDatabrick* get_node(int id){ return dnodes[id]; }
    const int* get_sortarray(){ return m_sortarray; }

public:
    void init_dataTree(int, float[3]);
    void sort_data_nodes(CFrustumCull* fculler);
    void set_stencil(int id);
    void draw_node_bboxes(int start, int end, int LP);
    void compute_bbox(int id, CFrustumCull* fculler, float pbr[4]);

private:

    dataTree* data_root;
    int* m_sortarray;

    std::vector<NodeDatabrick*> dnodes;

};


#endif
