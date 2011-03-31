#ifndef LOG_TIMER_H
#define LOG_TIMER_H

#include <fstream>

#include "Timer.h"



class Log_Timer : public Timer{
public:
    enum logItem{LOG_InterFrame=0, LOG_OFP, LOG_Render, 
                 LOG_Pack, LOG_Exch, LOG_Comp, 
                 LOG_Sync, LOG_ScreenPack, LOG_ScreenDispatch,
                 LOG_View};

    Log_Timer(const char* log_dir, 
                     unsigned int wid,
                     unsigned int hei,
                     int cpst_mode,
                     int runsize,
                     int rank,
                     int mode);
    ~Log_Timer();


    void start_timer();
    void stop_timer();

    void log(int item=-1);
    void flush_log();

private:
    std::ofstream m_outfile;
    int m_log_item;


    void init();

};



#endif
