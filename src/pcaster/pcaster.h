#ifndef PCASTER_H
#define PCASTER_H

#include "volrender.h"

#include "colorkey.h"
#include "footprint.h"
#include "overlap_footprint.h"
#include "pixel_exchanger.h"
#include "pixel_compositor.h"



class Pcaster : public Vol_Render{
public:
    Pcaster(int, int, int, int, float);
    ~Pcaster();

    //virutal funcs inherited from CG_Application
    //void init();
    void draw();
    void processKeys(int);
    //void processNetEvents();
    //void processMouseMove(int m, int x, int y);
    void frame_update();
    void assert_application_mode();

    void read_screen(GLenum mode, int* size, void** buf);

    //virutal funcs inherited from vol render
    void init_data_pool();


protected:

    void init_parallel_settings();


protected:
    OverLap_FootPrint* m_ofp;
    Pixel_Exchanger* m_exchanger;
    Pixel_Compositor* m_compositor;




protected:
    void construct_ofp();
    void render();
    void pack_pixels();
    void exchange_pixels();
    void composite();


protected:

    int m_demo_mode;
    void process_demo_mode(int);


};


#endif
