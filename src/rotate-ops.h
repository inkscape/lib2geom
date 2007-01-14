#ifndef SEEN_Geom_ROTATE_OPS_H
#define SEEN_Geom_ROTATE_OPS_H
#include "rotate.h"

namespace Geom {

inline Point operator*(Point const &v, Rotate const &r)
{
    return Point(r.vec[X] * v[X] - r.vec[Y] * v[Y],
                 r.vec[Y] * v[X] + r.vec[X] * v[Y]);
}

inline Rotate operator*(Rotate const &a, Rotate const &b)
{
    return Rotate( a.vec * b );
}

inline Rotate &Rotate::operator*=(Rotate const &b)
{
    *this = *this * b;
    return *this;
}

inline Rotate operator/(Rotate const &numer, Rotate const &denom)
{
    return numer * denom.inverse();
}

}  /* namespace Geom */


#endif /* !SEEN_Geom_ROTATE_OPS_H */

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
