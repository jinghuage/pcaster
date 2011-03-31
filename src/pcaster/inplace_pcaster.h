#ifndef INPLACE_PCASTER_H
#define INPLACE_PCASTER_H

#include "pcaster.h"

#include "image_exchanger.h"
#include "tiled_display.h"
#include "nodeRectile.h"
#include "imagefragment_tile.h"

#ifdef SAGE
// headers for SAGE
#include <sail.h>
#include <misc.h>
#endif


class Inplace_Pcaster : public Pcaster{
public:
    Inplace_Pcaster(int, int, int, int, float);
    ~Inplace_Pcaster();

    //virutal funcs inherited from CG_Application
    //void init();
    void draw();
    //void processKeys(int);
    //void processNetEvents();
    //void processMouseMove(int m, int x, int y);
    //void frame_update(float* navi, int num);
    void assert_application_mode();

    void read_screen(GLenum mode, int* size, void** buf);

    //directly draw tile to framebuffer
    void draw_tile();


protected:
    //virtual funcs inherited from vol render
    void init_data_pool();


protected:
    void init_screen();
    void init_view_settings();

#ifdef SAGE
    void init_sage_stuff(std::vector<float>&,
			 std::vector<float>&,
			 std::vector<int>&);
    //void sendSagePixels();
#endif

protected:
    //if also serve as an inplace viewer, compose a view
    Tiled_Display* m_screen;
    NodeRectile*   m_tnode;



private:
    Image_Exchanger* m_img_cger;
    ImageFragment_Tile*  m_ift;
    int m_nviewer;

#ifdef SAGE
    GLubyte* rgbBuffer;
    sail sageInf;
#endif


protected:

    void sync_structure();
    void pack_screen_fragments();
    void dispatch_fragments();


};


#endif
