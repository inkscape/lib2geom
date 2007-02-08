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
};

pw_sb partition(vector<double> const &c);

pw_sb operator-(pw_sb const &a);
pw_sb operator-(BezOrd const &b, const pw_sb&a);

pw_sb operator+=(pw_sb& a, const BezOrd& b);
pw_sb operator+=(pw_sb& a, double b);
pw_sb operator-=(pw_sb& a, const BezOrd& b);
pw_sb operator-=(pw_sb& a, double b);

pw_sb operator+(pw_sb const &a, pw_sb const &b);
pw_sb operator-(pw_sb const &a, pw_sb const &b);
pw_sb multiply(pw_sb const &a, pw_sb const &b);

pw_sb compose(pw_sb const &a, pw_sb const &b);

inline pw_sb operator*(pw_sb const &a, pw_sb const &b) { multiply(a, b); }
inline pw_sb operator*=(pw_sb &a, pw_sb const &b) { 
    a = multiply(a, b);
    return a;
}

}

#endif //SEEN_GEOM_SB_PW_H
