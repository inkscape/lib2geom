#include "convex-cover.h"

namespace Geom{

double SignedTriangleArea(Point p0, Point p1, Point p2) {
    return cross((p1 - p0), (p2 - p0))/2;
}

/*** ConvexHull::add_point
 * to add a point we need to find whether the new point extends the boundary, and if so, what it
 * obscures.  Tarjan?  Jarvis?*/

void ConvexHull::add_point(Point p) {
    
}

/*** ConvexHull::contains_point
 * to test whether a point is inside a convex hull we can travel once around the outside making
 * sure that each triangle made from an edge and the point has positive area. */

bool ConvexHull::contains_point(Point p) {
    Point op = boundary[0];
    for(std::vector<Point>::iterator it(boundary.begin()), e(boundary.end());
        it != e;) {
        ++it;
        if(SignedTriangleArea(op, *it, p) < 0)
            return false;
        op = *it;
    }
    return true;
}

ConvexHull intersection(ConvexHull a, ConvexHull b);
ConvexHull merge(ConvexHull a, ConvexHull b);

// we should be concerned about singular tranforms here.
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
    Path* path;
    
    
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


