#include "path.h"

/** A convex cover is a sequence of convex polygons that completely cover the path.  For now a
 * convex hull class is included here (the convex-hull header is wrong)
 */

namespace Geom{

/** ConvexHull
 * A convexhull is a convex region - every point between two points in the convex hull is also in
 * the convex hull.  It is defined by a set of points travelling in a clockwise direction.  We require the first point to be top most, and of the topmost, leftmost.

 * An empty hull has no points, we allow a single point or two points degenerate cases.

 * We provide the centroid as a member for efficient direction determination.  We can update the
 * centroid with all operations with the same time complexity as the operation or better.
 */
class ConvexHull{
public:
    std::vector<Point> boundary;
    Point centroid;
    
    void merge(Point p);
    bool contains_point(Point p);
    
    /** Is the convex hull clockwise?  We use the definition of clockwise from point.h
    **/
    bool is_clockwise() const;
    bool no_colinear_points() const;
    bool top_point_first() const;
    bool meets_invariants() const;
    
    bool empty() const { return boundary.empty();}
    bool is_degenerate() const;
};

bool intersectp(ConvexHull a, ConvexHull b);
ConvexHull intersection(ConvexHull a, ConvexHull b);
ConvexHull merge(ConvexHull a, ConvexHull b);


/*** Arbitrary transform operator.
 * Take a convex hull and apply an arbitrary convexity preserving transform.
 *  we should be concerned about singular tranforms here.
 */
template <class T> ConvexHull operator*(ConvexHull const &p, T const &m) {
    ConvexHull pr;
    
    pr.cmd = p.cmd;
    pr.boundary.reserve(p.boundary.size());
    
    for(unsigned i = 0; i < p.boundary.size(); i++) {
        pr.boundary.push_back(p.boundary[i]*m);
    }
    return pr;
}

class ConvexCover{
public:
    SubPath* path;
    
    
};

};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

