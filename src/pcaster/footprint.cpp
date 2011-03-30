#include <assert.h>

#include <glheaders.h>

#include "footprint.h"
#include "frustumCull.h"
#include "draw_routines.h"


FootPrint::FootPrint():
    m_id(0),
    m_layer(1),
    m_map(0),
    m_sl(0),
    m_slon(0)
{
    memset(m_sp, 0, 4*sizeof(int));
}


FootPrint::FootPrint(int id, int layer):
    m_id(id),
    m_layer(layer),
    m_map(0),
    m_sl(0),
    m_slon(0)
{
    memset(m_sp, 0, 4*sizeof(int));
}    

FootPrint::~FootPrint()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    clear();
}


//-------------------------------------------------------------------------------
//set stencil to a data nodes' footprint coverage
//don't forget to call glEnable(GL_STENCIL_TEST) before this function
//-------------------------------------------------------------------------------
void FootPrint::set_stencil(NodeDatabrick* node)
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
    node->draw();

    //set stencil test op
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);


    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);


    if( printOpenGLError() )   exit(1);

}


void FootPrint::draw_node_bboxes(std::vector<NodeDatabrick*>& dnodes, 
                                 int start, int end, int LP)
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

        const float *bpos = dnodes[id]->get_pos();
        const float *bscale = dnodes[id]->get_scale();

        glColor4ub(color[0], color[1], color[2], color[3]);
        drawPlainBox(bscale, bpos);
        //todo: why not call this function?
        //dnodes[m_id]->draw();
    }
}


void FootPrint::compute_bbox(NodeDatabrick* node,
                             CFrustumCull* fculler, float pbr[4])
{
    fculler->calculateMvpMatrix();
    node->compute_projection_bbox(fculler, pbr);
}


void FootPrint::compute_screenport(float pbr[4],
                                   int render_wid,
                                   int render_hei)
{

    int x0 = render_wid * (pbr[0] + 1.) / 2.;
    int x1 = render_wid * (pbr[1] + 1.) / 2.;
    int y0 = render_hei * (pbr[2] + 1.) / 2.;
    int y1 = render_hei * (pbr[3] + 1.) / 2.;
        
    x0 = std::max(0, x0-5);
    x1 = std::min(render_wid, x1+5);
    y0 = std::max(0, y0-5);
    y1 = std::min(render_hei, y1+5);

    m_sp[0] = x0;
    m_sp[2] = x1-x0;
    m_sp[1] = y0;
    m_sp[3] = y1-y0;

#ifdef _DEBUG6
    fprintf(stderr, "%d: footprint screenport(%d,%d,%d,%d)\n", 
            m_id, m_sp[0], m_sp[1], m_sp[2], m_sp[3]);
#endif
}


unsigned int* FootPrint::get_map()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_map==0){
        m_map = new unsigned int[m_sp[2] * m_sp[3]];
    }

    return m_map;
}



void FootPrint::read_map()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //glReadBuffer(GL_BACK);
    if(m_map==0){
        m_map = new unsigned int[m_sp[2] * m_sp[3]];
    }
    glReadPixels(m_sp[0], m_sp[1],
                 m_sp[2], m_sp[3], 
                 GL_RGBA, GL_UNSIGNED_BYTE, m_map);
}


void FootPrint::scan_map()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int wid = m_sp[2];
    int hei = m_sp[3];

    if(m_sl){ delete[] m_sl; m_sl=0; }
    m_sl = new ScanLine[hei];

    unsigned int* p = m_map;

    unsigned int colorkey = 0, p_colorkey = 0;

    bool segment_closed = true;

    //fp_mask is used for inside-footprint test for non-stenciled footprint
    //unsigned int fp_mask = 0;
    //if(m_id >=0) fp_mask = 1 << m_id;

    bool P = false;

    for(int i=0; i<hei; i++)
    {
        m_sl[i].set_id(i);

        p_colorkey = 0;
        for(int j=0; j<wid; j++)
        {
            colorkey = *p++;
            //inside footprint test, w or w/o stenciled footprint
            if(colorkey != 0) // && m_id < 0 ? true : (colorkey & fp_mask))
            {
                if(colorkey != p_colorkey)
                {
                    if(segment_closed) //new segment
                    {
                        m_sl[i].add_key(colorkey, m_layer);
                        m_sl[i].add_seg(j);
                        if(P) std::cout << "(" << j << ",";
                        segment_closed = false;
                    }
                    else //current segment
                    {
                        m_sl[i].add_seg(j-1);
                        if(P) std::cout << j-1 << "),";
                        m_sl[i].add_key(colorkey, m_layer);
                        m_sl[i].add_seg(j);
                        if(P) std::cout << "(" << j << ",";
                    }
                   
                    p_colorkey = colorkey;
                }
                if(segment_closed == false && j==wid-1) //close the segment at last
                {
                    m_sl[i].add_seg(j);
                    if(P) std::cout << j << "),";
                    segment_closed = true;
                }
            }
            else if(segment_closed == false)
            {
                m_sl[i].add_seg(j-1);
                if(P) std::cout << j-1 << "),";
                segment_closed = true;
                continue;
            }
        }

        assert(segment_closed == true);
    }
    if(P) std::cout << "\n";
}


FootPrint& FootPrint::operator+=(const FootPrint& rhs)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_sl == 0) m_sl = new ScanLine[m_sp[3]];

    for(int i=0; i < m_sp[3]; i++)
    {
        m_sl[i] += rhs.get_sl(i);
    }
    
    return *this;
}

FootPrint& FootPrint::operator=(const FootPrint& rhs)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(m_sl == 0) m_sl = new ScanLine[m_sp[3]];

    for(int i=0; i < m_sp[3]; i++)
    {
      
        m_sl[i] = rhs.get_sl(i);
    }
    
    return *this;
}


bool FootPrint::retrieve_seg(unsigned int& s, 
                             unsigned int& l, 
                             unsigned int& r, 
                             ColorKey& ck)
{   

    s = m_slon;
    while(m_slon < m_sp[3] && !m_sl[m_slon].retrieve_current(l, r, ck) )
    {
          m_slon++;
          s=m_slon;
    }

    if(m_slon >= m_sp[3]) return false; else return true;
}

