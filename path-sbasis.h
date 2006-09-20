#ifndef _PATH_SBASIS_
#define _PATH_SBASIS_

#include "path.h"
#include "s-basis.h"
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"

inline multidim_sbasis<2> elem_to_sbasis(Geom::SubPath::Elem const & elm) {
    switch(elm.op) {
    case Geom::lineto:
        return bezier_to_sbasis<2, 1>(elm.begin());
    case Geom::quadto:
        return bezier_to_sbasis<2, 2>(elm.begin());
    case Geom::cubicto:
        return bezier_to_sbasis<2, 3>(elm.begin());
    default:
        break;
    }
    return multidim_sbasis<2>();

}

#endif
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

