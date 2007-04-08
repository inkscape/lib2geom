#ifndef _2GEOM_D2
#define _2GEOM_D2

#include "point.h"

//#include <boost/concept_check.hpp>

//using namespace boost;

namespace Geom{

/*template <class T>
struct AlgGroupConcept {
    T i, j;
    void constraints() {
        i += j; i = i + j;
        i -= j; i = i - j;
    }
};

template <class T>
struct AlgRingConcept {
    T i, j;
    void constraints() {
        i *= j; i = i * j;
    }
};

template <class T>
struct NegateableConcept {
    T i;
    void constraints() {
        i = -i;
    }
};

template <class T>
struct ScaleableConcept {
    T i;
    double j;
    void constraints() {
        i *= j; i = j * i;
    }
};*/

template <class T>
class D2{
public:
    //BOOST_CLASS_REQUIRE(T, boost, AssignableConcept);

    T f[2];
    
    D2() {}

    D2(T const &a, T const &b) {
        f[0] = a;
        f[1] = b;
    }

    T& operator[](unsigned i)              { return f[i]; }
    T const & operator[](unsigned i) const { return f[i]; }

    Point operator()(double t) const;
    Point operator()(double x, double y) const;
};

template <typename T>
inline D2<T>
operator+(D2<T> const &a, D2<T> const &b) {
    //function_requires<AlgGroupConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + b[i];
    return r;
}

template <typename T>
inline D2<T>
operator-(D2<T> const &a, D2<T> const &b) {
    //function_requires<AlgGroupConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] - b[i];
    return r;
}

template <typename T>
inline D2<T>
operator+=(D2<T> &a, D2<T> const &b) {
    //function_requires<AlgGroupConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] += b[i];
    return a;
}

/*
template <typename T>
inline D2<T>
operator-=(D2<T> &a, D2<T> const & b) {
    //function_requires<AlgGroupConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] -= v[i];
    return a;
}*/

template <typename T>
inline D2<T>
operator*=(D2<T> &a, double v) {
    //function_requires<ScaleableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] *= v;
    return a;
}

template <typename T>
inline D2<T>
operator*(double a, D2<T> const & b) {
    //function_requires<ScaleableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a * b[i];
    return r;
}

template <typename T>
inline T
dot(D2<T> const & a, D2<T> const & b) {
    //function_requires<AlgGroupConcept<T> >();
    //function_requires<AlgRingConcept<T> >();

    T r;
    for(unsigned i = 0; i < 2; i++)
        r += a[i] * b[i];
    return r;
}

/* Doesn't match composition
template <typename T>
inline D2<T>
compose(T const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a,b[i]);
    return r;
}
*/

template <typename T>
inline D2<T>
rot90(D2<T> const & a) {
    //function_requires<NegateableConcept<T> >();

    D2<T> r;
    r[0] = -a[1];
    r[1] = a[0];
    return r;
}

template <typename T>
inline D2<T>
cross(D2<T> const & a, D2<T> const & b) {
    //function_requires<NegateableConcept<T> >();
    //function_requires<AlgRingConcept<T> >();

    D2<T> r;
    r[0] = -a[0] * b[1];
    r[1] = a[1] * b[0];
    return r;
}

//Following only apply to func types, hard to encode this with templates
template <typename T>
inline D2<T>
compose(D2<T> const & a, T const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b);
    return r;
}

template <typename T>
inline D2<T>
composeEach(D2<T> const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b[i]);
    return r;
}

template<typename T>
inline Point
D2<T>::operator()(double t) const {
    Point p;
    for(int i = 0; i < 2; i++)
       p[i] = (*this)[i](t);
    return p;
}

template<typename T>
inline Point
D2<T>::operator()(double x, double y) const {
    Point p;
    for(int i = 0; i < 2; i++)
       p[i] = (*this)[i](x, y);
    return p;
}

};

//SBasis specific decls:

#include "s-basis.h"
#include "s-basis-2d.h"
#include "pw.h"

namespace Geom {

inline D2<SBasis> compose(D2<SBasis> const & a, SBasis const & b) {
    D2<SBasis> r;
    for(int i = 0; i < 2; i++)
        r[i] = compose(a[i], b);
    return r;
}

D2<SBasis> derivative(D2<SBasis> const & a);
D2<SBasis> integral(D2<SBasis> const & a);

SBasis L2(D2<SBasis> const & a, int k);
double L2(D2<double> const & a);

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b);
D2<SBasis> operator*(Linear const & a, D2<SBasis> const & b);
D2<SBasis> operator+(D2<SBasis> const & a, Point b);
D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b);
D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms);

unsigned sbasisSize(D2<SBasis> const & a);
double tailError(D2<SBasis> const & a, unsigned tail);
bool isFinite(D2<SBasis> const & a);

vector<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a, vector<double> &cuts);

class Rect;

Rect local_bounds(D2<SBasis> const & s, double t0, double t1, int order=0);

};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
#endif
