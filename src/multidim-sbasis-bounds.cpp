/** root finding for sbasis functions.
 * Copyright 2006 N Hurst
 *
 */

#include <cmath>
#include <map>

#include "multidim-sbasis-bounds.h"

namespace Geom{

Rect
local_bounds(MultidimSBasis<2> const & s, 
             double t0, double t1, 
             int order) {
    Point mn, mx;
    for(int d = 0; d < 2; d++)
        local_bounds(s[d], t0, t1, mn[d], mx[d], order);
    return Rect::define(mn,mx);
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
