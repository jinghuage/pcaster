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


#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <string>
#include <vector>

#include <event_relayer.h>

class Camera;
class Transform;

class Scene_Manager{
public:

    Scene_Manager();
    virtual ~Scene_Manager();

    void register_event_processor(Event_Relayer* e)
    {
        m_event_processor = e;
    }


public:
    void setup_view(int eye = 0);

    void create_camera(bool stereo=false, float screen_w=1.0, float screen_h = 1.0);
    void create_world_transform(float t[3], float r[3]);

    void update(bool);

protected:

    Camera*    thecamera;
    Transform* theworld;

    Event_Relayer* m_event_processor;
};



#endif
