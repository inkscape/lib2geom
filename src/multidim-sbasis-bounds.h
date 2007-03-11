#ifndef _MULTIDIM_SBASIS_BOUNDS
#define _MULTIDIM_SBASIS_BOUNDS

#include "multidim-sbasis.h"

#include "rect.h"

namespace Geom{
Geom::Rect
local_bounds(MultidimSBasis<2> const & s, 
             double t0, double t1, 
             int order=0);

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
