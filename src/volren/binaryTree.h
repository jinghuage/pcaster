#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include <string>
#include <vector>
#include <iostream>

#include "frustumCull.h"

template <class NodeData> class binaryTree
{
public:
    typedef binaryTree<NodeData> Node;

    //data copy constructor
    binaryTree(const NodeData& D) : m_nodebase(D) 
    {
        m_leafCount = 1;
        m_rank = -1;
        m_level = 0;  
        m_parent = 0;
        m_sibling = 0;
        m_child[0] = m_child[1] = 0;     
    }
    
    binaryTree() 
    {
        m_leafCount = 1;
        m_rank = -1;
        m_level = 0;
        m_parent = 0;
        m_sibling = 0;
        m_child[0] = m_child[1] = 0;
    }
    
    ~binaryTree() {}

    //return member reference
    NodeData& get_nodebase() { return m_nodebase;}
    

    void split_node()
    {
        //std::cout << "split node:";
        //print();

        if(!m_nodebase.howto_split()) return;
       

        for(int i=0; i<2; i++)
        {
            m_child[i] = new Node(m_nodebase); 
            NodeData& nbase = m_child[i]->get_nodebase();
            nbase.split_data(i);

            m_child[i]->set_level(m_level+1);
            m_child[i]->set_parent(this);
        }

        //update leaf count        
        increase_leafcount();

        m_child[0]->set_sibling(m_child[1]);
        m_child[1]->set_sibling(m_child[0]);
        
        //std::cout << "to child nodes:";
        //m_child[0]->print();
        //m_child[1]->print();

        //std::cout << "\n";
    }

    Node *get_child(int n){return m_child[n];}
    Node *get_parent(){return m_parent;}
    Node *get_sibling(){return m_sibling; }
    void set_parent(Node* t) {m_parent = t; }
    void set_sibling(Node* t) {m_sibling = t; }

    int get_level() { return m_level; }
    void set_level(int l) { m_level = l; }
    bool is_leaf() 
    {
        return (m_child[0] == 0 && m_child[1] == 0);
    }

    bool is_root()
    {
        return (m_parent == 0);
    }

    void set_rank(int rank){ m_rank = rank; }
    int get_rank(){ return m_rank; }

    int  get_leafcount() { return m_leafCount; }
    
    void print_node()
    {
        std::cout << " rank:" << m_rank;
        std::cout << " level:" << m_level;
        std::cout << " leafcout:" << m_leafCount;
    }


    //recursive operations

    //return the whole tree size
    int get_wholetreesize()
    {
        if(m_parent == 0) return m_leafCount;
        else return m_parent->get_wholetreesize();
    }


    void increase_leafcount()
    {
        m_leafCount ++;
        if(m_parent == 0) return;
        else m_parent->increase_leafcount();
    }

    //pass the rank for root and rank the whole tree
    void rank_tree(int rank)
    {
        set_rank(rank);

        if(is_leaf()) return;
        else
        {
            int leap_l = m_child[0]->get_leafcount();

            m_child[0]->rank_tree(rank);
            m_child[1]->rank_tree(rank+leap_l);
        }
    }

    void unrank_tree()
    {
        set_rank(-1);

        if(is_leaf()) return;
        else{
        m_child[0]->unrank_tree();
        m_child[1]->unrank_tree();
        }
    }

    
    void print_tree()
    {
        print_node();
        m_nodebase.print();
        
        if( is_leaf() ) return;
        else
        {
            m_child[0]->print_tree();
            m_child[1]->print_tree();
        }
    }

    void retrieve_nodedata(std::vector<NodeData*>& nodelist)
    {
        if(is_leaf()) nodelist.push_back(&m_nodebase);
        else
        {
            m_child[0]->retrieve_nodedata(nodelist);
            m_child[1]->retrieve_nodedata(nodelist);
        }
    }

/*
    void retrieve_nodedata(std::vector<NodeData*>& nodelist, int sortarray[] )
    {
        if(is_leaf()) nodelist[sortarray[m_rank]] = &m_nodebase;
        else
        {
            m_child[0]->retrieve_nodedata(nodelist, sortarray);
            m_child[1]->retrieve_nodedata(nodelist, sortarray);
        }
    }


    void retrieve_nodedata_byrank(int rank, std::vector<NodeData*>& node)
    {
        if(is_leaf() && m_rank == rank) node.push_back(&m_nodebase);
        else
        {
            int leap_l = m_child[0]->get_leafcount();

            if(rank >= m_rank && rank < m_rank + leap_l)
                m_child[0]->retrieve_nodedata_byrank(rank, node);
            else if(rank >= m_rank + leap_l && rank < m_rank + m_leafCount)
                m_child[1]->retrieve_nodedata_byrank(rank, node);
            else
            {
                printf("%d: can't retrieve rank %d 's treenode\n", m_rank, rank);
            }
        }
    }


    void retrieve_nodedata_byitem(int item_id, 
                                  std::vector<NodeData*>& nodelist)
    {
        if(is_leaf() )
        {
            if(m_nodebase.contain_item(item_id))
            {
                std::cout << "screen node " << m_rank << " contain item " << item_id << "\n";
                nodelist.push_back(&m_nodebase);
                //ranklist.push_back(m_rank);
                return;
            }
            else
            {
                std::cout << "screen node " << m_rank << " doesn't contain item " << item_id << "\n";
                return;
            }
        }
        else
        {
            m_child[0]->retrieve_nodedata_byitem(item_id, nodelist);
            m_child[1]->retrieve_nodedata_byitem(item_id, nodelist);
        }
    }
*/

    //draw non_leaf node's cut process
    void draw_treeCut(int maxLevel)
    {
        if( is_leaf() || m_level > maxLevel) return;
        else
        {
            m_nodebase.draw_cut();
            m_child[0]->draw_treeCut(maxLevel);
            m_child[1]->draw_treeCut(maxLevel);
        }
    }

    //draw leaf nodes and their items
    void draw_tree()
    {
        if( is_leaf() ) m_nodebase.draw();
        else
        {
            m_child[0]->draw_tree();
            m_child[1]->draw_tree();
        }
    }

    //used to have an arguemnt maxleafnode prevents the tree from growing too big
    void build_tree()
    {
        //int wholetreesize = get_wholetreesize();
        if( m_nodebase.apply_condition() ) return;
        else
        {
            split_node(); //this could end the tree building process as well
            if(m_child[0]) m_child[0]->build_tree();
            if(m_child[1]) m_child[1]->build_tree();
        }
    }

    //delete the whole tree
    void remove_tree()
    {        
        if( is_leaf() ) {delete this; return;}
        else
        {
            m_child[0]->remove_tree();
            m_child[1]->remove_tree();
        }
        delete this;
    }

/*
    void collect_tree(std::vector<int>& clist)
    {
        
        if( is_leaf() )
        {
            m_nodebase.set_collector(clist);
            return;
        }
        else
        {
            m_child[0]->collect_tree(clist);
            m_child[1]->collect_tree(clist);
        }
    }

    void analyze_tree(float* stat_array, int* root_array, int dim)
    {
        
        if( is_leaf() )
        {
            m_nodebase.fillin_statistics(stat_array + m_rank*dim, 
                                         root_array + m_rank,
                                         dim);
            return;
        }
        else
        {
            m_child[0]->analyze_tree(stat_array, root_array, dim);
            m_child[1]->analyze_tree(stat_array, root_array, dim);
        }
    }

    void analyze_tree_2(float* send_array, int dim)
    {
        
        if( is_leaf() )
        {
            m_nodebase.fillin_statistics_2(send_array, 
                                           dim);
            return;
        }
        else
        {
            m_child[0]->analyze_tree_2(send_array, dim);
            m_child[1]->analyze_tree_2(send_array, dim);
        }
    }
*/

    void sort_tree(CFrustumCull* fculler, int sortId, int sortArray[])
    {
        if( is_leaf() )
        {
            sortArray[m_rank] = sortId;
            return;
        }
        else
        {
            int leap_l = m_child[0]->get_leafcount();
            int leap_r = m_child[1]->get_leafcount();

            int cut_direction = m_nodebase.get_split_direction();
            float cut_position = m_nodebase.get_split_position();

            bool f = fculler->front(cut_direction, cut_position);
            if(f)
            {
                m_child[0]->sort_tree(fculler, sortId, sortArray);
                m_child[1]->sort_tree(fculler, sortId+leap_l, sortArray);
            }
            else
            {
                m_child[0]->sort_tree(fculler, sortId+leap_r, sortArray);
                m_child[1]->sort_tree(fculler, sortId, sortArray);
            }
        }
    }


private:

    int m_rank;
    int m_level;
    int m_leafCount;

    Node *m_parent;
    Node *m_child[2];
    Node *m_sibling;

    NodeData m_nodebase;

};





#endif
