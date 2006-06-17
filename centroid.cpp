#include "path.h"
#include "poly.h"
#include "arc-length.h"


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

namespace Geom{
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

static Poly
cubic_bezier_poly(SubPath::Elem const & b, int dim) {
    Poly result;
    double c[10] = {1, 
                    -3, 3, 
                    3, -6, 3,
                    -1, 3, -3, 1};

    int cp = 0;
    
    result.coeff.resize(4);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j <= i; j++) {
            result.coeff[3 - j] += (c[cp]*(b[3- i]))[dim];
            cp++;
        }
    }
    return result;
}


static Poly
quadratic_bezier_poly(SubPath::Elem const & b, int dim) {
    Poly result;
    double c[6] = {1, 
                    -2, 2, 
                    1, -2, 1};

    int cp = 0;
    
    result.coeff.resize(3);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j <= i; j++) {
            result.coeff[2 - j] += (c[cp]*(b[2- i]))[dim];
            cp++;
        }
    }
    return result;
}



int centroid(SubPath const &p, Point& centroid, double &area) {
    Point centroid_tmp(0,0);
    double atmp = 0;
    for(SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        SubPath::Elem elm = *iter;
        switch(iter.cmd()) {
            case Geom::moveto:
                break;
            case Geom::lineto:
            {
                const double ai = cross(elm.first(), elm.last());
                atmp += ai;
                centroid_tmp += ai*(elm.first() + elm.last()); // first moment.
                break;
            }
            case Geom::quadto:
            {
                Poly Bx = quadratic_bezier_poly(elm, X); // poly version of bezier (0-1)
                Poly By = quadratic_bezier_poly(elm, Y);
                Poly dBx = derivative(Bx);
                Poly dBy = derivative(By);
                Poly curl = By*dBx -Bx*dBy;
                Poly A = integral(curl);
                Poly Cx = integral(curl*Bx);
                Poly Cy = integral(curl*By);
                const double ai = A(1); // we don't need to subtract the constant as integral makes the constant 0.
                atmp += ai;
                centroid_tmp += 2*Point(Cx(1), Cy(1)); // first moment.
                break;
            }
            case Geom::cubicto:
            {
                Poly Bx = cubic_bezier_poly(elm, X); // poly version of bezier (0-1)
                Poly By = cubic_bezier_poly(elm, Y);
                Poly dBx = derivative(Bx);
                Poly dBy = derivative(By);
                Poly curl = By*dBx -Bx*dBy;
                Poly A = integral(curl);
                Poly Cx = integral(curl*Bx);
                Poly Cy = integral(curl*By);
                const double ai = A(1); // we don't need to subtract the constant as integral makes the constant 0.
                atmp += ai;
                centroid_tmp += 2*Point(Cx(1), Cy(1)); // first moment.
                break;
            }
            default:
                break;
        }
    }
    // join ends
    const double ai = cross(p.handles.back(), p.handles[0]);
    atmp += ai;
    centroid_tmp += ai*(p.handles.back(), p.handles[0]); // first moment.
    
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
}

int dcentroid(SubPath const &p, Point& dcentroid) {
    Point centroid_tmp(0,0);
    for(SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        SubPath::Elem elm = *iter;
        switch(iter.cmd()) {
            case Geom::moveto:
                break;
            case Geom::lineto:
            {
                centroid_tmp += (elm.first() + elm.last())/2;
                break;
            }
            case Geom::cubicto:
            {
                Poly Bx = cubic_bezier_poly(elm, X); // poly version of bezier (0-1)
                Poly By = cubic_bezier_poly(elm, Y);
                Poly dBx = derivative(Bx);
                Poly dBy = derivative(By);
                Poly Cx = integral(Bx + 0.5*dBx);
                Poly Cy = integral(By + 0.5*dBy);
                centroid_tmp += Point(Cx(1), Cy(1)); // first moment.
                break;
            }
            default:
                break;
        }
    }
    
    dcentroid = centroid_tmp /arc_length_integrating(p, 0.01);
    return 0;
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
