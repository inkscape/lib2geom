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
    inline unsigned size() const { return segs.size(); }

    bool cheap_invariants() const;
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
