#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

#include <assert.h>

#include <glheaders.h>
#include <draw_routines.h>
#include <buffer_object.h>
#include <pmisc.h>

#include "log_timer.h"
#include "renderscreen.h"
#include "pixel_compositor.h"

#define MAX_OVERLAP_INFO_LEN 128


Pixel_Compositor::Pixel_Compositor(int rank, int runsize,
                                   int mode, int wid, int hei,
                                   std::string& shaderpath):
    m_rank(rank),
    m_runsize(runsize),
    m_mode(mode),
    m_wid(wid),
    m_hei(hei),
    m_compTexture(0),
    m_compScreen(0),
    m_compShader(0),
    m_compBuffer(0)
{

    if(m_mode == COMP_GPU){
        init_shader(shaderpath);
        init_renderScreen();
    }
    else if(m_mode == COMP_CPU){
        init_buffer();
    }
}


Pixel_Compositor::~Pixel_Compositor()
{
    if(m_compBuffer) delete[] m_compBuffer;
    if(m_compScreen) delete m_compScreen;
    if(m_compShader) delete m_compShader;
}


void Pixel_Compositor::init_shader(std::string& path)
{ 
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    std::string vfile = path + "composite.vert";
    std::string ffile = path + "ofp_composite.frag";

    m_compShader = new shader_object;
    m_compShader->init_from_file(vfile.c_str(), ffile.c_str());

}

void Pixel_Compositor::init_renderScreen()
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_compTexture = new tex_unit;
    m_compTexture->setformat(GL_TEXTURE_RECTANGLE_ARB,
                             GL_RGBA,
                             GL_RGBA,
                             GL_UNSIGNED_BYTE);
    m_compTexture->setfilter(GL_NEAREST, GL_NEAREST);

    m_compTexture->create(m_wid, m_hei*2, 0, (char*)0);



    //offscreen composite 
    m_compScreen = new CRenderScreen(m_wid, m_hei, CRenderScreen::SINGLE,
                                     GL_TEXTURE_RECTANGLE_ARB, 
                                     GL_RGBA, 
                                     GL_RGBA, 
                                     GL_UNSIGNED_BYTE,
                                     GL_NEAREST, GL_NEAREST);   
}



void Pixel_Compositor::init_buffer()
{
    m_compBuffer = new unsigned int[m_wid * m_hei];
}


//-----------------------------------------------------------------------------
//fake data for indepedent compositing test
//data from the same overlap will have the same color
//so result can be easy to see.
//-----------------------------------------------------------------------------
void Pixel_Compositor::fake_composite_data(unsigned int* databuf,
                          std::vector<int>& ofp_info)
{
    std::vector<int>::iterator iit = ofp_info.begin();
    int conum = *iit++;
    if(conum == 0) return;


    unsigned int preset_color[10]; //RGBA, in integer buffer byte order is ABGR
    preset_color[0] = 0xefff0000;
    preset_color[1] = 0xef00ff00;
    preset_color[2] = 0xef0000ff;
    preset_color[3] = 0xefefef00;
    preset_color[4] = 0xefefefef;
    preset_color[5] = 0xef00efef;
    preset_color[6] = 0xefef00ef;
    preset_color[7] = 0xefef0000;
    preset_color[8] = 0xef00ef00;
    preset_color[9] = 0xef0000ef;


    int *pixnum = new int[conum];
    for(int i=0; i<conum; i++) pixnum[i] = *iit++;
    for(int i=0; i<conum; i++)
    {
        int nproc = *iit++;
        int npix = pixnum[i];
        for(int p=0; p<nproc; p++)
        {
            int offset = *iit++;
            std::fill(databuf + offset, databuf + offset + npix, preset_color[i]);
        }
    }

}

static int bend1dto2d(int size_1d, int w_2d)
{
    int m = size_1d % w_2d;
    int d = size_1d / w_2d;

    int h = m>0 ? d+1 : d;

    return h;
}

void Pixel_Compositor::composite(unsigned int* databuf,
                                 unsigned int pre_compsize,
                                 unsigned int compsize, 
                                 std::vector<int>& info,
                                 Log_Timer* timer)
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    if(timer) timer->start_timer();

    unsigned int wsize = m_wid * m_hei;
    if(compsize > wsize)
    {
        fprintf(stderr, "%s:%s(): composite pixel num %d exceed limit %d\n",
                __FILE__, __func__, compsize, wsize);
        exit(1);
    }
    if(pre_compsize > wsize*2)
    {
        fprintf(stderr, "%s:%s(): total pixel num %d exceed limit %d\n",
                __FILE__, __func__, pre_compsize, wsize*2);
        exit(1);
    }

    if(m_mode == COMP_GPU)
    {
        unsigned int fit_hei = bend1dto2d(pre_compsize, m_wid);
        m_compTexture->update_subimage(0, 0,
                                 m_wid, fit_hei,
                                 databuf);

//         fprintf(stderr, "%d: update comp texture(%d, %d)\n", 
//                 m_rank, m_wid, fit_hei); 
//         if(printOpenGLError()) exit(1);

        unsigned int comp_hei = bend1dto2d(compsize, m_wid);
        gpuComposite(comp_hei, info);

//         fprintf(stderr, "%d: comp hei=%d\n", m_rank, comp_hei);
//         if(printOpenGLError()) exit(1);


        int sp[4] = {0, 0, m_wid, comp_hei};
        m_compScreen->read_back(sp, (unsigned char*)databuf);
        //if(printOpenGLError()) exit(1);
    }
    else if(m_mode == COMP_CPU)
    {
        cpuComposite(databuf, compsize, info);
        memcpy(databuf, m_compBuffer, compsize * sizeof(unsigned int));
    }


    if(timer){ timer->stop_timer(); timer->log(Log_Timer::LOG_Comp); }
}


//-----------------------------------------------------------------------------
//data are already there, just need to tell encough information to the 
//composite shader so that correct sets of pixels can be composited 
//an example info_array: 
//ofp info: 3, 22503, 1371, 527, 
//          5, 447572, 398243, 423171, 473344, 497745, 
//          6, 471446, 470075, 420746, 445674, 495847, 520248, 
//          6, 472817, 422644, 422117, 447045, 497218, 521619, 
//find the bug!!! the offsets should be relative to the cbuf_offset
//not relative to the origin of data_pbo
//here is a correct example: 
//localrecv size: 6185
//composite size: 30925
//ofp info: 1, 6185, 5, 24740, 18555, 6185, 0, 12370, 
//-----------------------------------------------------------------------------

void Pixel_Compositor::gpuComposite(unsigned int comp_hei, 
                                    std::vector<int>& info_array)
{
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    glActiveTexture(GL_TEXTURE0);
    m_compTexture->bind();

    //before the strlen is set of 16 -- which is not enough  -- stack overflow
    char uniform_name[32];

    m_compShader->use();
    glUniform1i(m_compShader->getUniformLocation("tex"), 0);
    glUniform1i(m_compShader->getUniformLocation("texwid"), m_wid);

    int infolen = info_array.size();
    if(infolen > MAX_OVERLAP_INFO_LEN)
    {
        fprintf(stderr, "%d: comp info len = %d, longer then %d, \
can't use GPU composite mode, try CPU composite and restart!!!\n",
                m_rank, infolen, MAX_OVERLAP_INFO_LEN);
        exit(1);
    }


    for(int uid=0; uid < infolen; uid++)
    {
        sprintf(uniform_name, "overlap_info[%d]", uid);
        glUniform1i(m_compShader->getUniformLocation(uniform_name), 
                    info_array[uid]);
//         fprintf(stderr, "%d: set uniform %s value %d\n", 
//                 m_rank, uniform_name, info_array[uid]);
    }


    m_compScreen->bind_render_buffer();

    push_all_matrix();
    //toOrtho(0);//frustum -1 to +1
    toOrtho(0, 0, m_wid, 0, m_hei); //frustum l to r, b to t
    glViewport(0, 0, m_wid, m_hei);

    draw_quad(0, m_wid, 0, comp_hei, 1.0, 1.0);

    pop_all_matrix();
    glUseProgram(0);   
 
    m_compScreen->unbind_render_buffer();

    m_compTexture->unbind();

}




//the over operation
static void over(unsigned int* s0, unsigned int* s1, unsigned int* d)
{
    unsigned char* cs0 = (unsigned char*)s0;
    unsigned char* cs1 = (unsigned char*)s1;
    unsigned char* cd = (unsigned char*)d;


    //todo: which byte is the alpha value, 0 or 3?
    float alpha = cs0[3]/255.0;
    for(int i=0; i<4; i++)
    {
        //front to back compositing
        cd[i] = (unsigned char)clip(cs0[i] + (1.0-alpha) * cs1[i], 
                                    0., 
                                    255.);
    }    
}


static void over_op(unsigned int* src0, 
                    unsigned int* src1,
                    int size,
                    unsigned int* dst)
{
    for(int i=0; i<size; i++)
    {
        over(src0+i, src1+i, dst + i);
    }
}



//-----------------------------------------------------------------------------
//cpu composite, do over operation
//-----------------------------------------------------------------------------
void Pixel_Compositor::cpuComposite(unsigned int* src,
                                    unsigned int compsize, 
                                    std::vector<int>& info_array)
{	
    int infoid = 0;
    

    int nol = info_array.at(infoid);
    infoid++;

    int *ol_pixnum = new int[nol];

    for(int i=0; i<nol; i++) ol_pixnum[i] = info_array[infoid+i];

    infoid += nol;

    unsigned int* dst = m_compBuffer;
    int offset = 0;
    for(int i=0; i<nol; i++)
    {
        int procnum = info_array[infoid];
        infoid++;

//        fprintf(stderr, "composite overlap %d, procnum %d\n", 
//                i, procnum);


        for(int j=0; j<procnum; j++)
        {
            offset = info_array[infoid + j];

            if(j==0)
                memcpy(dst, src+offset, ol_pixnum[i]*sizeof(unsigned int));
            else
                over_op(dst, src+offset, ol_pixnum[i], dst);
        }

//         //for debug, copy the original, comment out when composite
//         memcpy(dst, src+offset, ol_pixnum[i]*sizeof(unsigned int));
//         offset += ol_pixnum[i];

        //prepare for next overlap
        dst += ol_pixnum[i];
        infoid += procnum;
    }
    
    delete[] ol_pixnum;
}

