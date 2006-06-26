#ifndef SEEN_SBASIS_H
#define SEEN_SBASIS_H
#include <vector>

class BezOrd{
public:
    double d[2];
    BezOrd() {}
    BezOrd(double a, double b) {d[0] = a; d[1] = b;}

    double operator[](const int i) const {
        assert(i >= 0);
        assert(i < 2);
        return d[i];
    }
    double& operator[](const int i) {
        assert(i >= 0);
        assert(i < 2);
        return d[i];
    }
};

BezOrd operator+(BezOrd const & a, BezOrd const & b) {
    return BezOrd(a[0] + b[0], a[1] + b[1]);
}

class SBasis{
public:
    std::vector<BezOrd> a;
    
    unsigned size() const { return a.size(); }
    
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
