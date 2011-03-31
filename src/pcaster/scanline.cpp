#include <assert.h>

#include "scanline.h"


ScanLine& ScanLine::operator=(const ScanLine& rhs)
{

    if(this != &rhs)
    {
        m_id = rhs.get_id();
        m_segments.clear();
        m_keys.clear();
        m_segments = rhs.get_segments();
        m_keys = rhs.get_keys();
    }

    return *this;
}


ScanLine& ScanLine::operator+=(const ScanLine& rhs)
{
    
    std::vector<unsigned int> nseg;
    std::vector<ColorKey> nck;

    unsigned int size1 = get_size();
    unsigned int size2 = rhs.get_size();

    if(size2==0) return *this;
    else if(size1==0) { *this = rhs; return *this;}

    unsigned int nsize = size1 + size2;


    nseg.resize(nsize * 4);
    nck.resize(nsize * 2);



    std::vector<unsigned int>::const_iterator first1 = m_segments.begin();
    std::vector<unsigned int>::const_iterator last1 = m_segments.end();

    const std::vector<unsigned int>& seg = rhs.get_segments();
    std::vector<unsigned int>::const_iterator first2 = seg.begin();
    std::vector<unsigned int>::const_iterator last2 = seg.end();

    std::vector<unsigned int>::iterator result = nseg.begin();

    std::vector<ColorKey>::const_iterator kf1 = m_keys.begin();
    std::vector<ColorKey>::const_iterator kl1 = m_keys.end();

    const std::vector<ColorKey>& key = rhs.get_keys();
    std::vector<ColorKey>::const_iterator kf2 = key.begin();
    std::vector<ColorKey>::const_iterator kl2 = key.end();

    std::vector<ColorKey>::iterator kr= nck.begin();

    bool proceed1 = true;
    bool proceed2 = true;
    unsigned int seg1_l=0, seg1_r=0;
    unsigned int seg2_l=0, seg2_r=0;
    ColorKey k1, k2;
    bool P = false;

    while(true)
    {
        //std::merge algorithm implementation
        //*result++ = (*first1 < *first2) ? *first1++ : *first2++;
        //if(first1 == last1) return std::copy(first2, last2, result);
        //if(first2 == last2) return std::copy(first1, last1, result);

        //merge segments algorithm
        if(proceed1) 
        { 
            seg1_l = *first1++; 
            seg1_r = *first1++; 
            k1 = *kf1++;
        }
        if(proceed2) 
        { 
            seg2_l = *first2++; 
            seg2_r = *first2++; 
            k2 = *kf2++;
        }

        if(P) std::cout << "merge seg1(" << seg1_l << "," << seg1_r << ") and seg2("
                  << seg2_l << "," << seg2_r << "):";

        //check all relationships between seg1 and seg2, find intersections
        if( seg1_r < seg2_l) //seg1 leads seg2
        {
            if(P) std::cout << "seg1 leads seg2. ";
            *result++ = seg1_l;
            *result++ = seg1_r;
            *kr++ = k1;
            proceed1 = true;
            proceed2 = false;
            if(P) std::cout << "new seg(" << seg1_l << "," << seg1_r << ")\n";
        }
        else if(seg1_l > seg2_r) //seg2 leads seg1
        {
            if(P) std::cout << "seg2 leads seg1. ";
            *result++ = seg2_l;
            *result++ = seg2_r;
            *kr++ = k2;
            proceed1 = false;
            proceed2 = true;
            if(P) std::cout << "new seg(" << seg2_l << "," << seg2_r << ")\n";
        }
        else if(seg1_l <= seg2_l && seg1_r >= seg2_r) //seg1 contains seg2
        {
            if(P) std::cout << "seg1 contains seg2. ";
            if(seg1_l < seg2_l)
            {
                *result++ = seg1_l;
                *result++ = seg2_l-1;
                *kr++ = k1;
                if(P) std::cout << "new seg(" << seg1_l << "," << seg2_l-1 << "). ";
            }
            *result++ = seg2_l;
            *result++ = seg2_r;
            *kr++ = k1 + k2;
            if(P) std::cout << "new seg(" << seg2_l << "," << seg2_r << ")\n";

            if(seg1_r == seg2_r) proceed1 = true; 
            else 
            {
                proceed1 = false;
                seg1_l = seg2_r+1;
            }
            proceed2 = true;
            
        }
        else if(seg2_l <= seg1_l && seg2_r >= seg1_r) //seg2 contains seg1
        {
            if(P) std::cout << "seg2 contains seg1. ";
            if(seg2_l < seg1_l)
            {
                *result++ = seg2_l;
                *result++ = seg1_l-1;
                *kr++ = k2;
                if(P) std::cout << "new seg(" << seg2_l << "," << seg1_l-1 << "). ";
            }
            *result++ = seg1_l;
            *result++ = seg1_r;
            *kr++ = k1 + k2;
            if(P) std::cout << "new seg(" << seg1_l << "," << seg1_r << ")\n";

            proceed1 = true;
            if(seg1_r == seg2_r) proceed1 = true; 
            else 
            {
                proceed2 = false;
                seg2_l = seg1_r+1;
            }
            
        }
        else if(seg1_r >= seg2_l && seg1_r < seg2_r) //seg1 lead-intersec seg2
        {
            if(P) std::cout << "seg1 lead-cross seg2. ";
            if(seg1_l < seg2_l)
            {
                *result++ = seg1_l;
                *result++ = seg2_l-1;
                *kr++ = k1;
                if(P) std::cout << "new seg(" << seg1_l << "," << seg2_l-1 << "). ";
            }
            *result++ = seg2_l;
            *result++ = seg1_r;
            *kr++ = k1 + k2;
            if(P) std::cout << "new seg(" << seg2_l << "," << seg1_r << ")\n";

            proceed1 = true;
            proceed2 = false;
            seg2_l = seg1_r+1;
            
        }
        else if(seg1_l >= seg2_l && seg1_l < seg2_r) //seg2 lead-intersec seg1
        {
            if(P) std::cout << "seg2 lead-cross seg1. ";
            if(seg2_l < seg1_l)
            {
                *result++ = seg2_l;
                *result++ = seg1_l-1;
                *kr++ = k2;
                if(P) std::cout << "new seg(" << seg2_l << "," << seg1_l-1 << "). ";
            }
            *result++ = seg1_l;
            *result++ = seg2_r;
            *kr++ = k1 + k2;
            if(P) std::cout << "new seg(" << seg1_l << "," << seg2_r << ")\n";

            proceed1 = false;
            seg1_l = seg2_r+1;
            proceed2 = true;
            
        }

        if(proceed1 && first1 == last1)
        {
            if(!proceed2)
            {
                *result++ = seg2_l;
                *result++ = seg2_r;
                *kr++ = k2;
                if(P) std::cout << "new seg(" << seg2_l << "," << seg2_r << ")\n";
            }
            result = std::copy(first2, last2, result);
            kr = std::copy(kf2, kl2, kr);
            break;
        }
        if(proceed2 && first2 == last2)
        {
            if(!proceed1)
            {
                *result++ = seg1_l;
                *result++ = seg1_r;
                *kr++ = k1;
                if(P) std::cout << "new seg(" << seg1_l << "," << seg1_r << ")\n";
            }
            result = std::copy(first1, last1, result);
            kr = std::copy(kf1, kl1, kr);
            break;
        }
    }


    nseg.erase(result, nseg.end());
    if(P) std::cout << "merged segment size = " << nseg.size() << "\n";

    nck.erase(kr, nck.end());
    if(P) std::cout << "merged key size = " << nck.size() << "\n";

    m_segments.clear();
    m_segments = nseg;

    m_keys.clear();
    m_keys = nck;

    return *this;
}
