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
        if(t < cuts[0]) return 0;
        if(t > cuts[size()]) return size() - 1;
        for(int i = 0; i <= size(); i++) {
            if(cuts[i] <= t && (i == size() || t < cuts[i+1])) return i;
        }
    }
    
    inline double segt(double t, int i = -1) const {
        if(i == -1) i = segn(t);
        return (t - cuts[i]) / (cuts[i+1] - cuts[i]);
    }

    bool cheap_invariants() const;
    bool invariants() const;
};

pw_sb partition(const pw_sb &t, vector<double> const &c);

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
