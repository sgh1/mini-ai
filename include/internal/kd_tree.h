
#ifndef KD_TREE_H_
#define KD_TREE_H_

#include "kd_params.h"
#include "kd_util.h"
#include "kd_vector.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


namespace kd
{

/***********************
*kd_tree_node --
*a node in the kd tree
*
*template parameters:
*T - a vector type of size N.
*N - dimension of tree
*KD_PARAMS - a type which implements a few functions to
* help determine how to create new children, and how to
* stop tree creation.
************************/

template<
    typename T,
    unsigned N,
    typename KD_PARAMS = kd_params_default<
        typename T::value_type,N,1> >
class kd_tree_node
{

    public:

    	using num_type = 	typename T::value_type;
		using kd_vect = 	kd_vector<num_type,N>;
		using split_inf	=   kd_split_info<num_type>;

        kd_tree_node(kd_tree_node* parent):
			m_data(				),
			m_parent(		parent),
			m_child_split(      ),
			m_child_neg(	nullptr),
			m_child_pos(	nullptr),
			m_handle(		m_node_count++),
			m_level(		0)
        {
			if(m_parent){
				m_level = m_parent->m_level+1;
			}
		}

        ~kd_tree_node(){
            m_node_count -= 1;
            //std::cout << m_node_count << "\n";
        }

        kd_tree_node(const kd_tree_node& other) = delete;
        kd_tree_node(kd_tree_node&& other) = delete;

        //recursively create child nodes
        void create_children(){

            //decide if we're done building this branch
            if(!KD_PARAMS::need_children(m_data.size(),m_level)){
				return;
			}

            //get new pivot dim. and coord.
			split_inf splt = KD_PARAMS::get_split_dim(m_data, m_level+1);
            m_child_split = splt;

			unsigned	split_d = splt.m_dim;
			num_type	split_pt = splt.m_coord;

			m_child_neg.reset( new kd_tree_node(this) );
			m_child_pos.reset( new kd_tree_node(this) );

			if(!m_child_neg || !m_child_pos)
				return;

            //move the data to child nodes
			for(T& t : m_data)
			{
				if( t[split_d] <= split_pt){
					m_child_neg->add_data(std::move(t));
				}
				else if( t[split_d] > split_pt ){
					m_child_pos->add_data(std::move(t));
				}
			}

			m_data.clear();

			m_child_neg->create_children();
			m_child_pos->create_children();
		}

        //recursive nearest neighbor algorithm from
        //http://www.autonlab.org/autonweb/14665/version/2/part/5/data/moore-tutorial.pdf

        //this could use a little work
        //we want this to be const, but it's tricky.
        //also, should x be a kd_vect, or a T?
		void nearest_neighbor(const kd_vect& x, T* nearest, num_type& dist_squared) //const
		{

            //find nearest datum in this node
            for( T& t : m_data)
            {
                num_type d2 = dist2(x,t);
                if(d2 < dist_squared){
                    dist_squared = d2;
                    nearest = t;
                }
            }

            //pop
            if(is_leaf()){
                return;
            }

            assert( m_child_neg && m_child_pos);

            kd_tree_node* containing_node = nullptr;
            kd_tree_node* other_node = nullptr;

            if(x[m_child_split.m_dim] <= m_child_split.m_coord){
                containing_node = m_child_neg.get();
                other_node = m_child_pos.get();
            }
            else{
                containing_node = m_child_pos.get();
                other_node = m_child_neg.get();
            }

            //descend into node containing test point, x
            containing_node->nearest_neighbor(x,nearest,dist_squared);

            //if split plane is closer than dist to nearest point so far,
            //check that node also
            num_type other_dist = x[m_child_split.m_dim] - m_child_split.m_coord;
            other_dist *= other_dist;

            if(other_dist < dist_squared){
                other_node->nearest_neighbor(x,nearest,dist_squared);
            }

            return;
        }

		//const T& nearest_neighbor_approximate(const kd_vect& x, num_type& dist_squared) const
		//can't reseat const T&
		//void nearest_neighbor(const kd_vect& x, T* nearest, num_type& dist_squared) //const
        //can't do that'

		T nearest_neighbor_approximate(const kd_vect& x, num_type& dist_squared) const
		{

            const T* nearest = &m_data[0];

            //find nearest datum in this node
            for( const T& t : m_data)
            {
                num_type d2 = dist2(x,t);
                if(d2 < dist_squared){
                    dist_squared = d2;
                    nearest = &t;
                }
            }

            //pop
            if(is_leaf()){
                return T(*nearest);
            }

            kd_tree_node* containing_node = nullptr;

            if(x[m_child_split.m_dim] <= m_child_split.m_coord){
                containing_node = m_child_neg.get();
            }
            else{
                containing_node = m_child_pos.get();
            }

            //descend into node containing test point, x
            containing_node->nearest_neighbor_approximate(x,dist_squared);

            return T(*nearest);
        }

        friend std::ostream& operator<<(std::ostream& os, const kd_tree_node& node)
        {
            os << "node: " << node.m_handle << "\n";
            os << "leaf: " << node.is_leaf() << "\n";

            if(!node.is_leaf()){
                os << "split_dim: " << node.m_child_split.m_dim << "\n";
                os << "split_coord: " << node.m_child_split.m_coord << "\n";
            }

            os << "data:\n";

            for(const T& t : node.m_data){
                os << t << "\n";
            }

            os << "###\n";

            if(node.m_child_neg){
                os << *node.m_child_neg;
            }

            if(node.m_child_pos){
                os << *node.m_child_pos;
            }

            return os;

        }

        friend std::istream& operator>>(std::istream& is, kd_tree_node& node)
        {
            std::string item;
            std::string line;
            std::stringstream ss;

            node.m_handle = get_line_and_read_second_str<unsigned>(is);
            bool is_leaf = get_line_and_read_second_str<bool>(is);

            if(!is_leaf)
            {
                node.m_child_split.m_dim =
                    get_line_and_read_second_str<unsigned>(is);
                node.m_child_split.m_coord =
                    get_line_and_read_second_str<num_type>(is);
            }

            std::getline(is,line); //data:
            std::getline(is,line);

            while(line != "###")
            {
                T t;
                ss = std::stringstream(line);
                ss >> t;

                node.m_data.push_back(std::move(t));
                std::getline(is,line);
            }

            if(is_leaf){
                return is;
            }

            node.m_child_neg.reset( new kd_tree_node(&node) );
			node.m_child_pos.reset( new kd_tree_node(&node) );

			is >> *node.m_child_neg;
			is >> *node.m_child_pos;

            return is;
        }

        //todo: combine these, U&&, std::forward
		void add_data(const T& t){
			m_data.push_back(t);
		}

		void add_data(T&& t){
			m_data.push_back(std::move(t));
		}

		bool is_leaf() const{
			return !(m_child_neg || m_child_pos);
		}

        //not limited to any one tree!
        static unsigned m_node_count;

    private:

        std::vector<T>          m_data;

        //raw const pointer, but I think the ownership
        //is clear enough, ie, not us.
        const kd_tree_node*     m_parent;

        split_inf		                m_child_split;
        std::unique_ptr<kd_tree_node>   m_child_neg;
        std::unique_ptr<kd_tree_node>   m_child_pos;

        unsigned        m_handle;
        unsigned 		m_level;
};

template<
    typename T,
    unsigned N,
    typename KD_PARAMS
>
unsigned kd_tree_node<T,N,KD_PARAMS>::m_node_count = 0;


/***********************
*kd_tree --
*this class holds the root node of the kd_tree, and provides the
*interface. Really, kd_node should have been def'd within kd_tree.
*It also helps us wrap up some of the recursion nicely.
************************/

template<
    typename T,
    unsigned N,
    typename KD_PARAMS = kd_params_default<
        typename T::value_type,N,1> >
class kd_tree
{
	public:

	using num_type = 	typename T::value_type;
	using kd_vect = 	kd_vector<num_type,N>;
    using kd_node =     kd_tree_node<T,N,KD_PARAMS>;

    kd_tree():
        m_root(nullptr)
    {}

	kd_tree(const std::vector<T>& data):
		m_root(new kd_node(nullptr))
	{
		if(m_root){
            for( const T& t : data){
                m_root->add_data(t);
            }

			m_root->create_children();
		}
	}

    kd_tree(const kd_tree& other) = delete;
    kd_tree(kd_tree&& other) = delete;
    ~kd_tree() = default;

    friend std::ostream& operator<<(std::ostream& os, const kd_tree& tree)
    {
        os << std::setprecision(print_precision<num_type>::value);
        return os << *tree.m_root;
    }

    //by virtue of reset, this will delete an old tree, if
    //m_root is not nullptr
    friend std::istream& operator>>(std::istream& is, kd_tree& tree)
    {
        tree.m_root.reset(new kd_node(nullptr) );
        return is >> *tree.m_root;
    }

	const T& nearest_neighbor(const kd_vect& x) const
	{
        T       nearest;
        num_type min_dist2 = std::numeric_limits<num_type>::max();

        m_root->nearest_neighbor(x,nearest,min_dist2);

        //assert(nearest != nullptr);
        return nearest;
	}

	T nearest_neighbor_approximate(const kd_vect& x) const
	{
        T       nearest;
        num_type min_dist2 = std::numeric_limits<num_type>::max();

        nearest = m_root->nearest_neighbor_approximate(x,min_dist2);

        //assert(nearest != nullptr);
        return nearest;
	}




	private:

	std::unique_ptr<kd_node>	m_root;

};

}


#endif
