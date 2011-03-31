#ifndef COLORKEY_H
#define COLORKEY_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream>

class ColorKey
{
public:
    ColorKey()
    { 
        m_val = new unsigned int[keyLen]; 
        memset(m_val, 0, keyLen*sizeof(int)); 
    }

    ColorKey(unsigned int k, unsigned int pos)
    {
        m_val = new unsigned int[keyLen]; 
        memset(m_val, 0, keyLen*sizeof(int));
        assert(pos >=0 && pos < keyLen);
        m_val[pos] = k;
    }

    ColorKey(unsigned int* key)
    {
        m_val = new unsigned int[keyLen]; 
        memcpy(m_val, key, keyLen*sizeof(int));
    }
    ColorKey(const ColorKey& K)
    {
        m_val = new unsigned int[keyLen];
        if(m_val == 0){
          std::cout << __FILE__ << ":" << __func__ << "\n";
          exit(1);
        }
        memcpy(m_val, K.get_key(), keyLen*sizeof(int));
    }

    ~ColorKey(){ delete[] m_val; }

    const unsigned int* get_key() const
    {
        return m_val;
    }
    //return a reference to m_val, can be changed, function is not const
    unsigned int& operator[](unsigned int i) { assert(i>=0 && i<keyLen); return m_val[i]; }
    unsigned int get_key(int i) const {return m_val[i];}
    void set_key(int i, unsigned int k) {m_val[i] = k;}

    bool operator< (const ColorKey& rhs) const
    {
        bool ret = false;
        for(unsigned int i=0; i<keyLen; i++)
        {
            if(m_val[i] < rhs.get_key(i)){ ret = true; break; }
        }
        return ret;
    }

    ColorKey operator+ (const ColorKey& rhs) const
    {
        ColorKey nk;
        const unsigned int* k = rhs.get_key();
        for(unsigned int i=0; i<keyLen; i++)
        {
            nk.set_key(i, m_val[i] + k[i]);
        }
        return nk;
    }

    ColorKey& operator= (const ColorKey& rhs)
    {
        if(&rhs != this)
            memcpy(m_val, rhs.get_key(), keyLen*sizeof(int));
        return *this;
    }
    //void print() const
    friend std::ostream& operator<< (std::ostream& os, ColorKey& ck)
    {
        os << "[";
        for(unsigned int i=0; i<keyLen; i++) os << ck.m_val[i] << ",";
        os << "]";

        return os;
    }
    friend bool operator== (const ColorKey& lhs, const ColorKey& rhs)
    {
        for(unsigned int i=0; i<keyLen; i++) 
        {
            if(lhs.m_val[i] != rhs.m_val[i]) return false;
        }
        return true;
    }

    static const unsigned int keyLen;   

private:

    unsigned int *m_val;
};


#endif
