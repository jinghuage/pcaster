#ifndef VOL_RENDER_H
#define VOL_RENDER_H

#include "GL_Application.h"
#include "binaryTree.h"
#include "nodeDimension.h"
#include "nodeDatabrick.h"
#include "data_hierarchy.h"
#include "pcaster_options.h"
#include "ray_caster.h"
#include "frustumCull.h"
#include "log_timer.h"


class Vol_Render: public GL_Application{
public:
    Vol_Render(int, int, int, int, float);
    virtual ~Vol_Render();

    //virtual function inherited from GL_Application
    void assert_application_mode();
    void init();
    void draw();
    void frame_update();

    void processKeys(int);
    void processNetEvents();
    void processMouseMove(int m, int x, int y);

    void read_screen(GLenum mode, int* size, void** buf);

    //virtual functions for my sub-classes
    virtual void init_data_pool();



protected:
    //int m_wid, m_hei;
    int m_rank, m_runsize;
    //int m_fid;
    unsigned int* m_data_pool;

    CFrustumCull* m_fculler;
    Data_Hierarchy* m_kdGrid;
    NodeDatabrick* m_node;



    Ray_Caster* m_volrder;
    CTransferFunc* m_tFunc;
    CDataLoader* m_dataloader;
    Log_Timer* m_loger;



    //local function
    void init_datagrid(float gscale);
    void init_dataloader();
    void init_volrender();
    void init_transferFunc();
    void init_timerLog();

    void render_update(bool, bool, bool, bool);
    void print_options(bool, bool, bool, bool);

};


#endif
