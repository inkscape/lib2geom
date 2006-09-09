#ifndef _MULTIDIM_SBASIS
#define _MULTIDIM_SBASIS

#include "s-basis.h"
#include "point.h"

template <unsigned D>
class multidim_sbasis{
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
};

template <unsigned D>
inline multidim_sbasis<D>
operator +(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]+b.f[i];
    return r;
}

inline multidim_sbasis<2>
operator +(multidim_sbasis<2> const & a, Geom::Point const & b) {
    multidim_sbasis<2> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i]+BezOrd(b[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
operator -(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]-b.f[i];
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
operator *(double a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a*b.f[i];
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
operator +=(multidim_sbasis<D> & a, multidim_sbasis<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]+=b.f[i];
    return a;
}

inline multidim_sbasis<2>
operator +=(multidim_sbasis<2> & a, Geom::Point const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]+= b[i];
    return a;
}

template <unsigned D>
inline multidim_sbasis<D>
operator *=(multidim_sbasis<D> & a, double b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]*=b;
    return a;
}

/*template <unsigned D>
multidim_sbasis<D>
operator -=(multidim_sbasis<D> & a, multidim_sbasis<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]-=b.f[i];
    return a;
}*/

template <unsigned D>
inline multidim_sbasis<D>
derivative(multidim_sbasis<D> const & a) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
integral(multidim_sbasis<D> const & a) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
dot(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
L2(multidim_sbasis<D> const & a, int k) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],a.f[i]);
    return sqrt(r,k);
}

template <unsigned D>
inline multidim_sbasis<D>
multiply(BezOrd const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
multiply(SBasis const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
compose(SBasis const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a,b.f[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
compose(multidim_sbasis<D> const & a, BezOrd const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
compose(multidim_sbasis<D> const & a, SBasis const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
compose(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline multidim_sbasis<D>
truncate(multidim_sbasis<D> const & a, unsigned terms) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = truncate(a.f[i],terms);
    return r;
}

inline multidim_sbasis<2> 
rot90(multidim_sbasis<2> const & a) {
    multidim_sbasis<2> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
}

#include "point.h"
inline Geom::Point
point_at(multidim_sbasis<2> const & a, double t) {
    return Geom::Point(a[0](t), a[1](t));
}

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
