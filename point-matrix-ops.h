/** \file operator functions over (Geom::Point, Geom::Matrix). */
#ifndef SEEN_Geom_POINT_MATRIX_OPS_H
#define SEEN_Geom_POINT_MATRIX_OPS_H

#include "point.h"
#include "matrix.h"

namespace Geom {

inline Point operator*(Point const &v, Matrix const &m) {
    Point ret;
    for(int i = 0; i < 2; i++) {
        ret[i] = v[X] * m[i] + v[Y] * m[i + 2] + m[i + 4];
    }
    return ret;
}

inline Point &Point::operator*=(Matrix const &m)
{
    *this = *this * m;
    return *this;
}

} /* namespace Geom */


#endif /* !SEEN_Geom_POINT_MATRIX_OPS_H */

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
