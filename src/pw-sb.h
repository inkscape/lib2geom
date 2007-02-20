#ifndef SEEN_GEOM_SB_PW_H
#define SEEN_GEOM_SB_PW_H

#include "s-basis.h"
#include <vector>

using namespace std;

namespace Geom {

class pw_sb {
  public:
    vector<double> cuts;
    vector<SBasis> segs; 
    //segs[i] stretches from cuts[i] to cuts[i+1].

    inline SBasis operator[](unsigned i) const { return segs[i]; }
    inline SBasis &operator[](unsigned i) { return segs[i]; }
    inline double operator()(double t) const {
        int n = segn(t);
        return segs[n](segt(t, n));
    }
    inline unsigned size() const { return segs.size(); }

    inline int segn(double t) const {
        int low = 0, high = size();
        if(t < cuts[0]) return 0;
        if(t > cuts[size()]) return size() - 1;
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

    bool cheap_invariants() const;
    bool invariants() const;
};

pw_sb partition(const pw_sb &t, vector<double> const &c);
pw_sb portion(const pw_sb &a, double from, double to);

vector<double> roots(const pw_sb &a);

pw_sb operator-(pw_sb const &a);
pw_sb operator-(BezOrd const &b, const pw_sb&a);

pw_sb operator+=(pw_sb& a, const BezOrd& b);
pw_sb operator+=(pw_sb& a, double b);
pw_sb operator-=(pw_sb& a, const BezOrd& b);
pw_sb operator-=(pw_sb& a, double b);

pw_sb operator+(pw_sb const &a, pw_sb const &b);
pw_sb operator-(pw_sb const &a, pw_sb const &b);
pw_sb multiply(pw_sb const &a, pw_sb const &b);
pw_sb divide(pw_sb const &a, pw_sb const &b, int k);

pw_sb compose(pw_sb const &a, pw_sb const &b);

inline pw_sb operator*(pw_sb const &a, pw_sb const &b) { multiply(a, b); }
inline pw_sb operator*=(pw_sb &a, pw_sb const &b) { 
    a = multiply(a, b);
    return a;
}

}

#endif //SEEN_GEOM_SB_PW_H
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
