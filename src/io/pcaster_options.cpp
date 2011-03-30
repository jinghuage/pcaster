#include <assert.h>
#include <math.h>
#include <string.h>


#include <iostream>
#include <fstream>

#include <pmisc.h>



#include "pcaster_options.h"


//can initialize variables here. but since we use program options anyway
//so default can be set through options
 
int pcaster_options::   window_fullscreen(0);
int pcaster_options::   window_noframe(0);

int pcaster_options::   render_wid(1024);
int pcaster_options::   render_hei(1024);   

float pcaster_options::   screen_wid(1.0);
float pcaster_options::   screen_hei(1.0);   

float pcaster_options::run_time(30.f);
int pcaster_options:: cpst_mode(0);
std::string pcaster_options:: log_dir("./");
int pcaster_options:: timing(0);
int pcaster_options:: debug_level(0);

std::string pcaster_options::viewer("none");
int pcaster_options:: view_mode(0);
std::string pcaster_options::tile_config("2 2");

int pcaster_options:: bytevolume(1);
int pcaster_options::max_steps(1000);
int pcaster_options::ray_steps(256);
float pcaster_options:: color_mul(1.0);


int pcaster_options:: show_vpos(0);
int pcaster_options:: modify_tf(0);
int pcaster_options:: tf_channel(3);
int pcaster_options:: show_preint(0);

std::string pcaster_options:: SHADER_PATH("./");

int pcaster_options:: timestep(0);
int pcaster_options:: min_timestep(0);
int pcaster_options:: max_timestep(0);


std::string pcaster_options:: DATA_NAME("");
std::string pcaster_options:: DATASET_NAME("");
int pcaster_options::FILE_TYPE(0);
int pcaster_options::DATA_TYPE(0);
int pcaster_options:: comp(1);
int pcaster_options:: border(0);


std::vector<int> pcaster_options::data_dims(4, 0);
std::vector<float> pcaster_options::sample_scale(3, 1.0);


int pcaster_options:: data_dist(0);
std::vector<int> pcaster_options::chunk_dims(3, 0);
int pcaster_options:: data_loadmode(0);


double pcaster_options:: dmin(0.0);
double pcaster_options:: dmax(1.0);

int pcaster_options:: num_data_node(1);
int pcaster_options:: num_render_node(1);

std::string pcaster_options:: TFfile("");
float pcaster_options::colorizer_r(1.0);
float pcaster_options::colorizer_g(1.0);
float pcaster_options::colorizer_b(1.0);
int pcaster_options::peaks(1);
float pcaster_options::peakscale(1.0);
float pcaster_options::sharpness(0.9);
float pcaster_options::shift(3.14);
float pcaster_options::intensity(0.4);





pcaster_options::pcaster_options(int argc, char* argv[],
                                 const char* cfgfilename):
    m_cfgfile(cfgfilename),
    m_configOptions("Configurations")
{

//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    //po::options_description configOptions("Configurations");

    m_configOptions.add_options()
	("window_fullscreen", po::value<int>(&window_fullscreen), "fullscreen window")
        ("window_noframe", po::value<int>(&window_noframe), "window frame")

        ("render_wid", po::value<int>(&render_wid), "render image wid")
        ("render_hei", po::value<int>(&render_hei), "render image hei")

        ("screen_wid", po::value<float>(&screen_wid), "physical screen wid")
        ("screen_hei", po::value<float>(&screen_hei), "physical screen hei")

        ("run_time", po::value<float>(&run_time), "run time until program quits")          
        ("cpst_mode", po::value<int>(&cpst_mode), "set compositing mode: GPU vs. CPU")	
        ("log_dir", po::value<std::string>(&log_dir),  "log file directory")
        ("timing", po::value<int>(&timing), "save timing info")	
        ("debug_level", po::value<int>(&debug_level), "Debug level (0 - lowest = NONE, 1 = FATAL, 2 = ERROR, 3 = WARN, 4 = INFO, 5 = DEBUG, 6 = TRACE, 7 -highest = ALL")
 
        ("viewer", po::value<std::string>(&viewer), "viewer host")
        ("view_mode", po::value<int>(&view_mode), "tile or single")	
        ("tile_config", po::value<std::string>(&tile_config), "tile display configuration")

        ("bytevolume", po::value<int>(&bytevolume), "byte volume")
        ("max_steps", po::value<int>(&max_steps), "ray casting max step number")
        ("ray_steps", po::value<int>(&ray_steps), "ray casting unit step number")
        ("color_mul", po::value<float>(&color_mul), "color multiplier")

        ("show_vpos", po::value<int>(&show_vpos), "show front and back faces")
        ("modify_tf", po::value<int>(&modify_tf), "modify transfer function")
        ("tf_channel", po::value<int>(&tf_channel), "default transfer function channel")
        ("show_preint", po::value<int>(&show_preint), "show pre-integration table texture")
        ("SHADER_PATH", po::value<std::string>(&SHADER_PATH),  "shader path")

        ("timestep", po::value<int>(&timestep), "data time step")
        ("min_timestep", po::value<int>(&min_timestep), "Min data time step")
        ("max_timestep", po::value<int>(&max_timestep), "Max data time step")

//         ("DO_CLIPPING", po::value<int>(&DO_CLIPPING), "apply a clipping plane")
//         ("DO_LIGHTING", po::value<int>(&DO_LIGHTING), "apply lighting")



        ("DATA_NAME", po::value<std::string>(&DATA_NAME),  "data major name")
        ("DATASET_NAME", po::value<std::string>(&DATASET_NAME),  "dataset name")
        ("FILE_TYPE", po::value<int>(&FILE_TYPE), "file type")
        ("DATA_TYPE", po::value<int>(&DATA_TYPE), "data type")        
        ("comp", po::value<int>(&comp), "comp")
        ("border", po::value<int>(&border), "border")

        ("data_dims", po::value<std::string>(), "full data dimension")
        ("sample_scale", po::value<std::string>(), "data sampling scale")        

        ("data_dist", po::value<int>(&data_dist), "data distribution")
        ("chunk_dims", po::value<std::string>(), "data chunk dimension")
        ("data_loadmode", po::value<int>(&data_loadmode), "data load mode")

        ("dmin", po::value<double>(&dmin), "data range min")
        ("dmax", po::value<double>(&dmax), "data range max")

        ("num_data_node", po::value<int>(&num_data_node), "number of data nodes")
        ("num_render_node", po::value<int>(&num_render_node), "number of nodes rendering ")

        ("TFfile", po::value<std::string>(&TFfile),  "transfer function file")
        ("colorizer_r", po::value<float>(&colorizer_r), "colorizer_r")
        ("colorizer_g", po::value<float>(&colorizer_g), "colorizer_g")
        ("colorizer_b", po::value<float>(&colorizer_b), "colorizer_b")
        ("peaks", po::value<int>(&peaks), "peaks")
        ("peakscale", po::value<float>(&peakscale), "peakscale")
        ("sharpness", po::value<float>(&sharpness), "sharpness")
        ("shift", po::value<float>(&shift), "shift")
        ("intensity", po::value<float>(&intensity), "intensity")
       ;


    po::options_description cmdlineOptions("Command Line Options");

    cmdlineOptions.add_options()
        ("version,v", "print version string")
        ("help", "produce help message")  
        ("cpst_mode,c", po::value<int>(&cpst_mode), "set compositing mode: GPU vs. CPU")	
        ("timing,t", po::value<int>(&timing), "save timing info")	
        ("debug_level,d", po::value<int>(&debug_level), "Debug level (0 - lowest = NONE, 1 = FATAL, 2 = ERROR, 3 = WARN, 4 = INFO, 5 = DEBUG, 6 = TRACE, 7 -highest = ALL")
        ("log_dir,l", po::value<std::string>(&log_dir),  "log file directory")
        ("render_wid,w", po::value<int>(&render_wid), "render image wid")
        ("render_hei,h", po::value<int>(&render_hei), "render image hei")
        ("run_time,r", po::value<float>(&run_time), "run time until program quits")        
        ;

//     int opt_style =
//         (po::command_line_style::allow_short |                      
//          po::command_line_style::allow_long |
//          po::command_line_style::long_allow_adjacent | 
//          po::command_line_style::short_allow_adjacent | 
//          po::command_line_style::long_allow_next |
//          po::command_line_style::short_allow_next | 
//          po::command_line_style::allow_sticky | 
//          po::command_line_style::allow_dash_for_short); 


    po::options_description all_config_options("Allowed options");
    all_config_options.add(m_configOptions).add(cmdlineOptions);

/*
//parse the commandline first
    po::variables_map varmap;

    fprintf(stderr, "read from command line arguments\n");
    po::store(po::parse_command_line(argc, argv, cmdlineOptions, opt_style), 
              varmap);

    notify(varmap);

    if (varmap.count("help")) {
        std::cout << all_config_options << "\n";
        exit(0);
    }

    if (varmap.count("version")) {
        std::cout << "Parallel ray casting volume renderer, version 1.0\n";
    }
*/


    parse_configfile();
}



pcaster_options::~pcaster_options()
{
    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);
}



void pcaster_options::parse_configfile()
{
    po::variables_map varmap;

#ifdef _DEBUG
    //parse the config file
    fprintf(stderr, "read from config file %s\n", m_cfgfile.c_str());
#endif


    std::ifstream ifs(m_cfgfile.c_str());            
    po::store(po::parse_config_file(ifs, m_configOptions), varmap);


    notify(varmap);


    if (varmap.count("viewer"))
    {
        std::cout << "Set viewer to " << pcaster_options::viewer << "\n";
    }


    if(varmap.count("data_dims")){
        const std::string& s = varmap["data_dims"].as<std::string>();
        std::istringstream iss(s);
        iss >> data_dims[0] >> data_dims[1] >> data_dims[2] >> data_dims[3];
        //std::cout << "data dims(" << data_dims[0] << "," << data_dims[1] << "," << data_dims[2] << "," << data_dims[3] << ")\n";
    }

    if(varmap.count("sample_scale")){
        const std::string& s = varmap["sample_scale"].as<std::string>();
        std::istringstream iss(s);
        iss >> sample_scale[0] >> sample_scale[1] >> sample_scale[2];
        //std::cout << "sample scale(" << sample_scale[0] << "," << sample_scale[1] << "," << sample_scale[2] << ")\n";
    }

    if(varmap.count("chunk_dims")){
        const std::string& s = varmap["chunk_dims"].as<std::string>();
        std::istringstream iss(s);
        iss >> chunk_dims[0] >> chunk_dims[1] >> chunk_dims[2];
        //std::cout << "chunk dims(" << chunk_dims[0] << "," << chunk_dims[1] << "," << chunk_dims[2] << ")\n";
    }

}



void pcaster_options::save()
{
    FILE* fp = fopen(m_cfgfile.c_str(), "w");
    if(fp == 0) 
    {
        fprintf(stderr, "ERROR: %s:%s: can't open file %s to write\n", 
                __FILE__, __func__, m_cfgfile.c_str());
        return;
    }

    //fprintf(fp, "# === ===\n");
    fprintf(fp, "# ===window setup===\n");
    fprintf(fp, "window_fullscreen = \t %d\n", window_fullscreen);
    fprintf(fp, "window_noframe = \t %d\n\n", window_noframe);

    fprintf(fp, "# ===render size===\n");
    fprintf(fp, "render_wid = \t %d\n", render_wid);
    fprintf(fp, "render_hei = \t %d\n\n", render_hei);

    fprintf(fp, "# ===physical screen size===\n");
    fprintf(fp, "screen_wid = \t %f\n", screen_wid);
    fprintf(fp, "screen_hei = \t %f\n\n", screen_hei);
//     fprintf(fp, "# ===application mode===\n");
//     fprintf(fp, "standalone = \t %d\n", standalone);
//     fprintf(fp, "floating_viewport = \t %d\n", floating_viewport);
//    fprintf(fp, "simulate = \t %d\n", simulate);
    fprintf(fp, "run_time = \t %f\n", run_time);
    fprintf(fp, "cpst_mode = \t %d\n", cpst_mode);
    fprintf(fp, "log_dir = \t %s\n", log_dir.c_str());
    fprintf(fp, "timing = \t %d\n", timing);
    fprintf(fp, "debug_level = \t %d\n", debug_level);

    fprintf(fp, "# ===viewer===\n");
    fprintf(fp, "viewer = \t %s\n\n", viewer.c_str());
    fprintf(fp, "view_mode = \t %d\n", view_mode);
    fprintf(fp, "tile_config = \t %s\n\n", tile_config.c_str());
    
    fprintf(fp, "# ===ray caster ===\n");    
    fprintf(fp, "bytevolume = \t %d\n\n", bytevolume);
    fprintf(fp, "max_steps = \t %d\n", max_steps);
    fprintf(fp, "ray_steps = \t %d\n", ray_steps);
    fprintf(fp, "color_mul = \t %f\n", color_mul);

    fprintf(fp, "show_vpos = \t %d\n", show_vpos);
    fprintf(fp, "modify_tf = \t %d\n", modify_tf);
    fprintf(fp, "tf_channel = \t %d\n", tf_channel);
    fprintf(fp, "show_preint = \t %d\n\n", show_preint);

    fprintf(fp, "SHADER_PATH = \t %s\n\n", SHADER_PATH.c_str());

    fprintf(fp, "# ===data loader ===\n");
    fprintf(fp, "timestep = \t %d\n", timestep);
    fprintf(fp, "min_timestep = \t %d\n", min_timestep);
    fprintf(fp, "max_timestep = \t %d\n\n", max_timestep);

//     fprintf(fp, "DO_CLIPPING = \t %d\n", DO_CLIPPING);
//     fprintf(fp, "DO_LIGHTING = \t %d\n\n", DO_LIGHTING);




    fprintf(fp, "DATA_NAME = \t %s\n", DATA_NAME.c_str());
    fprintf(fp, "DATASET_NAME = \t %s\n", DATASET_NAME.c_str());
    fprintf(fp, "FILE_TYPE = \t %d\n", FILE_TYPE);
    fprintf(fp, "DATA_TYPE = \t %d\n\n", DATA_TYPE);
    fprintf(fp, "comp = \t %d\n", comp);
    fprintf(fp, "border = \t %d\n", border);


    fprintf(fp, "data_dims = \t %d %d %d %d\n", 
            data_dims[0], data_dims[1], data_dims[2], data_dims[3]);
    fprintf(fp, "sample_scale = \t %f %f %f\n", 
            sample_scale[0], sample_scale[1], sample_scale[2]); 


    fprintf(fp, "data_dist = \t %d\n\n", data_dist);
    fprintf(fp, "chunk_dims = \t %d %d %d\n", 
            chunk_dims[0], chunk_dims[1], chunk_dims[2]);
    fprintf(fp, "data_loadmode = \t %d\n\n", data_loadmode);

    fprintf(fp, "dmin = \t %f\n", dmin);
    fprintf(fp, "dmax = \t %f\n\n", dmax);


    fprintf(fp, "# ===datatree hierarchy ===\n");
    fprintf(fp, "num_data_node = \t %d\n", num_data_node);
    fprintf(fp, "num_render_node = \t %d\n", num_render_node);

    fprintf(fp, "# ===transfer function ===\n");
    fprintf(fp, "TFfile = \t %s\n", TFfile.c_str());
    fprintf(fp, "colorizer_r = \t %f\n", colorizer_r);
    fprintf(fp, "colorizer_g = \t %f\n", colorizer_g);
    fprintf(fp, "colorizer_b = \t %f\n", colorizer_b);
    fprintf(fp, "peaks = \t %d\n", peaks);
    fprintf(fp, "peakscale = \t %f\n", peakscale);
    fprintf(fp, "sharpness = \t %f\n", sharpness);
    fprintf(fp, "shift = \t %f\n", shift);
    fprintf(fp, "intensity = \t %f\n\n", intensity);




//    fprintf(fp, " = \t %d\n", );
    fclose(fp);
}





