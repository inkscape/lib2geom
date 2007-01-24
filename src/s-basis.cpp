#include <cmath>

#include "s-basis.h"
#include "isnan.h"

namespace Geom{

/*** At some point we should work on tighter bounds for the error.  It is clear that the error is
 * bounded by the L1 norm over the tail of the series, but this is very loose, leading to far too
 * many cubic beziers.  I've changed this to be \sum _i=tail ^\infty |hat a_i| 2^-i but I have no
 * evidence that this is correct.
 */

/*
double SBasis::tail_error(unsigned tail) const {
    double err = 0, s = 1./(1<<(2*tail)); // rough
    for(unsigned i = tail; i < size(); i++) {
        err += (fabs((*this)[i][0]) + fabs((*this)[i][1]))*s;
        s /= 4;
    }
    return err;
}
*/
double SBasis::tail_error(unsigned tail) const {
  double m,M;
  bounds(*this,m,M,tail);
  return std::max(fabs(m),fabs(M));
}

bool BezOrd::is_finite() const {
    return isFinite(a[0]) && isFinite(a[1]);
}

bool SBasis::is_finite() const {
    for(unsigned i = 0; i < size(); i++) {
        if(!(*this)[i].is_finite())
            return false;
    }
    return true;
}



SBasis operator*(double k, SBasis const &a) {
    SBasis c;
    c.resize(a.size(), BezOrd(0,0));
    for(unsigned j = 0; j < a.size(); j++) {
        for(unsigned dim = 0; dim < 2; dim++)
            c[j][dim] += k*a[j][dim];
    }
    return c;
}

SBasis shift(SBasis const &a, int sh) {
    SBasis c = a;
    if(sh > 0) {
        c.insert(c.begin(), sh, BezOrd(0,0));
    } else {
        // truncate
    }
    return c;
}

SBasis shift(BezOrd const &a, int sh) {
    SBasis c;
    if(sh > 0) {
        c.insert(c.begin(), sh, BezOrd(0,0));
        c.push_back(a);
    } else {
        // truncate
    }
    return c;
}

SBasis truncate(SBasis const &a, unsigned terms) {
    SBasis c;
    if(terms > a.size())
        terms = a.size();
    c.insert(c.begin(), a.begin(), a.begin() + terms);
    return c;
}

void
SBasis::truncate(unsigned k) {
    if(k < size()) {
        resize(k);
    }
}

SBasis multiply(SBasis const &a, SBasis const &b) {
    // c = {a0*b0 - shift(1, a.Tri*b.Tri), a1*b1 - shift(1, a.Tri*b.Tri)}
    
    // shift(1, a.Tri*b.Tri)
    SBasis c;
    if(a.empty() || b.empty())
        return c;
    c.resize(a.size() + b.size(), BezOrd(0,0));
    c[0] = BezOrd(0,0);
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            double tri = Tri(b[j])*Tri(a[i-j]);
            c[i+1/*shift*/] += Hat(-tri);
        }
    }
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            for(unsigned dim = 0; dim < 2; dim++)
                c[i][dim] += b[j][dim]*a[i-j][dim];
        }
    }
    c.normalize();
    //assert(!(0 == c.back()[0] && 0 == c.back()[1]));
    return c;
}

SBasis integral(SBasis const &c) {
    SBasis a;
    a.resize(c.size() + 1, BezOrd(0,0));
    a[0] = BezOrd(0,0);
    
    for(unsigned k = 1; k < c.size() + 1; k++) {
        double ahat = -Tri(c[k-1])/(2*k);
        a[k] = Hat(ahat);
    }
    double aTri = 0;
    for(int k = c.size()-1; k >= 0; k--) {
        aTri = (Hat(c[k]) + (k+1)*aTri/2)/(2*k+1);
        a[k][0] -= aTri/2;
        a[k][1] += aTri/2;
    }
    a.normalize();
    return a;
}

SBasis derivative(SBasis const &a) {
    SBasis c;
    c.resize(a.size(), BezOrd(0,0));
    
    for(unsigned k = 0; k < a.size(); k++) {
        double d = (2*k+1)*Tri(a[k]);
        
        for(unsigned dim = 0; dim < 2; dim++) {
            c[k][dim] = d;
            if(k+1 < a.size()) {
                if(dim)
                    c[k][dim] = d - (k+1)*a[k+1][dim];
                else
                    c[k][dim] = d + (k+1)*a[k+1][dim];
            }
        }
    }
    
    return c;
}

SBasis sqrt(SBasis const &a, int k) {
    if(k == 0)
        return SBasis();
    SBasis c;
    if(a.empty())
        return c;
    c.resize(k, BezOrd(0,0));
    c[0] = BezOrd(std::sqrt(a[0][0]), std::sqrt(a[0][1]));
    SBasis r = a - multiply(c, c); // remainder
    
    for(unsigned i = 1; i <= k and i<r.size(); i++) {
        BezOrd ci(r[i][0]/(2*c[0][0]), r[i][1]/(2*c[0][1]));
        SBasis cisi = shift(ci, i);
        r -= multiply(shift((2*c + cisi), i), SBasis(ci));
        r.truncate(k+1);
        c += cisi;
        if(r.tail_error(i) == 0) // if exact
            break;
    }
    
    return c;
}

// return a kth order approx to 1/a)
SBasis reciprocal(BezOrd const &a, int k) {
    SBasis c;
    assert(!a.zero());
    c.resize(k, BezOrd(0,0));
    double r_s0 = (Tri(a)*Tri(a))/(-a[0]*a[1]);
    double r_s0k = 1;
    for(int i = 0; i < k; i++) {
        c[i] = BezOrd(r_s0k/a[0], r_s0k/a[1]);
        r_s0k *= r_s0;
    }
    return c;
}

SBasis divide(SBasis const &a, SBasis const &b, int k) {
    SBasis c;
    if(a.empty())
        throw;
    SBasis r = a; // remainder
    
    k++;
    r.resize(k, BezOrd(0,0));
    c.resize(k, BezOrd(0,0));

    for(unsigned i = 0; i < k; i++) {
        BezOrd ci(r[i][0]/b[0][0], r[i][1]/b[0][1]); //H0
        c[i] += ci;
        r -= shift(multiply(ci,b), i);
        r.truncate(k+1);
        if(r.tail_error(i) == 0) // if exact
            break;
    }
    
    return c;
}

// a(b)
// return a0 + s(a1 + s(a2 +...  where s = (1-u)u; ak =(1 - u)a^0_k + ua^1_k
SBasis compose(SBasis const &a, SBasis const &b) {
    SBasis s = multiply((SBasis(BezOrd(1,1))-b), b);
    SBasis r;
    
    for(int i = a.size()-1; i >= 0; i--) {
        r = SBasis(BezOrd(Hat(a[i][0]))) - a[i][0]*b + a[i][1]*b + multiply(r,s);
    }
    return r;
}

// a(b)
// return a0 + s(a1 + s(a2 +...  where s = (1-u)u; ak =(1 - u)a^0_k + ua^1_k
SBasis compose(SBasis const &a, SBasis const &b, unsigned k) {
    SBasis s = multiply((SBasis(BezOrd(1,1))-b), b);
    SBasis r;
    
    for(int i = a.size()-1; i >= 0; i--) {
        r = SBasis(BezOrd(Hat(a[i][0]))) - a[i][0]*b + a[i][1]*b + multiply(r,s);
    }
    r.truncate(k);
    return r;
}

/*
Inversion algorithm. The notation is certainly very misleading. The
pseudocode should say:

c(v) := 0
r(u) := r_0(u) := u
for i:=0 to k do
  c_i(v) := H_0(r_i(u)/(t_1)^i; u)
  c(v) := c(v) + c_i(v)*t^i
  r(u) := r(u) ? c_i(u)*(t(u))^i
endfor
*/

//#define DEBUG_INVERSION 1 

SBasis inverse(SBasis a, int k) {
    assert(a.size() > 0);
// the function should have 'unit range'("a00 = 0 and a01 = 1") and be monotonic.
    double a0 = a[0][0];
    if(a0 != 0) {
        a -= a0;
    }
    double a1 = a[0][1];
    assert(a1 != 0); // not invertable.
    
    if(a1 != 1) {
        a /= a1;
    }
    SBasis c;                           // c(v) := 0
    if(a.size() >= 2 && k == 2) {
        c.push_back(BezOrd(0,1));
        BezOrd t1(1+a[1][0], 1-a[1][1]);    // t_1
        c.push_back(BezOrd(-a[1][0]/t1[0], -a[1][1]/t1[1]));
    } else if(a.size() >= 2) {                      // non linear
        SBasis r = BezOrd(0,1);             // r(u) := r_0(u) := u
        BezOrd t1(1./(1+a[1][0]), 1./(1-a[1][1]));    // 1./t_1
        BezOrd one(1,1);
        BezOrd t1i = one;                   // t_1^0
        SBasis one_minus_a = SBasis(one) - a;
        SBasis t = multiply(one_minus_a, a); // t(u)
        SBasis ti(one);                     // t(u)^0
#ifdef DEBUG_INVERSION
        std::cout << "a=" << a << std::endl;
        std::cout << "1-a=" << one_minus_a << std::endl;
        std::cout << "t1=" << t1 << std::endl;
        //assert(t1 == t[1]);
#endif
    
        c.resize(k+1, BezOrd(0,0));
        for(unsigned i = 0; i < k; i++) {   // for i:=0 to k do
#ifdef DEBUG_INVERSION
            std::cout << "-------" << i << ": ---------" <<std::endl;
            std::cout << "r=" << r << std::endl
                      << "c=" << c << std::endl
                      << "ti=" << ti << std::endl
                      << std::endl;
#endif
            if(r.size() <= i)                // ensure enough space in the remainder, probably not needed
                r.resize(i+1, BezOrd(0,0));
            BezOrd ci(r[i][0]*t1i[0], r[i][1]*t1i[1]); // c_i(v) := H_0(r_i(u)/(t_1)^i; u)
#ifdef DEBUG_INVERSION
            std::cout << "t1i=" << t1i << std::endl;
            std::cout << "ci=" << ci << std::endl;
#endif
            for(int dim = 0; dim < 2; dim++) // t1^-i *= 1./t1
                t1i[dim] *= t1[dim];
            c[i] = ci; // c(v) := c(v) + c_i(v)*t^i
            // change from v to u parameterisation
            SBasis civ = ci[0]*one_minus_a + ci[1]*a; 
            // r(u) := r(u) - c_i(u)*(t(u))^i
            // We can truncate this to the number of final terms, as no following terms can
            // contribute to the result.
            r -= multiply(civ,ti);
            r.truncate(k);
            if(r.tail_error(i) == 0)
                break; // yay!
            ti = multiply(ti,t);
        }
#ifdef DEBUG_INVERSION
        std::cout << "##########################" << std::endl;
#endif
    } else
        c = BezOrd(0,1); // linear
    c -= a0; // invert the offset
    c /= a1; // invert the slope
    return c;
}


void SBasis::normalize() {
    while(!empty() && 0 == back()[0] && 0 == back()[1])
        pop_back();
}

SBasis sin(BezOrd b, int k) {
    SBasis s = BezOrd(std::sin(b[0]), std::sin(b[1]));
    Tri tr(s[0]);
    double t2 = Tri(b);
    s.push_back(BezOrd(std::cos(b[0])*t2 - tr, -std::cos(b[1])*t2 + tr));
    
    t2 *= t2;
    for(int i = 0; i < k; i++) {
        BezOrd bo(4*(i+1)*s[i+1][0] - 2*s[i+1][1],
                  -2*s[i+1][0] + 4*(i+1)*s[i+1][1]);
        bo -= (t2/(i+1))*s[i];
        
        
        s.push_back((1./(i+2))*bo);
    }
    
    return s;
}

SBasis cos(BezOrd bo, int k) {
    return sin(BezOrd(bo[0] + M_PI/2,
                      bo[1] + M_PI/2),
               k);
}

SBasis reverse(SBasis const &c) {
    SBasis a;
    a.reserve(c.size());
    for(unsigned k = 0; k < c.size(); k++)
        a.push_back(reverse(c[k]));
    return a;
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
