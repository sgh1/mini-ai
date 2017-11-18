
#ifndef KD_PARAMS_H_
#define KD_PARAMS_H_

#include "kd_params.h"
#include "kd_vector.h"

#include <cassert>
#include <limits>
#include <vector>

namespace kd
{

/***********************
*kd_split_info --
*represents the axis and pivot by which a node's children are created
************************/

template<typename T>
struct kd_split_info
{
    kd_split_info():
        m_dim(  std::numeric_limits<unsigned>::max()),
        m_coord(0.0)
    {}

	unsigned 	m_dim;
	T	        m_coord;
};

/***********************
*kd_params_default --
*the default type to be used as KD_PARAMS for the kd_tree. Any type
*that defines the two functions in the default type below can be
*used for KD_PARAMS.  This way, we can easy change the way the next
*split axis and coordinate is chosen, and the stopping criterion
*for tree creation
************************/

template<
	typename    T,
	size_t      N,
	size_t      MAX_LEAF_DATA
	>
class kd_params_default
{
	public:

    using split_inf	=   kd_split_info<T>;
    using kd_vect =     kd_vector<T,N>;

    //keep building the tree if data_sz is > the
    //specified MAX_LEAF_DATA
	static bool need_children(size_t data_sz, size_t level){

		if(data_sz > MAX_LEAF_DATA){
			return true;
		}

		return false;
	}

    //select split dim as dimension with largest span
    //select split coord as median in that dim.
	static split_inf get_split_dim(
		const std::vector<kd_vect>& data, int level){

        split_inf ret;

		std::array<T,N> min_vals;
		std::array<T,N> max_vals;

		min_vals.fill(std::numeric_limits<T>::max() );
		max_vals.fill(std::numeric_limits<T>::min() );

		for( const kd_vect& t : data)
		{
			for(size_t i = 0 ; i < N; i++){
				min_vals[i] = std::min( min_vals[i] , t[i] );
				max_vals[i] = std::max( max_vals[i] , t[i] );
			}
		}

		ret.m_coord = std::numeric_limits<T>::min();
		for(size_t i = 0; i < N; i++){

			if(max_vals[i] - min_vals[i] > ret.m_coord){
				ret.m_dim = i;
				ret.m_coord = (max_vals[i] + min_vals[i])*0.5;
			}
		}

		assert(ret.m_dim < N);
		return ret;
	}
};


}


#endif
