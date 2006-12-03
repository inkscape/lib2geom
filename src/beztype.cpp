//"http://jgt.akpeters.com/papers/Vincent02/BezType.html"
/*
 * Implementation of the algorithm described in:
 *
 *      Stephen Vincent.
 *      Fast Detection of the Geometric Form of Two-Dimensional Cubic B&eacute;zier Curves.
 *      Journal of Graphics Tools, 7(3):43-51, 2002
 *
 * See the paper for discussion of the algorithm.
 */

#include <math.h>
#include "point.h"
#include "point-ops.h"
#include "math-utils.h"
#include "bezier-utils.h"

//TODO: header file?
namespace Geom {

enum BezType { BezTypeArch, BezTypeCusp, BezTypeStraightLine,
               BezTypeSingleInflection, BezTypeDoubleInflection, BezTypeSelfIntersecting};

static inline Point mulp(Point p0, Point p1) {return Point(p0[0]*p1[0], p0[1]*p1[1]);}

//Funky function
static double determ(Point p0, Point p1, Point p2 ) {
    Point p = mulp(p1 - p0, p2 - p0);
    return p[0] - p[1];
}

/* Determines the position of a point p2 with respect to a line defined by p0 and p1. Returns the
 * closest point on the line to p2. If it lies before p0 return -1 , if it lies after p1 return 1 :
 * otherwise return 0.
 */
static short PointRelativeToLine (Point p0, Point p1, Point p2 ) {
    Point p;

    p = mulp(p1 - p0, p2 - p0);
    if (p[0] + p[1] <= 0) return -1;
    
    p = mulp(p0 - p1, p2 - p1);
    if (p[0] + p[1] > 0) return 0;
    
    return 1;
}

/** Distinguish between loop, cusp, and 2 inflection point curves */
static short CuspValue ( double det_012 , double det_013 , double det_023 ) {
    double a = 3*det_012 + det_023 - 2*det_013,
           b = -3*det_012 +det_013,
           d = b*b - 4*a*det_012;

    //An epsilon would be faster than calculating this, but less accurate.
    double minc = sqrt(d) / 2*a;

    return (d>minc)? BezTypeDoubleInflection : ((d < -minc)? BezTypeSelfIntersecting : BezTypeCusp);
}

int GetBezierType (Point p[]) {   
    double det_012 = determ (p[0], p[1], p[2]),
           det_123 = determ (p[1], p[2], p[3]),
           det_013 = determ (p[0], p[1], p[3]),
           det_023 = determ (p[0], p[2], p[3]);

    int sign_012 = sgn(det_012),
        sign_123 = sgn(det_123),
        sign_013 = sgn(det_013),
        sign_023 = sgn(det_023);

    //First address the cases where 3 or more consecutive control points are colinear.
    //Perhaps we could distinguish all 4 equal as well.
    if ( sign_012 == 0 && sign_123 == 0 ) return (sign_013 == 0) ? BezTypeStraightLine : BezTypeArch;
    //Case F : first 3 control points are colinear
    if ( sign_012 == 0 ) return (sign_013 == sign_123) ? BezTypeArch : BezTypeSingleInflection;
    if ( sign_123 == 0) return  (sign_023 == sign_012) ? BezTypeArch : BezTypeSingleInflection;
    //Case G : points 0,1,3 are colinear
    if ( sign_013 == 0 ) {
        switch ( PointRelativeToLine (p[0], p[3], p[1]) ) {
            case -1: return BezTypeArch;
            case  0: return BezTypeSingleInflection;
            default: return CuspValue(det_012, det_013, det_023);
        }
    }
    if ( sign_023 == 0 ) {
    //Case G : points 0,2,3 are colinear
        switch ( PointRelativeToLine (p[0], p[3], p[2]) ) {
            case  1: return BezTypeArch;
            case  0: return BezTypeSingleInflection;
            default: return CuspValue(det_012, det_013, det_023);
        }
    }

    //	OK : on to the more interesting stuff. At this point it's known that
    //	no 3 of the control points are colinear

    //Case A : the control points zig-zag
    if (sign_012 != sign_123) return BezTypeSingleInflection;
    //Case B : Convex control polygon
    if (sign_012 == sign_013 && sign_012 == sign_023) return BezTypeArch;
    //Case C : Self-intersecting control polygon
    if (sign_012 != sign_013 && sign_012 != sign_023) return CuspValue(det_012, det_013, det_023);
    
    if (PointRelativeToLine ( p[0], p[3], bezier_pt(3, p, det_013 / ( det_013 - det_023 )) ) == 0)
        return CuspValue ( det_012 , det_013 , det_023 );
    else
        return BezTypeArch;
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

