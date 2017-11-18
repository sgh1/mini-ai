
#ifndef KD_UTIL_H_
#define KD_UTIL_H_

#include "kd_vector.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace kd
{

template<	typename T,
			unsigned N>
struct kd_query_result
{
    kd_query_result():
        m_query_id(0),
        m_nearest(0.0)
    {}

    kd_query_result(unsigned id, const kd_vector<T,N>& v ):
        m_query_id(id),
        m_nearest(v)
    {}

    std::string create_str() const
    {
        std::stringstream ss;
        ss << "Query" << m_query_id << "_" << m_nearest.m_index;
        return ss.str();
    }

	unsigned 		m_query_id;
	kd_vector<T,N>	m_nearest;
};


//read one of the provided 3D CSV files and return a
//vector with the points.
template<
    typename T,
    unsigned N>
std::vector< kd_vector<T,N> > read_vect3(const std::string& filename)
{
    using vect = kd::kd_vector<T,N>;

    std::vector<vect> pts;

    try
    {
        std::ifstream f(filename);
        std::string   line;
        unsigned      cnt = 0;

        while (std::getline(f, line))
        {
            std::replace( line.begin(), line.end(),',',' ');
            std::istringstream ss(line);

            vect pt = kd::read_vector_no_index<T,N>(ss);
            pt.m_index = cnt++;
            pts.push_back(pt);
        }
    }
    catch(...){
        std::cout << "error reading file\n";
    }

    return pts;
}

//some help for deserialization.  Probably not a high-
//performance solution.
template<typename T>
T get_line_and_read_second_str(std::istream& is)
{
    T t;
    std::string line,item;

    std::getline(is,line);
    std::stringstream ss(line);
    ss >> item >> t;

    return t;

}


}


#endif
