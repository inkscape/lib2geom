#include "path.h"

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
int centroid(std::vector<Point> p, Point centroid, double &area) {
    if (n < 3)
        return 1;
    Point centroid_tmp(0,0);
    double atmp = 0;
    for (int i(n-1), j(0); j < n; i = j, j++) {
        const double ai = cross(p[j], p[i]);
        atmp += ai;
        centroid_tmp += (p[j] + p[i]) * ai; // first moment.
    }
    *area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid / (3 * atmp);
        return 0;
    }
    return 2;
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
