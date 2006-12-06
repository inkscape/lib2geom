#ifndef SEEN_SBASIS_2D_H
#define SEEN_SBASIS_2D_H
#include <vector>
#include <cassert>
#include <algorithm>
#include "s-basis.h"
#include "multidim-sbasis.h"

class BezOrd2d{
public:
    /*  
        u 0,1
        v 0,2
    */
    double a[4];
    BezOrd2d() {}
    BezOrd2d(double aa) {
        for(unsigned i = 0 ; i < 4; i ++) 
            a[i] = aa;
    }
    BezOrd2d(double a00, double a01, double a10, double a11) 
    {
        a[0] = a00; 
        a[1] = a01;
        a[2] = a10; 
        a[3] = a11; 
    }

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 4);
        return a[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 4);
        return a[i];
    }
    double apply(double u, double v) {
        return (a[0]*(1-u)*(1-v) +
                a[1]*u*(1-v) +
                a[2]*(1-u)*v +
                a[3]*u*v);
    }
};

inline BezOrd extract_u(BezOrd2d const &a, double u) {
    return BezOrd(a[0]*(1-u) +
                  a[1]*u,
                  a[2]*(1-u) +
                  a[3]*u);
}
inline BezOrd extract_v(BezOrd2d const &a, double v) {
    return BezOrd(a[0]*(1-v) +
                  a[2]*v,
                  a[1]*(1-v) +
                  a[3]*v);
}
inline BezOrd2d operator-(BezOrd2d const &a) {
    return BezOrd2d(-a.a[0], -a.a[1],
                    -a.a[2], -a.a[3]);
}
inline BezOrd2d operator+(BezOrd2d const & a, BezOrd2d const & b) {
    return BezOrd2d(a[0] + b[0], 
                  a[1] + b[1],
                  a[2] + b[2], 
                  a[3] + b[3]);
}
inline BezOrd2d operator-(BezOrd2d const & a, BezOrd2d const & b) {
    return BezOrd2d(a[0] - b[0],
                  a[1] - b[1],
                  a[2] - b[2],
                  a[3] - b[3]);
}
inline BezOrd2d& operator+=(BezOrd2d & a, BezOrd2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] += b[i];
    return a;
}
inline BezOrd2d& operator-=(BezOrd2d & a, BezOrd2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] -= b[i];
    return a;
}
inline BezOrd2d& operator*=(BezOrd2d & a, double b) {
    for(unsigned i = 0; i < 4; i++)
        a[i] *= b;
    return a;
}

inline bool operator==(BezOrd2d const & a, BezOrd2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        if(a[i] != b[i])
            return false;
    return true;
}
inline bool operator!=(BezOrd2d const & a, BezOrd2d const & b) {
    for(unsigned i = 0; i < 4; i++)
        if(a[i] == b[i])
            return false;
    return true;
}
inline BezOrd2d operator*(double const a, BezOrd2d const & b) {
    return BezOrd2d(a*b[0], a*b[1],
                  a*b[2], a*b[3]);
}

class SBasis2d : public std::vector<BezOrd2d>{
public:
    // vector in u,v
    unsigned us, vs; // number of u terms, v terms
    SBasis2d() {}
    SBasis2d(BezOrd2d const & bo) 
        : us(1), vs(1) {
        push_back(bo);
    }
    SBasis2d(SBasis2d const & a) 
        : std::vector<BezOrd2d>(a), us(a.us), vs(a.vs) {}
    
    BezOrd2d& index(unsigned ui, unsigned vi) {
        assert(ui < us);
        assert(vi < vs);
        return (*this)[ui + vi*us];        
    }
    
    BezOrd2d index(unsigned ui, unsigned vi) const {
        if(ui >= us) 
            return BezOrd2d(0);
        if(vi >= vs)
            return BezOrd2d(0);
        return (*this)[ui + vi*us];        
    }
    
    double apply(double u, double v) const {
        double s = u*(1-u);
        double t = v*(1-v);
        BezOrd2d p;
        double tk = 1;
// XXX rewrite as horner
        for(unsigned vi = 0; vi < vs; vi++) {
            double sk = 1;
            for(unsigned ui = 0; ui < us; ui++) {
                p += (sk*tk)*index(ui, vi);
                sk *= s;
            }
            tk *= t;
        }
        return p.apply(u,v);
    }

    void clear() {
        fill(begin(), end(), BezOrd2d(0));
    }
    
    void normalize(); // remove extra zeros

    double tail_error(unsigned tail) const;
    
    void truncate(unsigned k);
};

inline SBasis2d operator-(const SBasis2d& p) {
    SBasis2d result;
    result.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(-p[i]);
    }
    return result;
}

inline SBasis2d operator+(const SBasis2d& a, const SBasis2d& b) {
    SBasis2d result;
    result.us = std::max(a.us, b.us);
    result.vs = std::max(a.vs, b.vs);
    const unsigned out_size = result.us*result.vs;
    result.resize(out_size);
        
    for(unsigned vi = 0; vi < result.vs; vi++) {
        for(unsigned ui = 0; ui < result.us; ui++) {
            BezOrd2d bo;
            if(ui < a.us && vi < a.vs)
                bo += a.index(ui, vi);
            if(ui < b.us && vi < b.vs)
                bo += b.index(ui, vi);
            result.index(ui, vi) = bo;
        }
    }
    return result;
}

inline SBasis2d operator-(const SBasis2d& a, const SBasis2d& b) {
    SBasis2d result;
    result.us = std::max(a.us, b.us);
    result.vs = std::max(a.vs, b.vs);
    const unsigned out_size = result.us*result.vs;
    result.resize(out_size);
        
    for(unsigned vi = 0; vi < result.vs; vi++) {
        for(unsigned ui = 0; ui < result.us; ui++) {
            BezOrd2d bo;
            if(ui < a.us && vi < a.vs)
                bo += a.index(ui, vi);
            if(ui < b.us && vi < b.vs)
                bo -= b.index(ui, vi);
            result.index(ui, vi) = bo;
        }
    }
    return result;
}


inline SBasis2d& operator+=(SBasis2d& a, const BezOrd2d& b) {
    if(a.size() < 1)
        a.push_back(b);
    else
        a[0] += b;
    return a;
}

inline SBasis2d& operator-=(SBasis2d& a, const BezOrd2d& b) {
    if(a.size() < 1)
        a.push_back(-b);
    else
        a[0] -= b;
    return a;
}

inline SBasis2d& operator+=(SBasis2d& a, double b) {
    if(a.size() < 1)
        a.push_back(BezOrd2d(b));
    else {
        for(unsigned i = 0; i < 4; i++)
            a[0] += double(b);
    }
    return a;
}

inline SBasis2d& operator-=(SBasis2d& a, double b) {
    if(a.size() < 1)
        a.push_back(BezOrd2d(-b));
    else {
        a[0] -= b;
    }
    return a;
}

inline SBasis2d& operator*=(SBasis2d& a, double b) {
    for(unsigned i = 0; i < a.size(); i++)
        a[i] *= b;
    return a;
}

inline SBasis2d& operator/=(SBasis2d& a, double b) {
    for(unsigned i = 0; i < a.size(); i++)
        a[i] *= (1./b);
    return a;
}

SBasis2d operator*(double k, SBasis2d const &a);
SBasis2d operator*(SBasis2d const &a, SBasis2d const &b);

SBasis2d shift(SBasis2d const &a, int sh);

SBasis2d shift(BezOrd2d const &a, int sh);

SBasis2d truncate(SBasis2d const &a, unsigned terms);

SBasis2d multiply(SBasis2d const &a, SBasis2d const &b);

SBasis2d integral(SBasis2d const &c);

SBasis2d derivative(SBasis2d const &a);

SBasis2d sqrt(SBasis2d const &a, int k);

// return a kth order approx to 1/a)
SBasis2d reciprocal(BezOrd2d const &a, int k);

SBasis2d divide(SBasis2d const &a, SBasis2d const &b, int k);

#include <iostream>
// a(b(t))
SBasis2d compose(SBasis2d const &a, SBasis2d const &b);
SBasis2d compose(SBasis2d const &a, SBasis2d const &b, unsigned k);
SBasis2d inverse(SBasis2d const &a, int k);

// these two should probably be replaced with compose
SBasis extract_u(SBasis2d const &a, double u);
SBasis extract_v(SBasis2d const &a, double v);

SBasis compose(BezOrd2d const &a, multidim_sbasis<2> p);

SBasis 
compose(SBasis2d const &fg, multidim_sbasis<2> p);

multidim_sbasis<2> 
compose(std::vector<SBasis2d> const &fg, multidim_sbasis<2> p);

inline std::ostream &operator<< (std::ostream &out_file, const BezOrd2d &bo) {
    out_file << "{" << bo[0] << ", " << bo[1] << "}, ";
    out_file << "{" << bo[2] << ", " << bo[3] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis2d & p) {
    for(int i = 0; i < p.size(); i++) {
        out_file << p[i] << "s^" << i << " + ";
    }
    return out_file;
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
