#ifndef SEEN_Geom_ROTATE_FNS_H
#define SEEN_Geom_ROTATE_FNS_H

/** \file
 * Declarations for rotation functions.
 */

#include "rotate.h"
#include "point-fns.h"

inline bool Rotate_equalp(Geom::Rotate const &a, Geom::Rotate const &b, double const eps)
{
    return point_equalp(a.vec, b.vec, eps);
}

Geom::Rotate Rotate_degrees(double degrees);

#endif /* !SEEN_Geom_ROTATE_FNS_H */

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
