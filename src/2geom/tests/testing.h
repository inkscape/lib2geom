#include "gtest/gtest.h"
#include <vector>
#include <2geom/interval.h>

// streams out a vector
template <class T>
std::ostream&
operator<< (std::ostream &out, const std::vector<T,
             std::allocator<T> > &v)
{
    typedef std::ostream_iterator<T, char,
    std::char_traits<char> > Iter;

    std::copy (v.begin (), v.end (), Iter (out, " "));

    return out;
}

template <typename T, int xn>
std::vector<T> vector_from_array(const T (&x)[xn]) {
    std::vector<T> v;
    for(int i = 0; i < xn; i++) {
        v.push_back(x[i]);
    }
    return v;
}

template <typename T, int xn>
void expect_array(const T (&x)[xn], std::vector<T> y) {
    EXPECT_EQ(xn, y.size());
    for(unsigned i = 0; i < y.size(); i++) {
        EXPECT_EQ(x[i], y[i]);
    }
}

Geom::Interval bound_vector(std::vector<double> v) {
    double low = v[0];
    double high = v[0];
    for(unsigned i = 0; i < v.size(); i++) {
      low = std::min(v[i], low);
      high = std::max(v[i], high);
    }
    return Geom::Interval(low-1, high-1);
}

void vector_equal(std::vector<double> a, std::vector<double> b) {
    EXPECT_EQ(a.size(), b.size());
    if(a.size() != b.size()) return;
    for(unsigned i = 0; i < a.size(); i++) {
        EXPECT_FLOAT_EQ(a[i], b[i]);
    }
}

