#ifndef _MULTIDIM_SBASIS
#define _MULTIDIM_SBASIS

#include "s-basis.h"
#include "point.h"

namespace Geom{

template <typename T>
class D2{
public:
    T f[2];
    
    T& operator[](unsigned i) {
        return f[i];
    }
    T const & operator[](unsigned i) const {
        return f[i];
    }

    unsigned size() const {
        unsigned s = f[0].size();
        for(unsigned i = 1; i < 2; i++)
            s = std::max(s, (unsigned) f[i].size());
        return s;
    }
    
    double tail_error(unsigned tail) const {
        double s = f[0].tail_error(tail);
        for(unsigned i = 1; i < 2; i++)
            s = std::max(s, f[i].tail_error(tail));
        return s;
    }

    bool is_finite() const {
        for(unsigned i = 0; i < 2; i++)
            if(!f[i].is_finite())
                return false;
        return true;
    }
};

template <typename T1, typename T2>
inline D2<T1>
operator +(D2<T1> const & a, D2<T2> const & b) {
    D2<T1> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i] + b.f[i];
    return r;
}

template <typename T1, typename T2>
inline D2<T1>
operator -(D2<T1> const & a, D2<T2> const & b) {
    D2<T1> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i] - b.f[i];
    return r;
}

template <typename T>
inline D2<T>
operator *(double a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a*b.f[i];
    return r;
}

template <typename T>
inline D2<T>
operator +=(D2<T> & a, D2<T> const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]+=b.f[i];
    return a;
}

template <typename T>
inline D2<T>
operator *=(D2<T> & a, double b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]*=b;
    return a;
}

/*template <typename T>
D2<T>
operator -=(D2<T> & a, D2<T> const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]-=b.f[i];
    return a;
}*/

inline D2<SBasis>
derivative(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

inline D2<SBasis>
integral(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <typename T>
inline SBasis
dot(D2<T> const & a, D2<T> const & b) {
    T r;
    for(unsigned i = 0; i < 2; i++)
        r += a.f[i] * b.f[i];
    return r;
}

//These two L2s are the same except the k param

inline SBasis
L2(D2<SBasis> const & a, int k) {
    double r;
    for(unsigned i = 0; i < 2; i++)
        r += multiply(a.f[i], a.f[i]);
    return sqrt(r,k);
}

inline double
L2(D2<double> const & a) {
    double r;
    for(unsigned i = 0; i < 2; i++)
        r += multiply(a.f[i], a.f[i]);
    return sqrt(r);
}

inline D2<SBasis>
multiply(BezOrd const & a, D2<SBasis> const & b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

template <typename T>
inline D2<T>
multiply(T const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

inline D2<SBasis>
operator*(BezOrd const & a, D2<SBasis> const & b) {
    return multiply(a, b);
}

template <typename T>
inline D2<T>
operator*(T const & a, D2<T> const & b) {
    return multiply(a, b);
}

/* Doesn't match composition
template <typename T>
inline D2<T>
compose(T const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a,b.f[i]);
    return r;
}
*/

//Only applies to func types, hard to encode this with templates
template <typename T>
inline D2<T>
compose(D2<T> const & a, T const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <typename T>
inline D2<T>
composeEach(D2<T> const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a.f[i],b.f[i]);
    return r;
}

inline D2<SBasis>
truncate(D2<SBasis> const & a, unsigned terms) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = truncate(a.f[i], terms);
    return r;
}

inline D2<T>
rot90(D2<T> const & a) {
    D2<T> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
}

inline D2<T>
cross(D2<T> const & a, D2<T> const & b) {
    D2<T> r;
    r[0] = (-1) * a.f[0] * b.f[1];
    r[1] = a.f[1] * b.f[0];
    return r;
}

//Only applies to (->Double) function types
template <typename T>
inline Point
operator()(D2<T> const & a, double t) {
    return Point(a[0](t), a[1](t);
}

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
