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

}

#endif //SEEN_GEOM_SB_PW_H
