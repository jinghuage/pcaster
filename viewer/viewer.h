/*****************************************************************************


    Copyright 2009,2010,2011 Jinghua Ge
    ------------------------------

    This file is part of Pcaster.

    Pcaster is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pcaster is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pcaster.  If not, see <http://www.gnu.org/licenses/>.


******************************************************************************/

#ifndef SINGLE_VIEWER_H
#define SINGLE_VIEWER_H

#include <string>
#include <vector>


#include "GL_Application.h"
#include "pcaster_options.h"
#include "renderscreen.h"


class Viewer : public GL_Application{
public:
    Viewer(int, int, int, int);

    virtual ~Viewer();
    
    //virtual function inherited from GL_Application
    void assert_application_mode();
    //void init();
    void draw();
    void frame_update();

    void processKeys(int);
    void processNetEvents();
    void processMouseMove(int m, int x, int y);

    void write_screen(GLenum, int, void*);


protected:

    int m_gw, m_gh;
    CRenderScreen* m_global_screen;


private:


};



#endif
