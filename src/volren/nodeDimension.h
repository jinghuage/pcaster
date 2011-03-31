#ifndef NODE_DIMENSION_H_
#define NODE_DIMENSION_H_

#include <string>
#include <vector>

//------------------------------------------------------------------
//NodeBase for a kdtree (m_k = k)
//how to split: cutting plane with direction and position
//stop condition: scale threshold
//nodeItem: None
//------------------------------------------------------------------

class NodeDimension
{
public:
    NodeDimension();
    NodeDimension(int, float);
    //copy constructor.
//default shallow copy constructor should be ok
/*     NodeDimension(const NodeDimension&); */

    virtual ~NodeDimension();

protected:
    int m_k;
    int m_direction;
    float m_ratio;
    float m_scale_thres;
    std::vector<float> m_scale;
    std::vector<float> m_pos;

    void init();

public:
    void scale(std::vector<float>&);
    void translate(std::vector<float>&);

    void kd_split(int);

    const float* get_pos() { return &(m_pos.front()); }
    const float* get_scale() { return &(m_scale.front()); }

    int get_dimension() {return m_k; }
    int get_split_direction() { return m_direction; }
    float get_split_position() 
    { 
        return m_pos[m_direction] + (m_ratio-0.5) * m_scale[m_direction]; 
    }

    friend std::ostringstream& operator<< (std::ostringstream&, 
                                           const NodeDimension&);


    virtual void split_data(int);
    virtual void draw(); //draw the node items
    virtual void draw_cut();
    virtual void print();
    virtual bool apply_condition(); //should be further splited or not
    virtual bool howto_split(); //determine how to split, the cutting plane
};


#endif
