#include <math.h>

#include "s-basis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"

void bounds(SBasis const & s,
            double &lo, double &hi) {
    double ss = 0.25;
    double mid = s(0.5);
    lo = hi = mid;
    for(unsigned i = 1; i < s.size(); i++) {
        for(unsigned dim = 0; dim < 2; dim++) {
            double b = s[i][dim]*ss;
            if(b < 0)
                lo += b;
            if(b > 0)
                hi += b;
        }
        ss *= 0.25;
    }
    lo = std::min(std::min(lo, s[0][1]),s[0][0]);
    hi = std::max(std::max(hi, s[0][0]), s[0][1]);
}

#if 0
double Laguerre_internal(SBasis const & p,
                         double x0,
                         double tol,
                         bool & quad_root) {
    double a = 2*tol;
    double xk = x0;
    double n = p.size();
    quad_root = false;
    while(a > tol) {
        //std::cout << "xk = " << xk << std::endl;
        BezOrd b = p.back();
        BezOrd d(0), f(0);
        double err = fabs(b);
        double abx = fabs(xk);
        for(int j = p.size()-2; j >= 0; j--) {
            f = xk*f + d;
            d = xk*d + b;
            b = xk*b + p[j];
            err = fabs(b) + abx*err;
        }
        
        err *= 1e-7; // magic epsilon for convergence, should be computed from tol
        
        double px = b;
        if(fabs(b) < err)
            return xk;
        //if(std::norm(px) < tol*tol)
        //    return xk;
        double G = d / px;
        double H = G*G - f / px;
        
        //std::cout << "G = " << G << "H = " << H;
        double radicand = (n - 1)*(n*H-G*G);
        //assert(radicand.real() > 0);
        if(radicand < 0)
            quad_root = true;
        //std::cout << "radicand = " << radicand << std::endl;
        if(G.real() < 0) // here we try to maximise the denominator avoiding cancellation
            a = - sqrt(radicand);
        else
            a = sqrt(radicand);
        //std::cout << "a = " << a << std::endl;
        a = n / (a + G);
        //std::cout << "a = " << a << std::endl;
        xk -= a;
    }
    //std::cout << "xk = " << xk << std::endl;
    return xk;
}
#endif

void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots, 
                   double left, double right) {
    double lo, hi;
    bounds(s, lo, hi);
    if(lo > 0 || hi < 0)
        return; // no roots here
    if(s.tail_error(1) < 1e-7) {
        double t = s[0][0] / (s[0][0] - s[0][1]);
        roots.push_back(left*(1-t) + t*right);
        return;
    }
    double middle = (left + right)/2;
    subdiv_sbasis(compose(s, BezOrd(0, 0.5)), roots, left, middle);
    subdiv_sbasis(compose(s, BezOrd(0.5, 1.)), roots, middle, right);
}

std::vector<double> roots(SBasis const & s) {
    std::vector<double> b = sbasis_to_bezier(s), r;
    
    find_bernstein_roots(&b[0], b.size()-1, r, 0, 0., 1.);
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
