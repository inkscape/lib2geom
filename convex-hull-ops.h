#ifndef SEEN_Geom_CONVEX_HULL_FNS_H
#define SEEN_Geom_CONVEX_HULL_FNS_H

/* ex:set et ts=4 sw=4: */

/*
 * A class representing the convex hull of a set of points.
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/matrix-fns.h>
#include <libnr/convex-hull.h>

namespace Geom {

ConvexHull operator*(const Rect &r, const Matrix &m) {
    ConvexHull points(r.corner(0));
    for ( unsigned i = 1 ; i < 4 ; i++ ) {
        points.add(r.corner(i));
    }
    return points;
}

} /* namespace Geom */

#endif
