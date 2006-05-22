#ifndef SEEN_Geom_ROTATE_H
#define SEEN_Geom_ROTATE_H

/** \file
 * Rotation about the origin.
 */

#include "point.h"
#include "point-fns.h"
#include "point-ops.h"

namespace Geom {

/** Notionally an Geom::Matrix corresponding to rotation about the origin.
    Behaves like Geom::Matrix for multiplication.
**/
class rotate {
public:
    Point vec;

private:
    rotate();

public:
    explicit rotate(Coord theta);
    explicit rotate(Point const &p) : vec(p) {}
    explicit rotate(Coord const x, Coord const y) : vec(x, y) {}

    bool operator==(rotate const &o) const {
        return vec == o.vec;
    }

    bool operator!=(rotate const &o) const {
        return vec != o.vec;
    }

    inline rotate &operator*=(rotate const &b);
    /* Defined in rotate-ops.h. */

    rotate inverse() const {
        /** \todo
         * In the usual case that vec is a unit vector (within rounding error),
         * dividing by len_sq is either a noop or numerically harmful. 
         * Make a unit_rotate class (or the like) that knows its length is 1.
         */
        double const len_sq = dot(vec, vec);
        return rotate( Point(vec[X], -vec[Y])
                       / len_sq );
    }
};

} /* namespace Geom */


#endif /* !SEEN_Geom_ROTATE_H */

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
