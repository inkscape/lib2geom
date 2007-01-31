#ifndef _MULTIDIM_SBASIS
#define _MULTIDIM_SBASIS

#include "s-basis.h"
#include "point.h"

namespace Geom{

template <unsigned D>
class MultidimSBasis{
public:
    SBasis f[D];
    
    SBasis& operator[](unsigned i) {
        return f[i];
    }
    SBasis const & operator[](unsigned i) const {
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

template <unsigned D>
inline MultidimSBasis<D>
operator +(MultidimSBasis<D> const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]+b.f[i];
    return r;
}

inline MultidimSBasis<2>
operator +(MultidimSBasis<2> const & a, Geom::Point const & b) {
    MultidimSBasis<2> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i]+BezOrd(b[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
operator -(MultidimSBasis<D> const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i] - b.f[i];
    return r;
}

inline MultidimSBasis<2>
operator -(MultidimSBasis<2> const & a, Geom::Point const & b) {
    MultidimSBasis<2> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i] - BezOrd(b[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
operator *(double a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a*b.f[i];
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
operator +=(MultidimSBasis<D> & a, MultidimSBasis<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]+=b.f[i];
    return a;
}

inline MultidimSBasis<2>
operator +=(MultidimSBasis<2> & a, Geom::Point const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]+= b[i];
    return a;
}

inline MultidimSBasis<2>
operator -=(MultidimSBasis<2> & a, Geom::Point const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i] -= b[i];
    return a;
}

template <unsigned D>
inline MultidimSBasis<D>
operator *=(MultidimSBasis<D> & a, double b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]*=b;
    return a;
}

/*template <unsigned D>
MultidimSBasis<D>
operator -=(MultidimSBasis<D> & a, MultidimSBasis<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]-=b.f[i];
    return a;
}*/

template <unsigned D>
inline MultidimSBasis<D>
derivative(MultidimSBasis<D> const & a) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
integral(MultidimSBasis<D> const & a) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
dot(MultidimSBasis<D> const & a, MultidimSBasis<D> const & b) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
L2(MultidimSBasis<D> const & a, int k) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],a.f[i]);
    return sqrt(r,k);
}

template <unsigned D>
inline MultidimSBasis<D>
multiply(BezOrd const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
multiply(SBasis const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
operator*(BezOrd const & a, MultidimSBasis<D> const & b) {
    return multiply(a, b);
}

template <unsigned D>
inline MultidimSBasis<D>
operator*(SBasis const & a, MultidimSBasis<D> const & b) {
    return multiply(a, b);
}

template <unsigned D>
inline MultidimSBasis<D>
compose(SBasis const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a,b.f[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
compose(MultidimSBasis<D> const & a, BezOrd const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
compose(MultidimSBasis<D> const & a, SBasis const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
compose(MultidimSBasis<D> const & a, MultidimSBasis<D> const & b) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline MultidimSBasis<D>
truncate(MultidimSBasis<D> const & a, unsigned terms) {
    MultidimSBasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = truncate(a.f[i],terms);
    return r;
}

inline MultidimSBasis<2> 
rot90(MultidimSBasis<2> const & a) {
    MultidimSBasis<2> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
}

inline MultidimSBasis<2>
cross(MultidimSBasis<2> const & a, MultidimSBasis<2> const & b) {
    MultidimSBasis<2> r;
    r[0] = -multiply(a.f[0],b.f[1]);
    r[1] = multiply(a.f[1],b.f[0]);
    return r;
}

inline Geom::Point
point_at(MultidimSBasis<2> const & a, double t) {
    return Geom::Point(a[0](t), a[1](t));
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
