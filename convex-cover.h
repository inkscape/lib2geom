#include "path.h"

/** A convex cover is a sequence of convex polygons that completely
 * cover the path.  For now a convex hull class is included here.
 */

namespace Geom{

/** ConvexHull
 * A convexhull is a convex region - every point between two points in the convex hull is also in
 * the convex hull.  It is defined by a set of points travelling in a clockwise direction.
 */
class ConvexHull{
public:
    std::vector<Point> boundary;
    
    void add_point(Point p);
    bool contains_point(Point p);
};

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

