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

#include <mpi.h>
#include <string>
#include <vector>
#include <math.h>
#include <assert.h>

#include "onitcs.h"
#include "scene_manager.h"
#include "camera.h"
#include "transform.h"


Scene_Manager::Scene_Manager()
{
}


Scene_Manager::~Scene_Manager()
{
}


//-----------------------------------------------------------------------------
// things belongs to a scene manager (beyond interactor role)
//-----------------------------------------------------------------------------
void Scene_Manager::create_camera(bool stereo, float screen_w, float screen_h)
{
    thecamera = new Camera;
    float tvWidth2 =  screen_w / 2.0; //in meters
    float tvHeight2 = screen_h / 2.0; //in meters
    float tvBL[3] = {-tvWidth2, -tvHeight2, -2.0};
    float tvBR[3] = { tvWidth2, -tvHeight2, -2.0};
    float tvTL[3] = {-tvWidth2,  tvHeight2, -2.0};
    thecamera->set_screen_corners(tvBL, tvBR, tvTL);

    if(stereo)
    {
        float binocular2 = 0.03;
        float pleft[3] = {-binocular2, 0.0f, 0.0f};
        float pright[3] = { binocular2, 0.0f, 0.0f};
        thecamera->set_positions(pleft, pright);
        thecamera->print();
        m_event_processor->mapto_camera_position(pleft, pright);
    }
    else
    {
        float phead[3] = {0.0f, 0.0f, 0.0f};
        thecamera->set_positions(phead, phead);
        thecamera->print();
        m_event_processor->mapto_camera_position(phead, phead);
    }    
}



void Scene_Manager::create_world_transform(float objpos[3], float objrot[3])
{
    theworld = new Transform(objpos, objrot);
    theworld->print();
    m_event_processor->mapto_world_transform(objpos, objrot);
}



void Scene_Manager::setup_view(int eye)
{
    thecamera->setup_projection(eye);
    theworld->transform();
}

//-----------------------------------------------------------------------------
// acquire input from IPC or Net
//-----------------------------------------------------------------------------
void Scene_Manager::update(bool print)
{
    float info[32];

    m_event_processor->deliver_output(info);

    // printf("info[6-8](%.3f, %.3f, %.3f)\n",
    //        info[6], info[7], info[8]);
    // printf("info[9-11](%.3f, %.3f, %.3f)\n",
    //        info[9], info[10], info[11]);

    thecamera->lefteye_update(info);
    thecamera->righteye_update(info+3);
    if(print) thecamera->print();

    theworld->set_translation(info+6);
    theworld->set_rotation(info+9);
    if(print) theworld->print();
}




