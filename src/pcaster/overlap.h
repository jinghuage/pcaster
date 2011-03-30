#ifndef OVERLAP_H_
#define OVERLAP_H_


#include <vector>
#include <sstream>
#include <iostream>

#include "colorkey.h"
#include "segment_polygon.h"

class tex_unit;


class OverLap : public Segment_Polygon
{
public:
    OverLap();
    virtual ~OverLap();

    virtual void print();



public:
    //void print(std::ostringstream&);
    friend std::ostringstream& operator<< (std::ostringstream&, const OverLap&);

    void insert_process(int id) { m_processes.push_back(id); }
    void get_proc_from_key(const ColorKey&);
    bool is_singleton() { return m_singleton; }
    const std::vector<int>& get_processes() const { return m_processes; }
    void compute_root();



protected: 
    //single contribution to final image
    bool m_singleton;
    std::vector<int> m_processes; //all involving processes, include myself

};


#endif
