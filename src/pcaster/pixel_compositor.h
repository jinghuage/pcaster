#ifndef PIXEL_COMPOSITOR_H
#define PIXEL_COMPOSITOR_H

#include <string.h>
#include <iostream>
#include <vector>
#include <map>


#include "overlap_footprint.h"


class Log_Timer;

//extract overlaps from a footprint and save as a std::map data structure
class Pixel_Compositor
{
public:

    enum compMode{COMP_GPU=0, COMP_CPU};

    Pixel_Compositor(int rank, int runsize,
                     int mode, int wid, int hei, 
                     std::string& shaderpath);
    ~Pixel_Compositor();

    //friend ostringstream& operator<<(ostringstream& s, Pixel_Compositor& pc);



    void composite(unsigned int* databuf,
                   unsigned int pre_compsize,
                   unsigned int compsize, 
                   std::vector<int>& info,
                   Log_Timer* timer = 0);
 

private:
    int m_rank;
    int m_runsize;

    int m_mode;

    int m_wid;
    int m_hei;


//for gpu composite
    tex_unit*      m_compTexture;
    CRenderScreen* m_compScreen;
    shader_object* m_compShader;


//for cpu composite
    unsigned int* m_compBuffer;



    void init_shader(std::string& shaderpath);
    void init_renderScreen();
    void init_buffer();

    void gpuComposite(unsigned int comp_hei, 
                      std::vector<int>& info_array);

    void cpuComposite(unsigned int* src,
                      unsigned int compsize, 
                      std::vector<int>& info_array);


    //debug or test only
    void fake_composite_data(unsigned int* databuf,
                             std::vector<int>& ofp_info);
};


#endif
