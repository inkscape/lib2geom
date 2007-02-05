#ifndef SEEN_SBASIS_H
#define SEEN_SBASIS_H
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>

namespace Geom{

class Hat{
public:
    Hat () {}
    Hat(double d) :d(d) {}
    operator double() const { return d; }
    double d;
};

class Tri{
public:
    Tri () {}
    Tri(double d) :d(d) {}
    operator double() const { return d; }
    double d;
};

class BezOrd{
public:
    double a[2];
    BezOrd() {}
    BezOrd(double aa, double b) {a[0] = aa; a[1] = b;}
    BezOrd(Hat h, Tri t) {
        a[0] = double(h) - double(t)/2; 
        a[1] = double(h) + double(t)/2;
    }

    BezOrd(Hat h) {
        a[0] = double(h); 
        a[1] = double(h);
    }

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 2);
        return a[i];
    }
    double point_at(double t) {
        return (a[0]*(1-t) + a[1]*t);
    }
    double
    operator()(double t) {
        return (a[0]*(1-t) + a[1]*t);
    }
    operator Tri() const {
        return a[1] - a[0];
    }
    operator Hat() const {
        return (a[1] + a[0])/2;
    }
    double apply(double t) { return (1-t)*a[0] + t*a[1];}
    
    bool zero() const { return a[0] == 0 && a[1] == 0; }
    bool is_finite() const;
};

inline BezOrd operator-(BezOrd const &a) {
    return BezOrd(-a.a[0], -a.a[1]);
}
inline BezOrd operator+(BezOrd const & a, BezOrd const & b) {
    return BezOrd(a[0] + b[0], a[1] + b[1]);
}
inline BezOrd operator-(BezOrd const & a, BezOrd const & b) {
    return BezOrd(a[0] - b[0], a[1] - b[1]);
}
inline BezOrd& operator+=(BezOrd & a, BezOrd const & b) {
    a[0] += b[0];
    a[1] += b[1];
    return a;
}
inline BezOrd& operator-=(BezOrd & a, BezOrd const & b) {
    a[0] -= b[0];
    a[1] -= b[1];
    return a;
}
inline bool operator==(BezOrd const & a, BezOrd const & b) {
    return a[0] == b[0] &&
        a[1] == b[1];
}
inline bool operator!=(BezOrd const & a, BezOrd const & b) {
    return a[0] != b[0] ||
        a[1] != b[1];
}
inline BezOrd operator*(double const a, BezOrd const & b) {
    return BezOrd(a*b[0], a*b[1]);
}

inline BezOrd
reverse(BezOrd const &b) {
    return BezOrd(b[1], b[0]);
}

/*** An empty SBasis is identically 0. */
class SBasis : public std::vector<BezOrd>{
public:
    SBasis() {}
    SBasis(SBasis const & a) :
        std::vector<BezOrd>(a)
    {}
    SBasis(BezOrd const & bo) {
        push_back(bo);
    }
    
    double point_at(double t) const {
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        double sk = 1;
        int k = 0;
//TODO: rewrite as horner
        for(int k = 0; k < size(); k++) {
            p0 += sk*(*this)[k][0];
            p1 += sk*(*this)[k][1];
            sk *= s;
        }
        return (1-t)*p0 + t*p1;
    }
    double operator()(double t) const {
        return point_at(t);
    }
    SBasis operator+(const SBasis& p) const {
        SBasis result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] + p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(p[i]);
        assert(result.size() == out_size);
        return result;
    }
    SBasis operator-(const SBasis& p) const {
        SBasis result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.push_back((*this)[i] - p[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.push_back((*this)[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.push_back(-p[i]);
        assert(result.size() == out_size);
        return result;
    }

    void clear() {
        fill(begin(), end(), BezOrd(0,0));
    }
    
    void normalize(); // remove extra zeros

    double tail_error(unsigned tail) const;
    
    void truncate(unsigned k);

// compute f(g)
    SBasis
    operator()(SBasis const & g) const;
    
    BezOrd&
    operator[](unsigned i) {
        //assert(i < size());
        return this->at(i);
        //return std::vector<BezOrd>::operator[](i);
    }
    
    BezOrd
    operator[](unsigned i) const {
        assert(i < size());
        return std::vector<BezOrd>::operator[](i);
    }
    bool is_finite() const;
};

inline SBasis operator-(const SBasis& p) {
    SBasis result;
    result.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(-p[i]);
    }
    return result;
}

inline SBasis operator-(BezOrd const & bo, const SBasis& p) {
    SBasis result;
    result.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.push_back(-p[i]);
    }
    result[0] += bo;
    return result;
   
}

inline SBasis& operator+=(SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    a.reserve(out_size);
        
    for(unsigned i = 0; i < min_size; i++)
        a[i] += b[i];
    for(unsigned i = min_size; i < b.size(); i++)
        a.push_back(b[i]);
    
    assert(a.size() == out_size);
    return a;
}

inline SBasis& operator-=(SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    a.reserve(out_size);
        
    for(unsigned i = 0; i < min_size; i++)
        a[i] -= b[i];
    for(unsigned i = min_size; i < b.size(); i++)
        a.push_back(-b[i]);
    
    assert(a.size() == out_size);
    return a;
}

inline SBasis& operator+=(SBasis& a, const BezOrd& b) {
    if(a.empty())
        a.push_back(b);
    else
        a[0] += b;
    return a;
}

inline SBasis& operator-=(SBasis& a, const BezOrd& b) {
    if(a.empty())
        a.push_back(-b);
    else
        a[0] -= b;
    return a;
}

inline SBasis& operator+=(SBasis& a, double b) {
    if(a.empty())
        a.push_back(BezOrd(b,b));
    else {
        a[0][0] += double(b);
        a[0][1] += double(b);
    }
    return a;
}

inline SBasis operator+(BezOrd b, SBasis a) {
    if(a.empty())
        a.push_back(b);
    else {
        a[0] += b;
    }
    return a;
}

inline SBasis operator+(double b, SBasis a) {
    if(a.empty())
        a.push_back(BezOrd(b,b));
    else {
        a[0][0] += double(b);
        a[0][1] += double(b);
    }
    return a;
}

inline SBasis& operator-=(SBasis& a, double b) {
    if(a.empty())
        a.push_back(BezOrd(-b, -b));
    else {
        a[0][0] -= double(b);
        a[0][1] -= double(b);
    }
    return a;
}

inline SBasis& operator*=(SBasis& a, double b) {
    for(unsigned i = 0; i < a.size(); i++) {
        a[i][0] *= b;
        a[i][1] *= b;
    }
    return a;
}

inline SBasis& operator/=(SBasis& a, double b) {
    for(unsigned i = 0; i < a.size(); i++) {
        a[i][0] /= b;
        a[i][1] /= b;
    }
    return a;
}

SBasis operator*(double k, SBasis const &a);
SBasis operator*(SBasis const &a, SBasis const &b);

SBasis shift(SBasis const &a, int sh);

SBasis shift(BezOrd const &a, int sh);

SBasis truncate(SBasis const &a, unsigned terms);

SBasis multiply(SBasis const &a, SBasis const &b);

SBasis integral(SBasis const &c);

SBasis derivative(SBasis const &a);

SBasis sqrt(SBasis const &a, int k);

// return a kth order approx to 1/a)
SBasis reciprocal(BezOrd const &a, int k);

SBasis divide(SBasis const &a, SBasis const &b, int k);

inline SBasis
operator*(SBasis const & a, SBasis const & b) {
    return multiply(a, b);
}

inline SBasis& operator*=(SBasis& a, SBasis const & b) {
    a = multiply(a, b);
    return a;
}

// a(b(t))
SBasis compose(SBasis const &a, SBasis const &b);
SBasis compose(SBasis const &a, SBasis const &b, unsigned k);
SBasis inverse(SBasis a, int k);

// compute f(g)
inline SBasis
SBasis::operator()(SBasis const & g) const {
    return compose(*this, g);
}
 
inline std::ostream &operator<< (std::ostream &out_file, const BezOrd &bo) {
    out_file << "{" << bo[0] << ", " << bo[1] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis & p) {
    for(int i = 0; i < p.size(); i++) {
        out_file << p[i] << "s^" << i << " + ";
    }
    return out_file;
}

SBasis sin(BezOrd bo, int k);
SBasis cos(BezOrd bo, int k);

SBasis reverse(SBasis const &s);

//void bounds(SBasis const & s, double &lo, double &hi);
void bounds(SBasis const & s, double &lo, double &hi,int order=0);
 void slow_bounds(SBasis const & s, double &lo, double &hi,int order=0,double tol=0.01);

std::vector<double> roots(SBasis const & s);

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
