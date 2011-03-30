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


//have to put mpi.h first
//sth. like glew.h....
#include <mpi.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <cstdlib>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>


#include "viewer.h"
#include "pcaster_options.h"
#include "scene_manager.h"
#include "event_relayer.h"
#include "image_streamer.h"
#include "socket_data_mover.h"

//-----------------------------------------------------------------------------
//general global types, definitions, and variables
//-----------------------------------------------------------------------------

bool screen_shot = false;
bool anim = false;
bool print = false;

Event_Relayer* tangible = 0;
Scene_Manager* scmanager = 0;
Image_Streamer* streamer = 0;
Viewer* viewer = 0;
pcaster_options* pops = 0;


int window_noframe = 1;
int window_fullscreen = 0;
int window_w = 512;
int window_h = 512;
int window_x = 800;
int window_y = 0;
float screen_w = 1.284;
float screen_h = 0.722;

float objpos[3] = {0., 0., -1.5};
float objrot[3] = {0., 0., 0.};
bool stereo = false;

int myrank = 0;
int runsize = 1;



//-------------------------------------------------------------------------------
//setup transform before display and wrap up after display
//-------------------------------------------------------------------------------

void reshape()
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -1.0, 1.0, -1.0, 1.0, 0., 100.);
}


void display()
{
    reshape();

    glViewport(0, 0, window_w, window_h);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    viewer->draw();

    SDL_GL_SwapBuffers();

    if(screen_shot) viewer->save_screen(0);
}




//-----------------------------------------------------------------------------
// frame update
//-----------------------------------------------------------------------------

void animate()
{

}


void idle()
{
    if(anim) animate();       
    
    //if(connect_gui) renderer->processNetEvents();
    viewer->frame_update();

    tangible->acquire_input();
    tangible->deliver_output(0);

    streamer->acquire_input();

    float fps = viewer->compute_fps(1.0);

    if(fps > 0.)
    {
        char title[256];

        sprintf(title, "Viewer: fps=%.3f", fps);     
        SDL_WM_SetCaption(title, "SDL: "); 
    }

//    usleep(1);
}




//-----------------------------------------------------------------------------
//process keys
//-----------------------------------------------------------------------------

void processKeys(int key) {

    //static float zoom_step = 0.1;

    switch(key)
    {
    case SDLK_ESCAPE:  
        exit(0);
    case SDLK_p: print = !print; break;
    case SDLK_a: anim = !anim; break;
    case SDLK_n:
        tangible->mapto_world_translate(0, 0, -0.01);
        break;
    case SDLK_m:
        tangible->mapto_world_translate(0, 0, 0.01);
        break;
    default:
        break;
    }

}


//-----------------------------------------------------------------------------
//process mouse motion
//-----------------------------------------------------------------------------

void processMousePassiveMotion(int m, int x, int y) 
{
    static int px = -1, py = -1;

    if(m == 1)
    {
        float rx = (x-px) * 180.0 / window_w;
        float ry = (y-py) * 180.0 / window_h;

        tangible->mapto_world_rotate(ry, rx, 0);
    }
    else if(m == 3)
    {
        float tz = (y-py) * 180.0 / window_h;

        tangible->mapto_world_translate(0, 0, tz);
    }


    px = x;
    py = y;

}


//-----------------------------------------------------------------------------
//process mouse buttons: up and down
//-----------------------------------------------------------------------------

void processMouseButtonUp(int button, int& m) 
{
    m = 0;
}


void processMouseButtonDown(int button, int& m) 
{
    m = button;
}


//-----------------------------------------------------------------------------
//SDL event handling
//-----------------------------------------------------------------------------
static int x = 0;
static int y = 0;
static int move = 0;

static int do_event(SDL_Event *e)
{
    switch (e->type)
    {
    case SDL_MOUSEMOTION:
        x = e->motion.x;
        y = e->motion.y;
        processMousePassiveMotion(move, x, y);
        break;

    case SDL_MOUSEBUTTONDOWN:
        processMouseButtonDown(e->button.button, move);
        break;

    case SDL_MOUSEBUTTONUP:
        processMouseButtonUp(e->button.button, move);
        break;

    case SDL_KEYDOWN:
        processKeys(e->key.keysym.sym);
        viewer->processKeys(e->key.keysym.sym);
        break;

    case SDL_QUIT:
        return 0;
    }

    return 1;
}

//-----------------------------------------------------------------------------
// set environment
//-----------------------------------------------------------------------------

void setenv_window()
{
    char buf[32];


#ifdef _WIN32
    sprintf(buf, "SDL_VIDEO_WINDOW_POS=%d,%d", 
            window_x, 
            window_y);
    putenv(buf);
#else //if defined(__linux__)
    sprintf(buf, "%d,%d", 
            window_x, 
            window_y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);

    //setenv("DISPLAY", ":0.0", 1);
#endif
}


//-----------------------------------------------------------------------------
//MAIN ENTRY
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    setenv_window();



    const char* cfgfilename = "../configs/pcaster_options_viewer.cfg";
    pops = new pcaster_options(0, 0, cfgfilename);


    // window_w = pcaster_options::render_wid;
    // window_h = pcaster_options::render_hei;
    window_noframe = pcaster_options::window_noframe;
    window_fullscreen = pcaster_options::window_fullscreen;


    tangible = new Event_Relayer(myrank, runsize,
                                 GL_IO::MOUSE_KEY | GL_IO::TUIKIT,
                                 GL_IO::NET_TCP, 1);
    tangible->mapto_world_transform(objpos, objrot); 

#ifdef _STREAMING
//    std::cout << pcaster_options::viewer << "\n";

    tangible->init_data_mover( Socket_Data_Mover::NET_SERVER,
                               pcaster_options::viewer, 9332);
#endif

    streamer = new Image_Streamer(myrank, runsize, 
                                  GL_IO::NET_TCP,
                                  GL_IO::DDB, 1,
                                  GL_RGB, 
                                  pcaster_options::render_wid,
                                  pcaster_options::render_hei);

#ifdef _STREAMING
    streamer->init_data_mover( Socket_Data_Mover::NET_SERVER,
                               pcaster_options::viewer, 9333 );
#endif



    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        if (SDL_SetVideoMode(window_w, 
                             window_h, 
                             0,
                             window_noframe    * SDL_NOFRAME    |
                             window_fullscreen * SDL_FULLSCREEN |
                             SDL_OPENGL))
        {
           
            viewer = new Viewer(window_w, window_h,
                                pcaster_options::render_wid,
                                pcaster_options::render_hei);
            viewer->init();
            viewer->assert_application_mode();
            
            streamer->register_image_receiver(viewer);

            int run = 1;
            while (run)
            {
                SDL_Event e;


                //if(SDL_WaitEvent(&e)) run &= do_event(&e);
                while (SDL_PollEvent(&e)) run &= do_event(&e);

                idle();

                display();

                //sleep(10);
                //run = 0;
            }

        }
        else fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    
    return 0;
}

