#ifndef _MULTIDIM_PIECEWISE_SBASIS
#define _MULTIDIM_PIECEWISE_SBASIS

#include "pw-sb.h"
#include "point.h"
#include "s-basis-2d.h"

namespace Geom{

template <unsigned D>
class md_pw_sb {
public:
    pw_sb f[D];
    
    pw_sb& operator[](unsigned i) {
        return f[i];
    }
    pw_sb const & operator[](unsigned i) const {
        return f[i];
    }
    
    //TODO: keep?
    unsigned size() const {
        unsigned s = f[0].size();
        for(unsigned i = 1; i < D; i++)
            s = std::max(s, (unsigned) f[i].size());
        return s;
    }

    /*TODO
    bool is_finite() const {
        for(unsigned i = 0; i < D; i++)
            if(!f[i].is_finite())
                return false;
        return true;
    }
    */
};

template <unsigned D>
inline md_pw_sb<D>
operator +(md_pw_sb<D> const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i]+b.f[i];
    return r;
}

/* TODO
inline md_pw_sb<2>
operator +(md_pw_sb<2> const & a, Geom::Point const & b) {
    md_pw_sb<2> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i] + BezOrd(b[i]);
    return r;
}*/

template <unsigned D>
inline md_pw_sb<D>
operator -(md_pw_sb<D> const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a.f[i] - b.f[i];
    return r;
}

/* TODO
inline md_pw_sb<2>
operator -(md_pw_sb<2> const & a, Geom::Point const & b) {
    md_pw_sb<2> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i] = a.f[i] - BezOrd(b[i]);
    return r;
}*/

template <unsigned D>
inline md_pw_sb<D>
operator *(double a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i] = a*b.f[i];
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
operator +=(md_pw_sb<D> & a, md_pw_sb<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]+=b.f[i];
    return a;
}

inline md_pw_sb<2>
operator +=(md_pw_sb<2> & a, Geom::Point const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i]+= b[i];
    return a;
}

inline md_pw_sb<2>
operator -=(md_pw_sb<2> & a, Geom::Point const & b) {
    for(unsigned i = 0; i < 2; i++)
        a.f[i] -= b[i];
    return a;
}

template <unsigned D>
inline md_pw_sb<D>
operator *=(md_pw_sb<D> & a, double b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]*=b;
    return a;
}

/*template <unsigned D>
md_pw_sb<D>
operator -=(md_pw_sb<D> & a, md_pw_sb<D> const & b) {
    for(unsigned i = 0; i < D; i++)
        a.f[i]-=b.f[i];
    return a;
}*/

/*TODO
template <unsigned D>
inline md_pw_sb<D>
derivative(md_pw_sb<D> const & a) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
integral(md_pw_sb<D> const & a) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
dot(md_pw_sb<D> const & a, md_pw_sb<D> const & b) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline SBasis
L2(md_pw_sb<D> const & a, int k) {
    SBasis r;
    for(unsigned i = 0; i < D; i++)
        r += multiply(a.f[i],a.f[i]);
    return sqrt(r,k);
}
*/

template <unsigned D>
inline md_pw_sb<D>
multiply(BezOrd const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
multiply(pw_sb const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = multiply(a,b.f[i]);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
operator*(BezOrd const & a, md_pw_sb<D> const & b) {
    return multiply(a, b);
}

template <unsigned D>
inline md_pw_sb<D>
operator*(pw_sb const & a, md_pw_sb<D> const & b) {
    return multiply(a, b);
}

/*TODO
template <unsigned D>
inline md_pw_sb<D>
compose(pw_sb const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a,b.f[i]);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
compose(md_pw_sb<D> const & a, BezOrd const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
compose(md_pw_sb<D> const & a, pw_sb const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
compose(md_pw_sb<D> const & a, md_pw_sb<D> const & b) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = compose(a.f[i],b.f[i]);
    return r;
}

template <unsigned D>
inline md_pw_sb<D>
truncate(md_pw_sb<D> const & a, unsigned terms) {
    md_pw_sb<D> r;
    for(unsigned i = 0; i < D; i++)
        r[i] = truncate(a.f[i],terms);
    return r;
}

inline md_pw_sb<2> 
rot90(md_pw_sb<2> const & a) {
    md_pw_sb<2> r;
    r.f[0] = -a.f[1];
    r.f[1] = a.f[0];
    return r;
}

inline md_pw_sb<2>
cross(md_pw_sb<2> const & a, md_pw_sb<2> const & b) {
    md_pw_sb<2> r;
    r[0] = -multiply(a.f[0],b.f[1]);
    r[1] = multiply(a.f[1],b.f[0]);
    return r;
}
*/

inline pw_sb compose(SBasis2d const &a, md_pw_sb<2> const &b) {
    pw_sb ret, x = partition(b[0], b[1].cuts), y = partition(b[1], b[0].cuts);
    //TODO: extract this partition idiom
    assert(x.size() == y.size());
    for(int i = 0; i < x.size(); i++) {
        ret.cuts.push_back(x.cuts[i]);
        MultidimSBasis<2> sb;
        sb[0] = x[i]; sb[1] = y[i];
        ret.segs.push_back(compose(a, sb));
    }
    ret.cuts.push_back(x.cuts[x.size()]);
    return ret;
}

inline Geom::Point
point_at(md_pw_sb<2> const & a, double t) {
    return Geom::Point(a[0](t), a[1](t));
}

};
#endif
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
