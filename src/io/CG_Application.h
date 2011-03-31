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

#ifndef CG_APPLICATION_H
#define CG_APPLICATION_H


//An interface class for computer graphics application
//separate from the window management

class CG_Application{
public:
    CG_Application(){};
    virtual ~CG_Application(){};

    virtual void assert_application_mode() = 0;

    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void frame_update() = 0;

    virtual void processKeys(int) = 0;
    virtual void processNetEvents() = 0;
    virtual void processMouseMove(int m, int x, int y) = 0;


    virtual void save_screen(int vp[4]) = 0;
    virtual float compute_fps(float averagetime) = 0;

};



#endif
