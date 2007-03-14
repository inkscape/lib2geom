#include "d2.h"
#include "rect.h"

namespace Geom {

D2<SBasis> derivative(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i]=derivative(a.f[i]);
    return r;
}

D2<SBasis> integral(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r.f[i]=integral(a.f[i]);
    return r;
}

SBasis L2(D2<SBasis> const & a, int k) { return sqrt(dot(a, a), k); }
double L2(D2<double> const & a) { return hypot(a[0], a[1]); }

D2<SBasis> multiply(BezOrd const & a, D2<SBasis> const & b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

D2<SBasis> operator*(BezOrd const & a, D2<SBasis> const & b) {
    return multiply(a, b);
}

D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b.f[i]);
    return r;
}

D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = truncate(a.f[i], terms);
    return r;
}

unsigned sbasisSize(D2<SBasis> const & a) {
    return std::max((unsigned) a[0].size(), (unsigned) a[1].size());
}
    
double tailError(D2<SBasis> const & a, unsigned tail) {
    return std::max(a[0].tail_error(tail), a[1].tail_error(tail));
}

bool isFinite(D2<SBasis> const & a) {
    for(unsigned i = 0; i < 2; i++)
        if(!a[i].is_finite())
            return false;
    return true;
}

vector<D2<SBasis> > sectionize(D2<pw_sb> const &a, vector<double> &cuts) {
    pw_sb x = partition(a[0], a[1].cuts), y = partition(a[1], a[0].cuts);
    assert(x.size() == y.size());
    vector<D2<SBasis> > ret;
    for(int i = 0; i < x.size(); i++)
        ret.push_back(D2<SBasis>(x[i], y[i]));
    cuts.insert(cuts.end(), x.cuts.begin(), x.cuts.end());
    return ret;
}

Rect local_bounds(D2<SBasis> const & s, double t0, double t1, int order) {
    Point mn, mx;
    for(int d = 0; d < 2; d++)
        local_bounds(s[d], t0, t1, mn[d], mx[d], order);
    return Rect::define(mn,mx);
}

};
