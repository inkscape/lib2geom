#ifndef SEEN_SBASIS_H
#define SEEN_SBASIS_H
#include <vector>
#include <cassert>
#include <algorithm>

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
    operator Tri() const {
        return a[1] - a[0];
    }
    operator Hat() const {
        return (a[1] + a[0])/2;
    }
    double apply(double t) { return (1-t)*a[0] + t*a[1];}
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
inline BezOrd operator*(double const a, BezOrd const & b) {
    return BezOrd(a*b[0], a*b[1]);
}

class SBasis{
public:
    std::vector<BezOrd> a;
    
    unsigned size() const { return a.size(); }
    
    SBasis() {}
    SBasis(BezOrd const & bo) {
        a.push_back(bo);
    }
    
    BezOrd& operator[](const unsigned i) {
        assert(i < a.size());
        return a[i];
    }
    BezOrd const & operator[](const unsigned i) const {
        assert(i < a.size());
        return a[i];
    }
    double point_at(double t) {
        double s = t*(1-t);
        double p0 = 0, p1 = 0;
        double sk = 1;
        int k = 0;
// XXX rewrite as horner
        for(int k = 0; k < a.size(); k++) {
            p0 += sk*a[k][0];
            p1 += sk*a[k][1];
            sk *= s;
        }
        return (1-t)*p0 + t*p1;
    }
    SBasis operator+(const SBasis& p) const {
        SBasis result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.a.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.a.push_back(a[i] + p.a[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.a.push_back(a[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.a.push_back(p.a[i]);
        assert(result.size() == out_size);
        return result;
    }
    SBasis operator-(const SBasis& p) const {
        SBasis result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.a.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.a.push_back(a[i] - p.a[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.a.push_back(a[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.a.push_back(-p.a[i]);
        assert(result.size() == out_size);
        return result;
    }

    void clear() {
        fill(a.begin(), a.end(), BezOrd(0,0));
    }
};

inline SBasis operator-(const SBasis& p) {
    SBasis result;
    result.a.reserve(p.size());
        
    for(unsigned i = 0; i < p.size(); i++) {
        result.a.push_back(-p.a[i]);
    }
    return result;
}

inline SBasis& operator+=(SBasis& a, const SBasis& b) {
    const unsigned out_size = std::max(a.size(), b.size());
    const unsigned min_size = std::min(a.size(), b.size());
    a.a.reserve(out_size);
        
    for(unsigned i = 0; i < min_size; i++)
        a.a[i] += b.a[i];
    for(unsigned i = min_size; i < b.size(); i++)
        a.a.push_back(b.a[i]);
    
    assert(a.size() == out_size);
    return a;
}

SBasis operator*(double k, SBasis const &a);

SBasis shift(SBasis const &a, int sh);

SBasis shift(BezOrd const &a, int sh);

SBasis multiply(SBasis const &a, SBasis const &b);
SBasis compose(SBasis const &a, SBasis const &b);

SBasis integral(SBasis const &c);

SBasis derivative(SBasis const &a);

SBasis sqrt(SBasis const &a, int k);

// return a kth order approx to 1/a)
SBasis reciprocal(BezOrd const &a, int k);

SBasis divide(SBasis const &a, SBasis const &b, int k);

#include <iostream>
// a(b(t))
SBasis compose(SBasis const &a, SBasis const &b);

inline std::ostream &operator<< (std::ostream &out_file, const BezOrd &bo) {
    out_file << "{" << bo.a[0] << ", " << bo.a[1] << "}";
    return out_file;
}

inline std::ostream &operator<< (std::ostream &out_file, const SBasis & p) {
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
