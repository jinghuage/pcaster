
#include <assert.h>

#include <algorithm>

#include <glheaders.h>
#include <draw_routines.h>

#include "overlap_footprint.h"

const unsigned int OverLap_FootPrint::Layer_Procs = 32; 
const unsigned int ColorKey::keyLen = 2;

OverLap_FootPrint::OverLap_FootPrint(int myrank, int runsize, 
                                     int w, int h):
    m_rank(myrank),
    m_runsize(runsize),
    m_render_wid(w),
    m_render_hei(h),
    m_fp(0),
    m_ofScreen(0)
{
}



OverLap_FootPrint::~OverLap_FootPrint()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    clear();
}



//-----------------------------------------------------------------------------
//build: use offscreen rendering of footprint. 
//dataTree will handle the bbox drawing of its data nodes
//-----------------------------------------------------------------------------
void OverLap_FootPrint::build(int nodenum,
                              Data_Hierarchy* dataTree,
                              CFrustumCull* fculler)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    if(m_ofScreen==0)
    {
        m_ofScreen = new CRenderScreen(m_render_wid, m_render_hei, 
                                       CRenderScreen::SINGLE,
                                       GL_TEXTURE_RECTANGLE_ARB, 
                                       GL_RGBA, 
                                       GL_RGBA, 
                                       GL_UNSIGNED_BYTE,
                                       GL_NEAREST, GL_NEAREST,
                                       false, true);   

//         fprintf(stderr, "offscreen buffer (%d,%d) created for overlap drawing\n",
//                 m_render_wid, m_render_hei);
    }

    int nlayer = m_runsize / Layer_Procs;
    int extra = m_runsize % Layer_Procs;
    if(extra > 0) nlayer++;

    if(m_fp == 0) m_fp = new FootPrint(m_rank, -1);
    else m_fp->clear();

    float pbr[4];
    dataTree->compute_bbox(m_rank, fculler, pbr);
    m_fp->compute_screenport(pbr, m_render_wid, m_render_hei);

    for(int layer=0; layer<nlayer; layer++)
    {
        FootPrint layer_fp(m_rank, layer);
        layer_fp.set_screenport(m_fp->get_screenport());

        m_ofScreen->bind_render_buffer();

        //instead of clear buffer, draw a fullscreen black quad
        //this guanrantees that the read-back pixel will be 0 if not drawn.
        //simple clear sometimes won't do the job 
        //do it before set stencil, otherwise only inside-stencil pixel will be 
        //cleared
        glDisable(GL_CULL_FACE);
        glColor4f(0, 0, 0, 0);
        draw_fullscreen_quad();

        glEnable(GL_STENCIL_TEST);
        dataTree->set_stencil(m_rank);


        //make sure only one layer of faces get drawn
        //todo: there are more to it: when eye goes into the data, the backface
        //should be drawn. so maybe should always cull front faces?
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

        int start = layer*Layer_Procs;
        int end = start + nodenum - 1;
        dataTree->draw_node_bboxes(start, end, Layer_Procs);

        glDisable(GL_BLEND);
        
        glDisable(GL_STENCIL_TEST);
        m_ofScreen->unbind_render_buffer();
        
        m_ofScreen->read_back(layer_fp.get_screenport(), 
                              (unsigned char*)(layer_fp.get_map()));
        layer_fp.scan_map();

        if(layer==0) *m_fp = layer_fp;
        else *m_fp += layer_fp;

        //fp.print();

    }

    construct_overlaps(*m_fp);
}

//-----------------------------------------------------------------------------
//build: use onscreen rendering of footprint. 
//-----------------------------------------------------------------------------
void OverLap_FootPrint::build_onscreen(int nodenum,
                              Data_Hierarchy* dataTree,
                              CFrustumCull* fculler)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    int nlayer = m_runsize / Layer_Procs;
    int extra = m_runsize % Layer_Procs;
    if(extra > 0) nlayer++;

    if(m_fp == 0) m_fp = new FootPrint(m_rank, -1);
    else m_fp->clear();

    float pbr[4];
    dataTree->compute_bbox(m_rank, fculler, pbr);
    m_fp->compute_screenport(pbr, m_render_wid, m_render_hei);


    for(int layer=0; layer<nlayer; layer++)
    {
        FootPrint layer_fp(m_rank, layer);
        layer_fp.set_screenport(m_fp->get_screenport());

        glDisable(GL_CULL_FACE);
        glColor4f(0, 0, 0, 0);
        draw_fullscreen_quad();

        glEnable(GL_STENCIL_TEST);
        dataTree->set_stencil(m_rank);

        //make sure only one layer of faces get drawn
        //todo: there are more to it: when eye goes into the data, the backface
        //should be drawn. so maybe should always cull front faces?
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

        int start = layer*Layer_Procs;
        int end = start + nodenum - 1;
        dataTree->draw_node_bboxes(start, end, Layer_Procs);

        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);
        
        layer_fp.read_map();
        layer_fp.scan_map();

        if(layer==0) *m_fp = layer_fp;
        else *m_fp += layer_fp;

        //fp.print();

    }

    

    construct_overlaps(*m_fp);


    //confirm that all overlaps are retrieved. Note that correct node 
    //screenport is not applied here. 
    //validate_overlap_segments(render_wid, render_hei);
}


int OverLap_FootPrint::construct_overlaps(FootPrint& fp)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    clear();

    unsigned int scanline, left_extent, right_extent;
    ColorKey ck;

    fp.set_origin();
    int valid_pixels = 0;

    std::map<ColorKey, OverLap*> sort_ols;

    while(fp.retrieve_seg(scanline, left_extent, right_extent, ck) )
    {
//       std::cout << "get seg(" << scanline << ","
//                 << left_extent << ","
//                 << right_extent << ")\n";
             
//       std::cout << "insert by colorkey: " << ck << "\n";

        sort_ols.insert(std::pair<ColorKey, OverLap*>(ck, 0));

        if(sort_ols[ck] == 0){
            //std::cout << "colorkey is new\n";
            sort_ols[ck] = new OverLap;
        }
        sort_ols[ck]->insert_segment(scanline,
                                 left_extent,
                                 right_extent);
        valid_pixels += right_extent - left_extent + 1;
    }

#ifdef _DEBUG
    fprintf(stderr, "%d: build, ofp size = %ld\n", m_rank, sort_ols.size());
#endif

    std::map<ColorKey, OverLap*>::iterator mit;
    for(mit=sort_ols.begin(); mit!=sort_ols.end(); mit++)
    {
        OverLap* ol = mit->second;
        ol->get_proc_from_key(mit->first);
        ol->compute_root();

        m_overlaps.push_back(ol);
    }


// #ifdef _DEBUG      
//     std::ostringstream s;  
//     std::vector<OverLap*>::iterator oit;
//     for(oit=m_overlaps.begin(); oit!=m_overlaps.end(); oit++)
//     {
//         OverLap* ol = *oit;
//         s << *ol << "\n";        
//     }
//     fprintf(stderr, "%d: overlaps:\n%s\n", m_rank, s.str().c_str());
// #endif

    return valid_pixels;
}



//-----------------------------------------------------------------------------
//identify the overlap boundary by drawing segments
//-----------------------------------------------------------------------------
void OverLap_FootPrint::validate_overlap_segments(int wid, int hei)
{
//    std::map<ColorKey, OverLap*>::iterator mit;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, wid, 0, hei, -1, 1);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);

    std::vector<OverLap* >::iterator mit;
    for(mit=m_overlaps.begin(); mit!=m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;
        ol->define_GL_lines();
    }

    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void OverLap_FootPrint::draw_recv_pixels(int recvfrom, 
                                         unsigned int* pix_buffer,
                                         unsigned int rpos,
                                         int wid, int hei)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, wid, hei);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, wid, 0, hei, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int ox=0, oy=0;
    if(m_fp){
        int* sp = m_fp->get_screenport();
        ox = sp[0];
        oy = sp[1];
    }

    std::vector<OverLap* >::iterator mit;
    for(mit=m_my_ol.begin(); mit!=m_my_ol.end(); mit++)
    {
        OverLap* ol = *mit;  
        const std::vector<int>& procs = ol->get_processes();
        int npix = ol->get_numPixels();
        if( recvfrom != m_rank && 
            std::find(procs.begin(), procs.end(), recvfrom) != procs.end()) //Found
        {
//             fprintf(stderr, "%d: draw overlap pixels recv from %d, %d, %d\n",
//                     m_rank, recvfrom, npix, rpos);
            unsigned short* pos_buffer = new unsigned short[2 * npix];
            ol->pack_pixel_position(pos_buffer, ox, oy);

            glVertexPointer(2, GL_SHORT, 0, pos_buffer);
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, 
                           pix_buffer + rpos);
            rpos += npix;

            glDrawArrays(GL_POINTS, 0, npix);

            delete[] pos_buffer;           
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}


//-----------------------------------------------------------------------------
//identify the overlap boundary by drawing pixels
//-----------------------------------------------------------------------------
void OverLap_FootPrint::draw_overlap_pixels(unsigned int* pix_buffer, 
                                            int wid, int hei,
                                            int group)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, wid, hei);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, wid, 0, hei, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int ox=0, oy=0;
    if(m_fp){
        int* sp = m_fp->get_screenport();
        ox = sp[0];
        oy = sp[1];
    }

    //std::map<ColorKey, OverLap*>::iterator mit;
    std::vector<OverLap* >::iterator mit;
    for(mit=m_overlaps.begin(); mit!=m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;
        int root = ol->get_root();
        if(group == 0) //draw only single overlaps
        {
            if(!ol->is_singleton()) continue;
        }
        else if(group == 1) //draw only composite overlaps
        {
            if(root != m_rank || ol->is_singleton()) continue;
        }
        else if(group == 2) //draw only other overlaps
        {
            if(root == m_rank) continue;
        }

        unsigned int size = ol->get_numPixels();
        unsigned short* pos_buffer = new unsigned short[2 * size];


        ol->pack_pixel_position(pos_buffer, ox, oy);

        glVertexPointer(2, GL_SHORT, 0, pos_buffer);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, 
                       pix_buffer + ol->get_databuf_offset());


        glDrawArrays(GL_POINTS, 0, size);

        delete[] pos_buffer;
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    //checkError
    if(printOpenGLError())
    {
        std::cout << __FILE__ << ":" <<  __func__ <<"()\n";
        exit(1);
    } 
    
}



//-----------------------------------------------------------------------------
//distinguish the overlaps categories
//-----------------------------------------------------------------------------
void OverLap_FootPrint::group_overlaps(unsigned int& single_count,
                                       unsigned int& my_count,
                                       unsigned int& other_count)
{

    single_count = my_count = other_count = 0;

    //std::map<ColorKey, OverLap*>::iterator mit;
    std::vector<OverLap* >::iterator mit;
    for(mit=m_overlaps.begin(); mit!=m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;

        int root = ol->get_root();
        int cnt = ol->get_numPixels();

        if(ol->is_singleton()){
          m_single_ol.push_back(ol);
          single_count += cnt;
        }
        else if(m_rank == root){
          m_my_ol.push_back(ol);
          my_count += cnt;
        }
        else{
          m_other_ol.push_back(ol);
          other_count += cnt;
        }
    }
}



//-----------------------------------------------------------------------------
//when the footprint has pixel data, pack it overlap-wise into a logic buffer
//-----------------------------------------------------------------------------
void OverLap_FootPrint::pack_overlap_pixels(unsigned int* logicbuffer)
{
#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    std::vector<OverLap* >::iterator mit;
    int offset = 0;
    int* sp = m_fp->get_screenport();
    const unsigned int* pixels = m_fp->get_map();

    for(mit=m_overlaps.begin(); mit!=m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;
        offset = ol->get_databuf_offset(); 
        
        ol->pack_pixel_data(pixels,
                            sp[2], sp[3],
                            logicbuffer + offset);

// #ifdef _DEBUG
//         fprintf(stderr, "%d: pack overlap %d pixels to offset %d\n", 
//                 m_rank, ol->get_numPixels(), offset);
// #endif
    }
}



//-----------------------------------------------------------------------------
//set the databuf_offset for each overlap
//used by each pcaster renderer for pixel data storage
//reason: overlaps are sorted by colorkey, but their pixel data offset
//are sorted by their groups (single, my, other)
//furthermore, in send group, data offset are sorted again by procs
//-----------------------------------------------------------------------------
void OverLap_FootPrint::address_single_overlaps(unsigned int kbuf_offset)
{
  unsigned int pos = 0;

  std::vector<OverLap*>::iterator it;

  for(it = m_single_ol.begin(); it<m_single_ol.end(); it++)
    {
      OverLap* ol = *it;
      ol->set_databuf_offset(kbuf_offset + pos);
      pos += ol->get_numPixels();
    }
}

void OverLap_FootPrint::address_my_overlaps(unsigned int cbuf_offset)
{
  unsigned int pos = 0;

  std::vector<OverLap*>::iterator it;

  for(it = m_my_ol.begin(); it<m_my_ol.end(); it++)
    {
      OverLap* ol = *it;
      ol->set_databuf_offset(cbuf_offset + pos);
      pos += ol->get_numPixels();
    }
}


//-----------------------------------------------------------------------------
//set the databuf_offset for send overlaps
//databuf_offset will be sorted by procs, ready to send out
//-----------------------------------------------------------------------------
void OverLap_FootPrint::address_other_overlaps(unsigned int sbuf_offset,
                                               unsigned int* proc_offsets)
{
    unsigned int* spos = new unsigned int[m_runsize];
    memcpy(spos, proc_offsets, m_runsize * sizeof(unsigned int));

    std::vector<OverLap*>::iterator it;
    for(it = m_other_ol.begin(); it<m_other_ol.end(); it++)
    {
        OverLap* ol = *it;
        int root = ol->get_root();
        ol->set_databuf_offset(sbuf_offset + spos[root]);
        spos[root] += ol->get_numPixels();
    }

    delete[] spos;

}



//-----------------------------------------------------------------------------
//set the databuf_offset for each overlap. 
//used by pviewer to recv data from pcaster
//reason: one pcaster renderer might send multiple overlaps to viewer
//only have offset for each renderer is not enough. 
//Must distinguish for each overlap, so later overlap can draw their own pixels
//-----------------------------------------------------------------------------
void OverLap_FootPrint::address_overlaps(unsigned int* offsets, int nc)
{
    unsigned int* rpos = new unsigned int[nc];
    memcpy(rpos, offsets, nc * sizeof(unsigned int));

    std::vector<OverLap*>::iterator mit;
    for(mit = m_overlaps.begin(); mit != m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;
        int root = ol->get_root(); 
        int numpixels = ol->get_numPixels();
        ol->set_databuf_offset(rpos[root]);
        rpos[root] += numpixels;
    }
    
    delete[] rpos;    
}



//-----------------------------------------------------------------------------
//fill in count arrays. used by pcaster for pixel exchange
//-----------------------------------------------------------------------------
void OverLap_FootPrint::countfor_my_overlaps(unsigned int* count_array)
{

    memset(count_array, 0, m_runsize*sizeof(unsigned int));

    std::vector<OverLap*>::iterator it;

    for(it = m_my_ol.begin(); it<m_my_ol.end(); it++)
    {
        OverLap* ol = *it;

        const std::vector<int>& procs = ol->get_processes();
        std::vector<int>::const_iterator iit;

        for(iit = procs.begin(); iit != procs.end(); iit++)
        {
            int pid = *iit;
            //assert(pid >= 0 && pid < runsize);
            //others owe me
            if(pid != m_rank) count_array[pid] += ol->get_numPixels();
        }
    }
}


void OverLap_FootPrint::countfor_other_overlaps(unsigned int* count_array)
{
    memset(count_array, 0, m_runsize*sizeof(unsigned int));

    std::vector<OverLap*>::iterator it;

    for(it = m_other_ol.begin(); it<m_other_ol.end(); it++)
    {
        OverLap* ol = *it;
        int root = ol->get_root();

        //I owe root
        count_array[root] += ol->get_numPixels();
    }
}


//-----------------------------------------------------------------------------
//fill in count arrays. used by pviewer for recv from pcaster
//-----------------------------------------------------------------------------
void OverLap_FootPrint::count_overlaps(unsigned int* count_array, int nc)
{
    memset(count_array, 0, nc*sizeof(unsigned int));

    std::vector<OverLap*>::iterator it;

    for(it = m_overlaps.begin(); it<m_overlaps.end(); it++)
    {
        OverLap* ol = *it;
        int root = ol->get_root();

        count_array[root] += ol->get_numPixels();
    }
}



//-----------------------------------------------------------------------------
//save overlap info into an int vector. Want my_ol and single_ol
//-----------------------------------------------------------------------------
int OverLap_FootPrint::save_overlap_info(std::vector<int>& olbuffer)
{

#ifdef _DEBUG7
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif


    int* node_sp = m_fp->get_screenport();

    std::vector<OverLap*>::const_iterator mit;
    int count = 0;

    for(mit = m_overlaps.begin(); mit != m_overlaps.end(); mit++)
    {
        OverLap* ol = *mit;
        int root = ol->get_root();

        if(root == m_rank)
        {
            ol->serialize_structure(node_sp[0], node_sp[1], olbuffer);
            count++;
        }            
    }

    return count;
}




//-----------------------------------------------------------------------------
//retrieve overlaps from the olbuffer
//-----------------------------------------------------------------------------
void OverLap_FootPrint::retrieve_overlaps(std::vector<int>& olbuffer)
{

#ifdef _DEBUG
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
#endif

    clear();


    int count = 0;
    int size = olbuffer.size();

    while(count < size)
    {
        OverLap* ol = new OverLap;
        int r = ol->deserialize_structure(&olbuffer[count]);
        count += r;

        m_overlaps.push_back(ol);
    }
        
    olbuffer.clear();

#ifdef _DEBUG
    fprintf(stderr, "%d: retrieve total overlap num: %ld\n", 
            m_rank, m_overlaps.size() );
#endif
}
