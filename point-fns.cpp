#include "point-fns.h"
#include "isnan.h"

using Geom::Point;

/** Compute the L infinity, or maximum, norm of \a p. */
Geom::Coord Geom::LInfty(Point const &p) {
    Geom::Coord const a(fabs(p[0]));
    Geom::Coord const b(fabs(p[1]));
    return ( a < b || isNaN(b)
             ? b
             : a );
}

/** Returns true iff p is a zero vector, i.e.\ Point(0, 0).
 *
 *  (NaN is considered non-zero.)
 */
bool
Geom::is_zero(Point const &p)
{
    return ( p[0] == 0 &&
             p[1] == 0   );
}

bool
Geom::is_unit_vector(Point const &p)
{
    return fabs(1.0 - L2(p)) <= 1e-4;
    /* The tolerance of 1e-4 is somewhat arbitrary.  Geom::Point::normalize is believed to return
       points well within this tolerance.  I'm not aware of any callers that want a small
       tolerance; most callers would be ok with a tolerance of 0.25. */
}

Geom::Coord Geom::atan2(Point const p) {
    return std::atan2(p[Geom::Y], p[Geom::X]);
}

/** compute the angle turning from a to b.  This should give \f$\pi/2\f$ for angle_between(a, rot90(a));
 * This works by projecting b onto the basis defined by a, rot90(a)
 */
Geom::Coord Geom::angle_between(Point const a, Point const b) {
    return std::atan2(cross(b,a), dot(b,a));
}



/** Returns a version of \a a scaled to be a unit vector (within rounding error).
 *
 *  The current version tries to handle infinite coordinates gracefully,
 *  but it's not clear that any callers need that.
 *
 *  \pre a != Point(0, 0).
 *  \pre Neither coordinate is NaN.
 *  \post L2(ret) very near 1.0.
 */
Point Geom::unit_vector(Point const &a)
{
    Point ret(a);
    ret.normalize();
    return ret;
}

Geom::Point abs(Geom::Point const &b)
{
    Geom::Point ret;
    for ( int i = 0 ; i < 2 ; i++ ) {
        ret[i] = fabs(b[i]);
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
