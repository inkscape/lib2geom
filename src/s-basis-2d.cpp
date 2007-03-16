#include "s-basis-2d.h"

namespace Geom{

SBasis extract_u(SBasis2d const &a, double u) {
    SBasis sb;
    double s = u*(1-u);
    
    for(unsigned vi = 0; vi < a.vs; vi++) {
        double sk = 1;
        BezOrd bo(0,0);
        for(unsigned ui = 0; ui < a.us; ui++) {
            bo += sk*(extract_u(a.index(ui, vi), u));
            sk *= s;
        }
        sb.push_back(bo);
    }
    
    return sb;
}

SBasis extract_v(SBasis2d const &a, double v) {
    SBasis sb;
    double s = v*(1-v);
    
    for(unsigned ui = 0; ui < a.us; ui++) {
        double sk = 1;
        BezOrd bo(0,0);
        for(unsigned vi = 0; vi < a.vs; vi++) {
            bo += sk*(extract_v(a.index(ui, vi), v));
            sk *= s;
        }
        sb.push_back(bo);
    }
    
    return sb;
}

SBasis compose(BezOrd2d const &a, D2<SBasis> p) {
    SBasis sb;
    D2<SBasis> omp;
    for(int dim = 0; dim < 2; dim++)
        omp[dim] = BezOrd(1) - p[dim];
    sb = a[0]*multiply(omp[0], omp[1]) +
         a[1]*multiply(p[0], omp[1]) +
         a[2]*multiply(omp[0], p[1]) +
         a[3]*multiply(p[0], p[1]);
    return sb;
}

SBasis 
compose(SBasis2d const &fg, D2<SBasis> p) {
    SBasis B;
    SBasis s[2];
    SBasis ss[2];
    for(int dim = 0; dim < 2; dim++) 
        s[dim] = p[dim]*(BezOrd(1) - p[dim]);
    ss[1] = BezOrd(1);
    for(int vi = 0; vi < fg.vs; vi++) {
        ss[0] = ss[1];
        for(int ui = 0; ui < fg.us; ui++) {
            unsigned i = ui + vi*fg.us;
            B += ss[0]*compose(fg[i], p);
            ss[0] *= s[0];
        }
        ss[1] *= s[1];
    }
    return B;
}


D2<SBasis> 
compose(D2<SBasis2d> const &fg, D2<SBasis> const & p) {
    D2<SBasis> B;
    for(int dim = 0; dim < 2; dim++)
        B[dim] = compose(fg[dim], p);
    return B;
}

};
