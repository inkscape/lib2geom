#include <math.h>

#include "s-basis.h"

void bounds(SBasis const & s,
            double &lo, double &hi) {
    lo = std::min(s[0][0], s[0][1]);
    hi = std::max(s[0][0], s[0][1]);
    for(unsigned i = 1; i < s.size(); i++) {
        double b = fabs(Hat(s[i]));
        lo -= b;
        hi += b;
    }
}

void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots, 
                   double left, double right) {
    double lo, hi;
    bounds(s, lo, hi);
    if(lo > 0 || hi < 0)
        return; // no roots here
    if(s.tail_error(1) < 1) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, BezOrd(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, BezOrd(0.5, 1.)), roots, middle, right);
}

std::vector<double> roots(SBasis const & s) {
    std::vector<double> r;
    subdiv_sbasis(s, r, 0, 1);
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
