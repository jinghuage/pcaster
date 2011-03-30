#ifndef COMPOSE_VIEWER_H
#define COMPOSE_VIEWER_H

#include <string>
#include <vector>



#include "imagefragment_tile.h"
#include "viewer.h"


//A compose viewer don't get pixels as of an image directly for the view
//it get image fragments and need to compose a correct view

class Compose_Viewer : public Viewer{
public:
    Compose_Viewer(int, int, int, int, int, int, int);
    virtual ~Compose_Viewer();

    //virtual function inherited from CG_Application
    virtual void draw();
    virtual void processKeys(int);
    virtual void processNetEvents();
    virtual void processMouseMove(int m, int x, int y);
    virtual void frame_update(float* navi, int num);
    virtual void assert_application_mode();


    //for inplace viewer, without interactor
    void retrieve_structure(std::vector<int> ainfobuf);
    void resolve_image(unsigned int* databuf);


protected:
    //virtual function inherited from Viewer
    virtual void init_interactor();
    virtual void stream_pixels();
    virtual void draw_global_screen();


    //new virtual functions for my sub-classes
    virtual void init_ift();
    virtual void sync_render_structure();


protected:
    ImageFragment_Tile* m_ift;


};



#endif
