#ifndef SEEN_LIBGeom_Geom_MATRIX_DIV_H
#define SEEN_LIBGeom_Geom_MATRIX_DIV_H

#include "libnr/forward.h"

Geom::Point operator/(Geom::Point const &, Geom::Matrix const &);

Geom::Matrix operator/(Geom::Matrix const &, Geom::Matrix const &);

#endif /* !SEEN_LIBGeom_Geom_MATRIX_DIV_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
