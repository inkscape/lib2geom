#ifndef _MULTIDIM_SBASIS
#define _MULTIDIM_SBASIS

#include "s-basis.h"

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
};

template <unsigned D>
multidim_sbasis<D>
operator +(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]+b.f[i];
    return r;
}

template <unsigned D>
multidim_sbasis<D>
operator -(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]-b.f[i];
    return r;
}

template <unsigned D>
multidim_sbasis<D>
operator +=(multidim_sbasis<D> & a, multidim_sbasis<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]+=b.f[i];
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
multidim_sbasis<D>
derivative(multidim_sbasis<D> const & a) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

template <unsigned D>
multidim_sbasis<D>
integral(multidim_sbasis<D> const & a) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <unsigned D>
SBasis
dot(multidim_sbasis<D> const & a, multidim_sbasis<D> const & b) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
multidim_sbasis<D>
multiply(BezOrd const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
multidim_sbasis<D>
multiply(SBasis const & a, multidim_sbasis<D> const & b) {
    multidim_sbasis<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

multidim_sbasis<2> 
rot90(multidim_sbasis<2> const & a) {
    multidim_sbasis<2> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
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
