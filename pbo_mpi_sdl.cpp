#include <SDL/SDL.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#include "renderer.h"
#include "pbo_compositor.h"
#include "Timer.h"

// #ifdef __APPLE__
// #include <OpenGL/gl.h>
// #include <GLUT/glut.h>
// #endif

static int   window_fullscreen = 0;
static int   window_noframe    = 0;
static int   window_x          = 0;
static int   window_y          = 0;
static int   window_w          = 1920;
static int   window_h          = 1200;

int myrank=0, runsize=0;
int wid=800, hei=800;
int frames;
double cutime, pretime;

//float pos[3] = {0.0f, 0.0f, -1.5f};
//float angleY = 0, angleX = 0;
float posang[6]={0.0f, 0.0f, -1.5f, 0.0f, 0.0f, 0.0f};

float lightPos[4] = {0.0, 10.0, 0.0, 1.0};

volrenderer *rder;
pbo_compositor *cpst;

//tune the volume rendering shader
int ray_advance_steps = 400;
float ray_step_size = 0.005;
float iso_thres = 0.001f;
float iso_step = 0.001f;
float isovalue = 1.005f;


//control what to draw on your screen
int show_process = 0;
bool show_volumeface = false;
bool show_chunk = false;
bool show_transferfunction = true;
int combine = 0; //draw transfer function together or seperately
bool show_preintegration = false;
bool do_preint = true;
bool overlap_by_bb = false;
bool float_volume = false; //use float volume to extract iso values


void reshape(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    wid = w;
    hei = h;

    float ratio = 0.1 * w / h;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective.
    glFrustum(-ratio, ratio, -0.1, 0.1, 0.1, 100);
    //glOrtho(-1, 1, -1, 1, -100, 100);
	
    glMatrixMode(GL_MODELVIEW);


}


GLfloat ambient[3] = {0.1, 0.1, 0.1};
GLfloat diffuse[3] = {0.8, 0.8, 0.8};
GLfloat specular[3] = {1.0, 1.0, 1.0};

void init_GL()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP_NV);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_BLEND);
    
    glClearColor(0.0, 0.0, 1.0, 1.0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glEnable(GL_CLIP_PLANE0);

    glEnable(GL_TEXTURE_2D);
	
    //glShadeModel(GL_SMOOTH);
    // glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHTING);
	
    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
    //GLfloat spot_direction[] = {-1.0, -1.0, 0.0};
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
    //glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 2.0);
        
	
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	
//     glEnable(GL_COLOR_MATERIAL);
//     glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

fbo_unit* fbos;
int nfbo, npbo;

void init_renderer()
{
    rder = new volrenderer;
    cpst = new pbo_compositor;

    if(myrank == 0)
    {
        nfbo = 2;
        npbo = runsize - 1;
    }
    else
    {
        nfbo = 1;
        npbo = 2;
    }

    fbos = rder->init_scene(runsize, myrank, nfbo, 
                            float_volume, overlap_by_bb, wid, hei, 
                            ray_advance_steps, ray_step_size, iso_thres, isovalue);

    cpst->init(npbo, wid, hei, myrank);

}

void renderScene(int fid, bool single)
{
    reshape(wid, hei);

//    printf("render scene\n");
//    glDrawBuffer(GL_BACK);
//     glClearColor(0.0, 0.0, 0.0, 1.0);
//     glClear(GL_COLOR_BUFFER_BIT);

    //glPushMatrix();
    //glRotatef(a, 1.0, 0.0, 0.0);
    //glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    //glPopMatrix();
    
    glLoadIdentity();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glTranslatef(posang[0], posang[1], posang[2]);
    glRotatef(posang[4], 0.0, 1.0, 0.0);
    glRotatef(posang[3], 1.0, 0.0, 0.0);
    //render volume into fbos[fid] 
    rder->draw_volume(-1, fid);

    if(single)
    {
        cpst->draw_screen(&fbos[fid]);
        //draw wireframe
        glPushMatrix();
        rder->draw_volume_boundingbox(-2, 1.0);
        glPopMatrix();   
        rder->draw_debug_screens(show_chunk, show_volumeface, combine, show_transferfunction, show_preintegration);
        SDL_GL_SwapBuffers();
    }


}

Timer render_t, readback_t, transfer_t, comp_t;

//slave: use one fbo and 2 pbo
void render_slave()
{
    npbo = 2;
    static int  cpbo = 0;
    int ppbo = (cpbo+1) % npbo; 
    //printf("%d: cpbo=%d, ppbo=%d\n", myrank, cpbo, ppbo);

    static int frameid = 0;
    int printid = -1;

   MPI_Bcast(posang, 6, MPI_FLOAT, 0, MPI_COMM_WORLD);
   //printf("posang(%f,%f,%f,%f,%f,%f)\n", 
   //        posang[0], posang[1], posang[2], posang[3], posang[4], posang[5]);

   if(frameid > 0)
   {
       transfer_t.start();
       cpst->network_datatransfer_start(myrank, cpbo);
       transfer_t.stop();
       if(myrank == printid) fprintf(stderr, "%d: map pbo%d time: %f\n", myrank, cpbo, transfer_t.getElapsedTimeInMilliSec());    
   }
   renderScene(0, false); 
   cpst->async_pack_pbo(&fbos[0], ppbo);
   if(frameid > 0)
   {
       transfer_t.start();
       cpst->network_datatransfer_wait(myrank, cpbo);
       cpst->release_pbo(cpbo);
       transfer_t.stop();
       if(myrank == printid) fprintf(stderr, "%d: Isend wait time: %f\n", myrank, transfer_t.getElapsedTimeInMilliSec());    
   }

   cpbo = ppbo;
   frameid++;
}


//master: use 2 fbo and runsize-1 pbos
void render_master()
{
    nfbo = 2;
    static int  cfbo = 0;
    int p2fbo = (cfbo+2) % nfbo; 
    //printf("%d: cfbo=%d, pfbo=%d\n", myrank, cfbo, pfbo);

    static int frameid = 0;
    int printid = -1;

    MPI_Bcast(posang, 6, MPI_FLOAT, 0, MPI_COMM_WORLD);
    //printf("posang(%f,%f,%f,%f,%f,%f)\n", 
    //        posang[0], posang[1], posang[2], posang[3], posang[4], posang[5]);

    if(frameid > 1)
    {
        comp_t.start();

        if(show_process == 0) cpst->draw_screen_accm(&fbos[p2fbo]);
        else if(show_process == 1) cpst->draw_screen(&fbos[p2fbo]);
        else if(show_process == 2) cpst->draw_screen(0);

        SDL_GL_SwapBuffers();
        cpst->release_all_pbo_tex();
        comp_t.stop();
        if(myrank == printid) fprintf(stderr, "%d: composite time: %f\n", myrank, comp_t.getElapsedTimeInMilliSec() ); 
    }
   
    if(frameid > 0)
    {
        transfer_t.start();
        cpst->network_datatransfer_start(myrank, -1);
        transfer_t.stop();
        if(myrank == printid) fprintf(stderr, "%d: map all pbos time: %f\n", myrank, transfer_t.getElapsedTimeInMilliSec());    

    }
    renderScene(cfbo, false);
    if(frameid > 0)
    {
        transfer_t.start();
        cpst->network_datatransfer_wait(myrank, -1);
        transfer_t.stop();
        if(myrank == printid) fprintf(stderr, "%d: Irecv wait time: %f\n", myrank, transfer_t.getElapsedTimeInMilliSec());    

    }

    cfbo = (cfbo+1) % nfbo;
    frameid ++;
}

//one node two mpi process
//statistics: 
//draw volume: 6ms (GPU)
//read back: 1ms (GPU)
//send-recv: 5ms   (CPU+NET) 
//composite 2 texture:  0.03ms (GPU)

//so, expect that for 8 processes:
//draw volume: 12ms (GPU)
//read back: 2ms (GPU)
//send: 2-3ms
//recv: 40ms(CPU+NET)
//composite 8 textures: 0.1ms (GPU)


//note: this part of program is very much GPU heavy
//so basically need to multiplex GPU with CPU+NET.
//how to multiplex: GPU working on current frame, while CPU+NET working on the 
//previous frames. 
//scene is rendered to fbo, and composited in GPU
//so master node don't need to read back pixels



void render_seperate_measurement(void) {

    int printid = 0;
    int cfbo = 0;
    int cpbo = 0;

    //fprintf(stderr, "%d: before bcase\n", myrank);
    MPI_Bcast(posang, 6, MPI_FLOAT, 0, MPI_COMM_WORLD);
    //fprintf(stderr, "%d: after bcast\n", myrank);

    render_t.start();
    //renderScene(cfbo, false);
    //note: use glFinish() here. glFlush will not work
    //glFinish();
    SDL_GL_SwapBuffers();
    render_t.stop();
    if(myrank == printid) fprintf(stderr, "%d: render time: %f\n", myrank, render_t.getElapsedTimeInMilliSec());    
    
    if(myrank != 0)
    {
        readback_t.start();
        if(myrank!=0) cpst->async_pack_pbo(fbos+cfbo, cpbo);
        cpst->finish_pbo(myrank, cpbo);
        readback_t.stop();
        if(myrank == printid) fprintf(stderr, "%d: readback time: %f\n", myrank, readback_t.getElapsedTimeInMilliSec());    
    }

    transfer_t.start();
    cpst->network_datatransfer_start(myrank, cpbo);
    cpst->network_datatransfer_wait(myrank, cpbo);
    if(myrank != 0) cpst->release_pbo(cpbo);
    transfer_t.stop();
    if(myrank == printid) fprintf(stderr, "%d: data transfer time: %f\n", myrank, transfer_t.getElapsedTimeInMilliSec());

    if(myrank==0) 
    {
        comp_t.start();
        //cpst->draw_screen_accm(fbos+cfbo);
        cpst->draw_screen(cpbo);
        SDL_GL_SwapBuffers();
        cpst->release_all_pbo_tex();
        comp_t.stop();
        if(myrank == printid) fprintf(stderr, "%d: composite time: %f\n", myrank, comp_t.getElapsedTimeInMilliSec());    
    }
}


void mpi_test()
{
    fprintf(stderr, "%d: before bcase\n", myrank);
    MPI_Bcast(posang, 6, MPI_FLOAT, 0, MPI_COMM_WORLD);
    fprintf(stderr, "%d: after bcast\n", myrank);
}

void memcpy_test()
{
    int printid = 0;
    comp_t.start();
    cpst->memcpy_test();
    comp_t.stop();
    if(myrank == printid) fprintf(stderr, "%d: memcpy time: %f\n", myrank, comp_t.getElapsedTimeInMilliSec());    

}

void processKeys(int key) {

    switch(key)
    {
    case SDLK_ESCAPE:    exit(0);
    case SDLK_c: overlap_by_bb = !overlap_by_bb; break;
    case SDLK_a: posang[0] -= 0.1; break;
    case SDLK_d: posang[0] += 0.1; break;
    case SDLK_w: posang[2] += 0.2; break;
    case SDLK_z: posang[2] -= 0.2; break;
    case SDLK_m:iso_thres *= 2.0; break;
    case SDLK_n: iso_thres /= 2.0; break;
    case SDLK_j: isovalue -= iso_step; break;
    case SDLK_k: isovalue += iso_step; break;
    case SDLK_p:
        printf("toggle show pre-integration texture\n");
        show_preintegration = !show_preintegration;
        break;
    case SDLK_t:
        printf("toggle show transfer function curves\n");
        show_transferfunction = !show_transferfunction;
        break;
    case SDLK_f:
        printf("toggle show fbo textures\n");
        show_volumeface = !show_volumeface;
        break;
    case SDLK_b:
        printf("toggle show fbo chunk\n");
        show_chunk = !show_chunk;
        break;
    case SDLK_v:
        show_process ++;
        show_process %= 3;
        printf("show_process view: %d\n", show_process);
    }

    if (SDL_GetModState() & KMOD_SHIFT)
            
        switch (key) /* Shifted keys. */
        {
        case SDLK_DOWN:  
            ray_advance_steps = 1000;  
            printf("set ray steps to be 1000\n");
            break;
        case SDLK_UP:    
            ray_advance_steps = 1;  
            printf("set ray steps to be 1\n");
            break;
        }

    else
            
        switch (key) /* Unshifted keys. */
        {
        case SDLK_LEFT:  ray_step_size /= 2.0; break;
        case SDLK_RIGHT: ray_step_size *= 2.0; break;
        case SDLK_DOWN:  
            ray_advance_steps --; 
            printf("ray steps: %d\n", ray_advance_steps);
            break;
        case SDLK_UP:    
            ray_advance_steps ++; 
            printf("ray steps: %d\n", ray_advance_steps);
            break;
        }

}


void processMousePassiveMotion(int x, int y) 
{

    // setting the angle to be relative to the mouse 
    // position inside the window
    if (x < 0)
        posang[4] = 0.0;
    else if (x > wid)
        posang[4] = 180.0;
    else
        posang[4] = 180.0 * ((float) x)/wid;


    if (y < 0)
        posang[3] = 0.0;
    else if (y > hei)
        posang[3] = 180.0;
    else
        posang[3] = 180.0 * ((float) y)/hei;

}

void processMouseButton(int button) 
{


    if (button == SDL_BUTTON(1)) 
        {
            //rest
            posang[3] = 0; posang[4] = 0;
        }
    else if (button == SDL_BUTTON(2)) 
        {         
            //snap
            if( abs(posang[3]) < 15.0 ) posang[3] = 0.;
            else if( abs(posang[3]-45.) < 15. ) posang[3] = 45.;
            else if( abs(posang[3]-90.) < 15. ) posang[3] = 90.;
            else if( abs(posang[3]-135.) < 15. ) posang[3] = 135.;
            else if( abs(posang[3]-180.) < 15. ) posang[3] = 180;

            if( abs(posang[4]) < 15.0 ) posang[4] = 0.;
            else if( abs(posang[4]-45.) < 15. ) posang[4] = 45.;
            else if( abs(posang[4]-90.) < 15. ) posang[4] = 90.;
            else if( abs(posang[4]-135.) < 15. ) posang[4] = 135.;
            else if( abs(posang[4]-180.) < 15. ) posang[4] = 180;

        }

}


/*---------------------------------------------------------------------------*/

static int do_event(SDL_Event *e)
{
    static int x = 0;
    static int y = 0;

    switch (e->type)
    {
    case SDL_MOUSEMOTION:
        processMousePassiveMotion(x, y);
        x = e->motion.x;
        y = e->motion.y;
        break;

    case SDL_MOUSEBUTTONDOWN:
        processMouseButton(e->button.button);
        break;


    case SDL_KEYDOWN:
        processKeys(e->key.keysym.sym);
        break;

    case SDL_QUIT:
        return 0;
    }

    return 1;
}


void idle()
{
    //printf("idle\n");
    char title[256];

    frames++;
    cutime = getTimeInSecs();
    if( (cutime - pretime) >= 1.0)
    {
        //sprintf(title, "frame rate: %d fps", frames);
        sprintf(title, "rcsteps=%d, ssize=%.3f, isovalue=%.3f, iso_thres=%.6f @%dfps", 
                ray_advance_steps, ray_step_size, isovalue, iso_thres, frames);
        //if(draw_whole_volume) strcat(title, ",whole");
        
        if(overlap_by_bb) strcat(title, ",overlap");
        //glutSetWindowTitle(title);
        SDL_WM_SetCaption(title, "volume renderer");
        
        frames = 0;
        pretime = cutime;
    }

    rder->update(ray_advance_steps, ray_step_size, iso_thres, isovalue);
}


void init_mpi()
{
  /* Initialize MPI */

    MPI_Init(NULL, NULL);

    /* Find out my identity in the default communicator */

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &runsize);

    //test
//     runsize = 2;
//     myrank = 0;
}


int main(int argc, char *argv[])
{
    init_mpi();
    frames=0;
    cutime = pretime = getTimeInSecs();

    char buf[32];

#ifdef _WIN32
    sprintf(buf, "SDL_VIDEO_WINDOW_POS=%d,%d", window_x, window_y);
    putenv(buf);
#else
    sprintf(buf, "%d,%d", window_x, window_y);
    setenv("SDL_VIDEO_WINDOW_POS", buf, 1);
#endif

    /* Initialize SDL and create an OpenGL window. */

    if(myrank == 0) window_w = window_h = 800;
    else window_w = window_h = 1;
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (SDL_SetVideoMode(window_w, window_h, 0,
                             window_noframe    * SDL_NOFRAME    |
                             window_fullscreen * SDL_FULLSCREEN |
                             SDL_OPENGL))
        {
            //glewInit();
            init_glew_ext();
            init_GL();

//             runsize = 2;
//             myrank = 1;
            init_renderer();

            /* Start the application. */


            int run = 1;

            /* Run the main loop. */

            while (run)
            {
                SDL_Event e;

                /* Process all pending events. */

//                  if    (SDL_WaitEvent(&e)) run &= do_event(&e);
                while (SDL_PollEvent(&e)) run &= do_event(&e);
                idle();

                /* Render the scene. */
//                renderScene(0, true);

                if(runsize == 1) renderScene(0, true);
                else
                {
                    //render_seperate_measurement();
                    if(myrank == 0) render_master();
                    else render_slave();
                }
 
            }
        }
        else fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    
    MPI_Finalize();
    return 0;
}

/*---------------------------------------------------------------------------*/
