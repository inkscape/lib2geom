#include "path.h"

namespace Geom{
class Bezier {
public:
    Geom::Point p[4];
    Bezier * Split();
    void ParameterSplitLeft( double t, Bezier &result );
    
    ~Bezier() {}
};

std::vector<std::pair<double, double> > FindIntersections( Bezier a, Bezier b);
};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

