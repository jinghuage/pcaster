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
//#include <mpi.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <assert.h>
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


#include "standalone_render.h"
#include "pcaster_options.h"
#include "scene_manager.h"


//-----------------------------------------------------------------------------
//general global types, definitions, and variables
//-----------------------------------------------------------------------------

bool screen_shot = false;
bool anim = false;
bool print = false;

Event_Relayer* tracker = 0;
Scene_Manager* scmanager = 0;
Standalone_Render* renderer = 0;
pcaster_options* pops = 0;


int window_noframe = 1;
int window_fullscreen = 0;
int window_w = 1920;
int window_h = 1080;
int window_x = 0;
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

void draw_scene(int eye, int w, int h)
{
    scmanager->setup_view(eye);

    glViewport(0, 0, w, h);
    renderer->draw();
}


void display()
{
    glViewport(0, 0, window_w, window_h);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if(stereo)
    {
        draw_scene(0, window_w/2, window_h);
        draw_scene(1, window_w/2, window_h);
        glViewport(0, 0, window_w, window_h);
    }
    else
    {
        draw_scene(0, window_w, window_h);
    }

    SDL_GL_SwapBuffers();

    if(screen_shot) renderer->save_screen(0);
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
    renderer->frame_update();

    tracker->acquire_input();
    scmanager->update(print);

    float fps = renderer->compute_fps(1.0);

    if(fps > 0.)
    {
        char title[256];

        sprintf(title, "StandAloneRenderer: fps=%.3f", fps);     
        SDL_WM_SetCaption(title, "SDL render"); 
    }

//    usleep(1);
}




//-----------------------------------------------------------------------------
//process keys
//-----------------------------------------------------------------------------

void processKeys(int key) {

    switch(key)
    {
    // case SDLK_ESCAPE:  
    //     exit(0);
    case SDLK_h: print = !print; break;
    case SDLK_n:
        tracker->mapto_world_translate(0, 0, -0.01);
        break;
    case SDLK_m:
        tracker->mapto_world_translate(0, 0, 0.01);
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

    //left mouse button down + motion: rotate
    //right mouse button down + motion: zoom
    //middle mouse button down + motion: not processed in this function

    if(m == 1)
    {
        float rx = (x-px) * 180.0 / window_w;
        float ry = (y-py) * 180.0 / window_h;

        tracker->mapto_world_rotate(ry, rx, 0);
    }
    else if(m == 3)
    {
        float tz = (y-py) * 180.0 / window_h;

        tracker->mapto_world_translate(0, 0, tz);
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
    //mouse button, left=1, middle=2, right=3
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
        processMousePassiveMotion(move, x, window_h - y);
        renderer->processMouseMove(move, x, window_h - y);
        break;

    case SDL_MOUSEBUTTONDOWN:
        processMouseButtonDown(e->button.button, move);
        break;

    case SDL_MOUSEBUTTONUP:
        processMouseButtonUp(e->button.button, move);
        break;

    case SDL_KEYDOWN:
        processKeys(e->key.keysym.sym);
        renderer->processKeys(e->key.keysym.sym);
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
    //fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
    setenv_window();
    
    const char* cfgfilename = "../configs/pcaster_options_sd.cfg";;
    float global_scale = 0.5;


    if(argc == 3)
    {
        cfgfilename = argv[1];
        global_scale = atof(argv[2]);
    } 
   

    pops = new pcaster_options(0, 0, cfgfilename);

    window_w = pcaster_options::render_wid;
    window_h = pcaster_options::render_hei;
    window_noframe = pcaster_options::window_noframe;
    window_fullscreen = pcaster_options::window_fullscreen;
    screen_w = pcaster_options::screen_wid;
    screen_h = pcaster_options::screen_hei;

    tracker = new Event_Relayer(myrank, runsize,
                                GL_IO::IPC_SHARE_MEMORY | GL_IO::MOUSE_KEY,
                                GL_IO::MEM_COPY, 0);

    scmanager = new Scene_Manager();
    scmanager->register_event_processor(tracker);
    scmanager->create_camera(stereo, screen_w, screen_h);
    scmanager->create_world_transform(objpos, objrot);


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
            renderer = new Standalone_Render(window_w, window_h,
                                             myrank, runsize, 
                                             global_scale);
            renderer->init();
            renderer->assert_application_mode();

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

