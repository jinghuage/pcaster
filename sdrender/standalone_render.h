#ifndef STANDALONE_RENDER_H
#define STANDALONE_RENDER_H

#include "volrender.h"


class Standalone_Render: public Vol_Render{
public:
    Standalone_Render(int, int, int, int, float);
    ~Standalone_Render();

    void init();
    void draw();
    void processKeys(int);
    void assert_application_mode();

protected:


private:

    //draw a big dataset by multi-pass rendering sub-blocks
    bool m_multipass; 
    bool m_sortnodes;
    void process_multipass_mode(int);
    void process_debug_mode(int key);
};


#endif
