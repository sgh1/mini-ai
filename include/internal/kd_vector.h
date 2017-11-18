
#ifndef KD_VECT_H_
#define KD_VECT_H_

#include <array>
#include <cassert>
#include <iomanip>
#include <iostream>

namespace kd
{

/***********************
*print_precision --
*a small trait to determine printing precision
************************/

template<typename T>
struct print_precision{
    static const int value = 4;
};

template<>
struct print_precision<double>{
    static const int value = 16;
};

template<>
struct print_precision<float>{
    static const int value = 7;
};

/***********************
*kd_vector --
*an N-dimensional vector type based on std::array
************************/

template<
	typename T,
	unsigned N
>
class kd_vector
{
    public:

    typedef T value_type;

    kd_vector():
        m_data()
    {}

    kd_vector(const std::array<T,N>& v):
        m_data(v),
        m_index(0)
    {}

    kd_vector(const std::array<T,N>& v, unsigned idx):
        m_data(v),
        m_index(idx)
    {}

	kd_vector(const kd_vector& v) = default;
    ~kd_vector() = default;

    const T& operator[](unsigned idx) const{
        return m_data[idx];
    }

    T& operator[](unsigned idx){
        return m_data[idx];
    }

    size_t size() const{
        return m_data.size();
    }

    void fill(const T& t){
        m_data.fill(t);
    }

    std::array<T,N> m_data;
    unsigned		m_index;

};

//compute the distance squared between any V1 and V2
template<typename V1, typename V2>
auto dist2( const V1& a, const V2& b){

    assert(a.size() == b.size());

    auto sum = 0.0;
    for(size_t i = 0; i < a.size(); i++){
        sum += (a[i]-b[i])*(a[i]-b[i]);
    }

    return sum;
}

//operator<< and operator>> should deal with the whole
//class, but we need something to just read data, as in the
//provided CSV files, perhaps, a different type is needed
template <typename T, unsigned N>
kd_vector<T,N> read_vector_no_index(std::istream& is)
{
    kd_vector<T,N> v;

    for(T& t : v.m_data){
        is >> t;
    }

    return v;
}


//need these in the namespace

template <typename T, unsigned N>
std::ostream& operator<<(std::ostream& os, const kd_vector<T,N>& v){

    for(const T& t : v.m_data){
        os << t << " ";
    }

    os << v.m_index;

    return os;
}

template <typename T, unsigned N>
std::istream& operator>>(std::istream& is, kd::kd_vector<T,N>& v){

    for(T& t : v.m_data){
        is >> t;
    }

    is >> v.m_index;

    return is;
}

}


#endif
