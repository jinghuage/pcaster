#include "glheaders.h"
#include "draw_routines.h"

#include "data_hierarchy.h"


Data_Hierarchy::Data_Hierarchy():
    data_root(0),
    m_sortarray(0)
{
}


Data_Hierarchy::~Data_Hierarchy()
{
#ifdef _DEBUG
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    if(m_sortarray){ delete[] m_sortarray; m_sortarray=0; }
}


//-----------------------------------------------------------------------------
//init data tree
//-----------------------------------------------------------------------------
void Data_Hierarchy::init_dataTree(int blocknum, float scale[3])
{
#ifdef _DEBUG7    
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

#endif

    fprintf(stderr, "dataTree: blocknum=%d, physical scale=(%.3f, %.3f, %.3f)\n",
            blocknum, scale[0], scale[1], scale[2]);


    data_root = new dataTree(NodeDatabrick(blocknum, scale));
    data_root->build_tree();
    data_root->rank_tree(0);


//     data_root->print_tree();
//     std::cout << "\n";        
    

    //save the nodedata for later use
    data_root->retrieve_nodedata(dnodes);
	
    m_sortarray = new int[blocknum];
    for(int i=0; i<blocknum; i++) m_sortarray[i] = i;
}



void Data_Hierarchy::sort_data_nodes(CFrustumCull* fculler)
{
    fculler->calculateMvpMatrix();

    data_root->sort_tree(fculler, 0, m_sortarray);
}




//-------------------------------------------------------------------------------
//set stencil to one of my nodes' footprint coverage
//don't forget to call glEnable(GL_STENCIL_TEST) before this function
//-------------------------------------------------------------------------------
void Data_Hierarchy::set_stencil(int id)
{
    
    //glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClear(GL_STENCIL_BUFFER_BIT);

    //set original stencil op
    glStencilFunc(GL_ALWAYS, 0x1, 0x1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    //don't write into the color buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    //also disable texturing and any fancy shaders

    //render the major occluder(current node's footprint) into stencil
    dnodes[id]->draw();

    //set stencil test op
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);


    if( printOpenGLError() )
    {
        std::cout << __FILE__ << ": " << __func__ <<"()\n";
        exit(1);
    }
}


void Data_Hierarchy::draw_node_bboxes(int start, int end, int LP)
{
    GLubyte c[8] = {1, 2, 4, 8, 16, 32, 64, 128};


    for(int id = start; id <= end; id++)
    {
        int d = id % LP + 1;
        int m = 0;
        GLubyte color[4] = {0, 0, 0, 0};

        //A-B-G-R -> color[3, 2, 1, 0]
        for(int k=3; k>=0; k--)
        {
            m = std::max(0, d - k*8);
            d -= m;

            if(m>0 && m<=8) { color[k] = c[m-1]; break; }
        }

//         if(myrank == 0)
//         std::cout << id << ":" 
//                   << (int) color[0] << "," 
//                   << (int) color[1] << "," 
//                   << (int) color[2] << ","
//                   << (int) color[3] << "\n";

        const float *bpos = dnodes[id]->get_pos();
        const float *bscale = dnodes[id]->get_scale();

        glColor4ub(color[0], color[1], color[2], color[3]);
        drawPlainBox(bscale, bpos);
        //todo: why not call this function?
        //dnodes[m_id]->draw();
    }
}

void Data_Hierarchy::compute_bbox(int id, CFrustumCull* fculler, float pbr[4])
{
    fculler->calculateMvpMatrix();
    dnodes[id]->compute_projection_bbox(fculler, pbr);
}
