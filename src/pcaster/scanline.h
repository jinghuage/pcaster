#ifndef SCANLINE_H
#define SCANLINE_H

#include <string.h>
#include <iostream>
#include <vector>


#include "colorkey.h"


class ScanLine
{
public: 
    ScanLine() : m_id(0) {}
    ~ScanLine()
    {
        m_segments.clear();
        m_keys.clear();
    }

    void set_id(int id) {m_id = id;}
    void add_key(ColorKey& K) {m_keys.push_back(K); }
    void add_key(unsigned int k, unsigned int pos)
    {
        m_keys.push_back(ColorKey(k, pos));
    }
    void add_seg(unsigned int s) {m_segments.push_back(s); }

    void set_seg(unsigned int* first, unsigned int* last)
    {
        m_segments = std::vector<unsigned int>(first, last);        
    }
    void set_key(ColorKey* first, ColorKey* last)
    {
        m_keys = std::vector<ColorKey>(first, last);        
    }

    unsigned int get_size() const { return m_segments.size() / 2; }
    const std::vector<unsigned int>& get_segments() const { return m_segments; }
    const std::vector<ColorKey>& get_keys() const { return m_keys; }
    unsigned int get_id() const { return m_id; }

   
    void print()
    {
        std::vector<unsigned int>::iterator it;
        for(it=m_segments.begin(); it<m_segments.end(); it++)
        {
            std::cout << (*it) << ",";
        }
        std::cout << "\n";
    }
    void print_key()
    {
        std::vector<ColorKey>::iterator it;
        for(it=m_keys.begin(); it<m_keys.end(); it++)
        {
            //(*it).print();
            std::cout << (*it) << ",";
        }
        std::cout << "\n";
    }
    ScanLine& operator=(const ScanLine& rhs);
    ScanLine& operator+=(const ScanLine& rhs);

    bool retrieve_current(unsigned int& l, unsigned int& r, ColorKey& ck)
    {
        if(m_sit == m_segments.end() && m_kit == m_keys.end()) return false;
        else
        {
            l = *m_sit++;
            r = *m_sit++;
            ck = *m_kit++;
            //std::cout << "get seg( " << l << "," << r << "," << ck << ")\n";
        }
        return true;
    }

    void set_origin()
    {
      m_sit = m_segments.begin();
      m_kit = m_keys.begin();
    }

private:
    unsigned int m_id;
    std::vector<unsigned int> m_segments;
    std::vector<ColorKey> m_keys;

    std::vector<unsigned int>::const_iterator m_sit;
    std::vector<ColorKey>::const_iterator m_kit;

};

#endif
