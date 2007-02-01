/*
 * centroid.cpp
 *
 * Portions Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#include "path.h"
#include "multidim-sbasis.h"
#include "arc-length.h"


namespace Geom{

/*
 * ANSI C code from the article
 * "Centroid of a Polygon"
 * by Gerard Bashein and Paul R. Detmer,
 (gb@locke.hs.washington.edu, pdetmer@u.washington.edu)
 * in "Graphics Gems IV", Academic Press, 1994
 */

/**
 * polyCentroid: Calculates the centroid (xCentroid, yCentroid) and area of a polygon, given its
 * vertices (x[0], y[0]) ... (x[n-1], y[n-1]). It is assumed that the contour is closed, i.e., that
 * the vertex following (x[n-1], y[n-1]) is (x[0], y[0]).  The algebraic sign of the area is
 * positive for counterclockwise ordering of vertices in x-y plane; otherwise negative.

 * Returned values: 
    0 for normal execution; 
    1 if the polygon is degenerate (number of vertices < 3);
    2 if area = 0 (and the centroid is undefined).

    * for now we require the path to be a polyline and assume it is closed.
**/

int centroid(std::vector<Point> p, Point& centroid, double &area) {
    const unsigned n = p.size();
    if (n < 3)
        return 1;
    Point centroid_tmp(0,0);
    double atmp = 0;
    for (int i(n-1), j(0); j < n; i = j, j++) {
        const double ai = -cross(p[j], p[i]);
        atmp += ai;
        centroid_tmp += ai*(p[j] + p[i]); // first moment.
    }
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
}

/** centroid using sbasis integration.
 * This approach uses green's theorem to compute the area and centroid using integrals.  For curved
 * shapes this is much faster than converting to polyline and using the above function.
 *
 * Copyright Nathan Hurst 2006
 */

int centroid(Path const &p, Point& centroid, double &area) {
    Point centroid_tmp(0,0);
    double atmp = 0;
    for(Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        Path::Elem elm = *iter;
        MultidimSBasis<2> B = (*elm.op).to_sbasis(elm);
        MultidimSBasis<2> dB = rot90(derivative(B));
        SBasis curl = dot(B, dB);
        SBasis A = integral(curl);
        MultidimSBasis<2> C = integral(multiply(curl, B));
        atmp += A(1) - A(0);
        centroid_tmp += point_at(C, 1)- point_at(C, 0); // first moment.
    }
// join ends
    centroid_tmp *= 2;
    const double ai = cross(p.final_point(), p.initial_point());
    atmp += ai;
    centroid_tmp += ai*(p.final_point(), p.initial_point()); // first moment.
    
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
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
