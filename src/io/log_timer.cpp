#include <pmisc.h>
#include "log_timer.h"



Log_Timer::Log_Timer(const char* log_dir, 
                     unsigned int wid,
                     unsigned int hei,
                     int cpst_mode,
                     int runsize,
                     int rank,
                     int timing_mode)
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    char outfilename[64];
    sprintf(outfilename, "%s/timing%d_%dx%d_%d-%d-%d.dat",
            log_dir,
            timing_mode,
            wid, 
            hei,
            cpst_mode,
            runsize, 
            rank);

    m_outfile.open(outfilename, std::ofstream::out);
    m_log_item = 0;

    init();
}



Log_Timer::~Log_Timer()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);

    m_outfile.close();
}


void Log_Timer::init()
{

    m_outfile << "itf\t"
              << "ofp\t" 
              << "render\t" 
              << "pack\t" 
              << "exch\t" 
              << "comp\t" 
              << "sync\t"
              << "spack\t"
              << "compose\t"
              << "view\n\n";

}


void Log_Timer::start_timer()
{
    start();
}


void Log_Timer::stop_timer()
{
    stop();
}


void Log_Timer::log(int item)
{
    stop();
    while(m_log_item < item)
    {
        m_outfile << "\t";
        m_log_item++;
    }

    m_outfile << getElapsedTimeInMilliSec();
    start();
}



void Log_Timer::flush_log()
{
    m_outfile << "\n";
    m_outfile.flush();
    m_log_item = 0;
}
