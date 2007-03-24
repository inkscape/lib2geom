#ifndef SEEN_GEOM_PW_SB_H
#define SEEN_GEOM_PW_SB_H

#include "s-basis.h"
#include <vector>

using namespace std;

namespace Geom {

template <typename T> class Piecewise;

template <>
class Piecewise<SBasis> {
  public:
    vector<double> cuts;
    vector<SBasis> segs;
    //segs[i] stretches from cuts[i] to cuts[i+1].

    Piecewise() {}

    explicit Piecewise(const SBasis &sb) {
        push_cut(0.);
        push_seg(sb);
        push_cut(1.);
    }

    inline SBasis operator[](unsigned i) const { return segs[i]; }
    inline SBasis &operator[](unsigned i) { return segs[i]; }
    inline double operator()(double t) const {
        int n = segn(t);
        return segs[n](segt(t, n));
    }
    inline unsigned size() const { return segs.size(); }
    inline bool empty() const { return segs.empty(); }

    inline void push(SBasis seg, double to) {
        assert(cuts.size() != 0);
        push_seg(seg);
        push_cut(to);
    }

    inline void push_cut(double c) {
        assert(cuts.empty() || c > cuts.back()); 
        cuts.push_back(c);
    }
    inline void push_seg(const SBasis &s) { segs.push_back(s); }

    inline int segn(double t, int low = 0) const {
        int high = size();
        if(t < cuts[0]) return 0;
        if(t >= cuts[size()]) return size() - 1;
        while(low < high) {
            int mid = (high + low) / 2;
            double mv = cuts[mid];
            if(mv < t) {
                if(t < cuts[mid + 1]) return mid; else low = mid + 1;
            } else if(t < mv) {
                if(cuts[mid - 1] < t) return mid - 1; else high = mid - 1;
            } else {
                return mid;
            }
        }
        return low;
    }
    
    inline double segt(double t, int i = -1) const {
        if(i == -1) i = segn(t);
        return (t - cuts[i]) / (cuts[i+1] - cuts[i]);
    }

    inline void offsetDomain(double o) {
        for(int i = 0; i <= size(); i++)
            cuts[i] += o;
    }

    inline void scaleDomain(double s) {
        for(int i = 0; i <= size(); i++)
            cuts[i] *= s;
    }

    inline void setDomain(double from, double to) {
        if(empty()) return;
        double cf = cuts.front(), o = from - cf, s = (to - from) / (cuts.back() - cf);
        for(int i = 0; i <= size(); i++)
            cuts[i] = (cuts[i] - cf) * s + o;
    }

    inline void concat(const Piecewise &other) {
        if(empty()) return;
        double t = cuts.back() - other.cuts.front();
        for(int i = 0; i < other.size(); i++)
            push(other[i], other.cuts[i + 1] + t);
    }

    inline void continuousConcat(const Piecewise &other) {
        if(empty()) return;
        double t = cuts.back() - other.cuts.front(), y = segs.back()[0][1] - other.segs.front()[0][0];
        for(int i = 0; i < other.size(); i++)
            push(y + other[i], other.cuts[i + 1] + t);
    }

    bool invariants() const;
};

Piecewise<SBasis> partition(const Piecewise<SBasis> &t, vector<double> const &c);
Piecewise<SBasis> portion(const Piecewise<SBasis> &a, double from, double to);

vector<double> roots(const Piecewise<SBasis> &a);

inline Piecewise<SBasis> operator+(Piecewise<SBasis> const &a) { return a; }

Piecewise<SBasis> operator+(Piecewise<SBasis> const &a, double b);
Piecewise<SBasis> operator-(Piecewise<SBasis> const &a);

Piecewise<SBasis> operator+=(Piecewise<SBasis>& a, double b);
Piecewise<SBasis> operator-=(Piecewise<SBasis>& a, double b);
Piecewise<SBasis> operator*=(Piecewise<SBasis>& a, double b);
Piecewise<SBasis> operator/=(Piecewise<SBasis>& a, double b);

Piecewise<SBasis> operator+(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b);
Piecewise<SBasis> operator-(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b);
Piecewise<SBasis> multiply(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b);
Piecewise<SBasis> divide(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b, int k);

Piecewise<SBasis> compose(Piecewise<SBasis> const &a, SBasis const &b);
Piecewise<SBasis> compose(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b);

inline Piecewise<SBasis> operator*(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b) { multiply(a, b); }
inline Piecewise<SBasis> operator*=(Piecewise<SBasis> &a, Piecewise<SBasis> const &b) { 
    a = multiply(a, b);
    return a;
}

Piecewise<SBasis> integral(Piecewise<SBasis> const &a);
Piecewise<SBasis> derivative(Piecewise<SBasis> const &a);

}

#endif //SEEN_GEOM_PW_SB_H
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
