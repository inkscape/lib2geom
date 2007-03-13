#ifndef _MULTIDIM_SBASIS
#define _MULTIDIM_SBASIS

#include "s-basis.h"
#include "point.h"

namespace Geom{

template <typename T, unsigned D>
class Multidim{
public:
    T f[D];
    
    T& operator[](unsigned i) {
        return f[i];
    }
    T const & operator[](unsigned i) const {
        return f[i];
    }

    unsigned size() const {
        unsigned s = f[0].size();
        for(unsigned i = 1; i < D; i++)
            s = std::max(s, (unsigned) f[i].size());
        return s;
    }
    
    double tail_error(unsigned tail) const {
        double s = f[0].tail_error(tail);
        for(unsigned i = 1; i < D; i++)
            s = std::max(s, f[i].tail_error(tail));
        return s;
    }

    bool is_finite() const {
        for(unsigned i = 0; i < D; i++)
            if(!f[i].is_finite())
                return false;
        return true;
    }
};

template <typename T1, typename T2, unsigned D>
inline Multidim<T1, D>
operator +(Multidim<T1, D> const & a, Multidim<T2, D> const & b) {
    Multidim<T1, D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i] + b.f[i];
    return r;
}

template <typename T1, typename T2, unsigned D>
inline Multidim<T1, D>
operator -(Multidim<T1, D> const & a, Multidim<T2, D> const & b) {
    Multidim<T1, D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i] - b.f[i];
    return r;
}

template <typename T, unsigned D>
inline Multidim<T, D>
operator *(double a, Multidim<T, D> const & b) {
    Multidim<T, D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a*b.f[i];
    return r;
}

template <typename T, unsigned D>
inline Multidim<T, D>
operator +=(Multidim<T, D> & a, Multidim<T, D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]+=b.f[i];
    return a;
}

template <typename T, unsigned D>
inline Multidim<T, D>
operator *=(Multidim<T, D> & a, double b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]*=b;
    return a;
}

/*template <typename T, unsigned D>
Multidim<T, D>
operator -=(Multidim<T, D> & a, Multidim<T, D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]-=b.f[i];
    return a;
}*/

template <unsigned D>
inline Multidim<SBasis, D>
derivative(Multidim<SBasis, D> const & a) {
    Multidim<SBasis, D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

template <unsigned D>
inline Multidim<SBasis, D>
integral(Multidim<SBasis, D> const & a) {
    Multidim<SBasis, D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
dot(Multidim<SBasis, D> const & a, Multidim<SBasis, D> const & b) {
    T r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i], b.f[i]);
    return r;
}

//These two L2s are the same except the k param

template <unsigned D>
inline SBasis
L2(Multidim<SBasis, D> const & a, int k) {
    double r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i], a.f[i]);
    return sqrt(r,k);
}

template <unsigned D>
inline double
L2(Multidim<double, D> const & a) {
    double r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i], a.f[i]);
    return sqrt(r);
}

template <unsigned D>
inline Multidim<SBasis, D>
multiply(BezOrd const & a, Multidim<SBasis, D> const & b) {
    Multidim<SBasis, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

template <typename T, unsigned D>
inline Multidim<T, D>
multiply(T const & a, Multidim<T, D> const & b) {
    Multidim<T, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

template <unsigned D>
inline Multidim<SBasis, D>
operator*(BezOrd const & a, Multidim<SBasis, D> const & b) {
    return multiply(a, b);
}

template <typename T, unsigned D>
inline Multidim<T, D>
operator*(T const & a, Multidim<T, D> const & b) {
    return multiply(a, b);
}

/* Doesn't match composition
template <typename T, unsigned D>
inline Multidim<T, D>
compose(T const & a, Multidim<T, D> const & b) {
    Multidim<T, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a,b.f[i]);
    return r;
}
*/

//Only applies to func types, hard to encode this with templates
template <typename T, unsigned D>
inline Multidim<T, D>
compose(Multidim<T, D> const & a, T const & b) {
    Multidim<T, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <typename T, unsigned D>
inline Multidim<T, D>
composeEach(Multidim<T, D> const & a, Multidim<T, D> const & b) {
    Multidim<T, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline Multidim<SBasis, D>
truncate(Multidim<SBasis, D> const & a, unsigned terms) {
    Multidim<SBasis, D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = truncate(a.f[i], terms);
    return r;
}

inline Multidim<T, 2>
rot90(Multidim<T, 2> const & a) {
    Multidim<T, 2> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
}

inline Multidim<T, 2>
cross(Multidim<T, 2> const & a, Multidim<T, 2> const & b) {
    Multidim<T, 2> r;
    r[0] = (-1) * a.f[0] * b.f[1];
    r[1] = a.f[1] * b.f[0];
    return r;
}

//Only applies to (->Double) function types
template <typename T>
inline Point
operator()(Multidim<T, 2> const & a, double t) {
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
