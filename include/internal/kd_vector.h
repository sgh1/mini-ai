
#ifndef KD_VECT_H_
#define KD_VECT_H_

#include "miniai_error.h"

#include <array>

namespace miniai
{

// Until we need something more, we wil just use an alias to std::array
// as an nd vector.

using kd_vector = std::array<T, N>;

//
// Some free functions for kd_vector and similar types.
//


// Compute the distance squared between any V1 and V2
template<typename V1, typename V2>
auto dist2( const V1& a, const V2& b)
{
    auto a_size = a.size();

    // Check sizes.
    if(a.size() != b.size()){
        miniai_error::error("Sizes of vectors are not equal!");
    }
    
    // Compute distance squared.
    auto sum = 0.0;
    for(size_t i = 0; i < a_size; i++)
    {
        auto diff = (a[i]-b[i]); 
        sum += diff*diff;
    }

    return sum;
}

// Compute the distance between any V1 and V2
template<typename V1, typename V2>
auto dist( const V1& a, const V2& b)
{
    return std::sqrt( dist2(a, b));
}

}

#endif
