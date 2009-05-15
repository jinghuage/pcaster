#include <SDL/SDL.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "renderer.h"
//#include "pbo_compositor.h"
#include "Timer.h"

static int   window_fullscreen = 0;
static int   window_noframe    = 0;
static int   window_x          = 0;
static int   window_y          = 0;
static int   window_w          = 1920;
static int   window_h          = 1200;

int wid=800, hei=800;
int frames;
double cutime, pretime;

//float pos[3] = {0.0f, 0.0f, -1.5f};
//float angleY = 0, angleX = 0;
float posang[6]={0.0f, 0.0f, -1.5f, 0.0f, 0.0f, 0.0f};

float lightPos[4] = {0.0, 10.0, 0.0, 1.0};

volrenderer *rder;

//tune the volume rendering shader
int ray_advance_steps = 400;
float ray_step_size = 0.005;
float iso_thres = 0.001f;
float iso_step = 0.00005f;
float isovalue = 0.001f;


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

void init_renderer()
{
    rder = new volrenderer;

    int grps = 1;
    int rank = 0;
    float_volume = true;

    rder->init(wid, hei, float_volume, grps);
    for(rank = 0; rank<grps; rank++)
    {
        rder->init_data(rank);
    }

    rder->update_raycaster( ray_advance_steps, ray_step_size, iso_thres, isovalue);

}

void renderScene(int fid)
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
    //render volume
    //first -1 means draw the whole data
    //second -1 means draw to the frame buffer
    rder->draw_volume( -1, -1 );


    //draw wireframe
    glPushMatrix();
    rder->draw_volume_boundingbox(-2, 1.0);
    glPopMatrix();   
    rder->draw_debug_screens(show_chunk, show_volumeface, combine, show_transferfunction, show_preintegration);
    SDL_GL_SwapBuffers();

}



void processKeys(int key) {

    switch(key)
    {
    case SDLK_ESCAPE:    exit(0);
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
        sprintf(title, "rcsteps=%d, ssize=%.3f, isovalue=%.5f, iso_thres=%.6f @%dfps", 
                ray_advance_steps, ray_step_size, isovalue, iso_thres, frames);
        //if(draw_whole_volume) strcat(title, ",whole");
        
        if(overlap_by_bb) strcat(title, ",overlap");
        //glutSetWindowTitle(title);
        SDL_WM_SetCaption(title, "volume renderer");
        
        frames = 0;
        pretime = cutime;
    }

    rder->update_raycaster(ray_advance_steps, ray_step_size, iso_thres, isovalue);
}



int main(int argc, char *argv[])
{
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

    window_w = window_h = 800;

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
                renderScene(0);

            }
        }
        else fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());

        SDL_Quit();
    }
    else fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    
    return 0;
}

/*---------------------------------------------------------------------------*/
