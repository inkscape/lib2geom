/** \file
 * Functions to/from Geom::Rotate.
 */
#include "rotate-ops.h"
#include "rotate-fns.h"

#define g_return_val_if_fail(a, b) if(!(a)) return (b);

/**
 * Returns a rotation matrix corresponding by the specified angle about the origin.
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 */
Geom::Rotate
Rotate_degrees(double degrees)
{
    if (degrees < 0) {
        return Rotate_degrees(-degrees).inverse();
    }

    double const degrees0 = degrees;
    if (degrees >= 360) {
        degrees = fmod(degrees, 360);
    }

    Geom::Rotate ret(1., 0.);

    if (degrees >= 180) {
        Geom::Rotate const rot180(-1., 0.);
        degrees -= 180;
        ret = rot180;
    }

    if (degrees >= 90) {
        Geom::Rotate const rot90(0., 1.);
        degrees -= 90;
        ret *= rot90;
    }

    if (degrees == 45) {
        Geom::Rotate const rot45(M_SQRT1_2, M_SQRT1_2);
        ret *= rot45;
    } else {
        double const radians = M_PI * ( degrees / 180 );
        ret *= Geom::Rotate(cos(radians), sin(radians));
    }

    Geom::Rotate const raw_ret( M_PI * ( degrees0 / 180 ) );
    g_return_val_if_fail(Rotate_equalp(ret, raw_ret, 1e-8),
                         raw_ret);
    return ret;
}


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
