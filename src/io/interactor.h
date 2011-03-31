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


#ifndef INTERACTOR_H
#define INTERACTOR_H


//An abstraction interface class for users to interact with  
//Renderer (subclasses implements GL_Application)

// input-interact category:  accompany renderer: update scene from tracker
// output-interact category:  accompany renderer: stream images to viewer

// input-interact category:   accompany viewer:   recv image 
// output-interact category:  accompany viewer:   event-relayer



class Interactor{
public:
    virtual ~Interactor(){};

    enum Interact_Mode{DDB=0, MOUSE_KEY=1, IPC_SHARE_MEMORY=2, NET_TCP=4, 
               NET_UDP=8};

    enum Sync_Mode{SYNC_NONE=0, SYNC_MPI=1};
    enum Collect_Mode{COLLECT_NONE=0, COLLECT_MPI=1};
    enum Distribute_Mode(DIST_NONE=0, DIST_BCAST=1, DIST_SINGLE=2};
};




class Input_Interactor: public Interactor{
public:

    virtual ~Input_Interactor(){};


    virtual void acquire_input() = 0;

    virtual void sync_input_to_peer(int comm) = 0;

    virtual void update_interaction() = 0;

};


class Output_Interactor: public Interactor{
public:

    virtual ~Output_Interactor(){};

    virtual void pull_output() = 0;

    virtual void collect_output_from_peer(int comm) = 0;

    virtual void commit_interaction(int id) = 0;

};



#endif
