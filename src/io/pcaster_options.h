#ifndef PCASTER_OPTIONS_H_
#define PCASTER_OPTIONS_H_


#include <string>
#include <boost/program_options.hpp>
namespace po = boost::program_options;



class pcaster_options
{
public:

    pcaster_options(int argc, char* argv[], const char* cfgfilename);
    ~pcaster_options();


    static int   window_fullscreen;
    static int   window_noframe;

    static int   render_wid;
    static int   render_hei;    //global render size

    static float screen_wid;
    static float screen_hei;

// ===application mode===
//    static int standalone;
//    static int floating_viewport;
//    static int simulate;

    static float run_time;
    static int cpst_mode;
    static std::string log_dir;
    static int timing;
    static int debug_level;


// ===viewer===
    static std::string viewer;
    static int view_mode;
    static std::string tile_config;


// ===ray caster===
    static int bytevolume;
    static int   max_steps;
    static int   ray_steps;
    static float color_mul;


//control what to draw on your screen
    static int show_vpos;
    static int modify_tf;
    static int tf_channel;
    static int show_preint;
    static std::string SHADER_PATH;


// ===data loader===
//data timestep
    static int timestep;
    static int min_timestep;
    static int max_timestep;


//both light position and clip plane are set before transformation
/*     static int DO_CLIPPING; */
/*     static int DO_LIGHTING; */




//data path
    static std::string DATA_NAME;
    static std::string DATASET_NAME;    

    static int FILE_TYPE;
    static int DATA_TYPE;
    static int comp;
    static int border;

    static std::vector<int> data_dims;
    static std::vector<float> sample_scale;



    static int data_dist;
    static std::vector<int> chunk_dims;
    static int data_loadmode;
    static double dmin;
    static double dmax;

// ===datatree hierarchy===
    static int num_data_node;
    static int num_render_node;

// === transfer function===   
    static std::string TFfile;
    static float colorizer_r;
    static float colorizer_g;
    static float colorizer_b;
    static int peaks;
    static float peakscale;
    static float sharpness;
    static float shift;
    static float intensity;





    void parse_configfile();

    void save();


private:
    std::string m_cfgfile;
    po::options_description m_configOptions;


};

#endif
