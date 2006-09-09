#include "convex-cover.h"
#include <algorithm>

namespace Geom{

/*** SignedTriangleArea
 * returns the area of the triangle defined by p0, p1, p2.  A clockwise triangle has positive area.
 */
double
SignedTriangleArea(Point p0, Point p1, Point p2) {
    return cross((p1 - p0), (p2 - p0));
}

/*
template <typename T>
void
swap(T &a, T &b) {
    T t = a;
    a = b;
    b = t;
    }
*/
class angle_cmp{
public:
    Point o;
    angle_cmp(Point o) : o(o) {}
    
    bool
    operator()(Point a, Point b) {
        Point da = a - o;
        Point db = b - o;
        
#if 1
        double aa = db[0];
        if(da[1])
            aa = da[0] / da[1];
        double ab = db[0];
        if(db[1])
            ab = db[0] / db[1];
        if(aa > ab)
            return true;
#else
        //assert((ata > atb) == (aa < ab));
        double aa = atan2(da);
        double ab = atan2(db);
        if(aa < ab)
            return true;
#endif
        if(aa == ab)
            return L2(da) < L2(db);
        return false;
    }
};

ConvexHull::ConvexHull(std::vector<Point> const & points) {
    boundary = points;
    // Find pivot P;
    unsigned pivot = 0;
    for(unsigned i = 1; i < boundary.size(); i++) {
        if(boundary[i][1] < boundary[pivot][1])
            pivot = i;
        else if((boundary[i][1] == boundary[pivot][1]) && 
                (boundary[i][0] < boundary[pivot][0]))
            pivot = i;
    }
    std::swap(boundary[0], boundary[pivot]);
    
//Sort points by angle (resolve ties in favor of point farther from P);
    std::sort(boundary.begin()+1, boundary.end(), angle_cmp(boundary[0]));
    
    std::vector<Point> stac;
    stac.push_back(boundary[0]);
    stac.push_back(boundary[1]);
    for(int i = 2; i < boundary.size(); i++) {
        double o = -SignedTriangleArea(stac[stac.size()-2], stac.back(), boundary[i]);
        if(o == 0) {
            stac.pop_back();
            stac.push_back(boundary[i]);
        } else if(o > 0) {
            stac.push_back(boundary[i]);
        } else {
            while(o <= 0 && stac.size() > 2) {
                stac.pop_back();
                o = -SignedTriangleArea(stac[stac.size()-2], stac.back(), boundary[i]);
            }
            stac.push_back(boundary[i]);
        }
    }
    boundary = stac; // write results
}

/*** ConvexHull::add_point
 * to add a point we need to find whether the new point extends the boundary, and if so, what it
 * obscures.  Tarjan?  Jarvis?*/

void
ConvexHull::merge(Point p) {
    
}

/*** ConvexHull::contains_point
 * to test whether a point is inside a convex hull we can travel once around the outside making
 * sure that each triangle made from an edge and the point has positive area. */

bool
ConvexHull::contains_point(Point p) {
    Point op = boundary[0];
    for(std::vector<Point>::iterator it(boundary.begin()+1), e(boundary.end());
        it != e;) {
        if(SignedTriangleArea(op, *it, p) > 0)
            return false;
        op = *it;
        ++it;
    }
    return true;
}

/*** ConvexHull::is_clockwise
 * We require that successive pairs of edges always turn right.
 * proposed algorithm: walk successive edges and require triangle area is positive.
 */
bool
ConvexHull::is_clockwise() const {
    if(is_degenerate())
        return true;
    Point first = boundary[0];
    Point second = boundary[1];
    for(std::vector<Point>::const_iterator it(boundary.begin()+2), e(boundary.end());
        it != e;) {
        if(SignedTriangleArea(first, second, *it) > 0)
            return false;
        first = second;
        second = *it;
        ++it;
    }
    return true;    
}

/*** ConvexHull::top_point_first
 * We require that the first point in the convex hull has the least y coord, and that off all such points on the hull, it has the least x coord.
 * proposed algorithm: track lexicographic minimum while walking the list.
 */
bool
ConvexHull::top_point_first() const {
    std::vector<Point>::const_iterator pivot = boundary.begin();
    for(std::vector<Point>::const_iterator it(boundary.begin()+1), 
            e(boundary.end());
        it != e; it++) {
        if((*it)[1] < (*pivot)[1])
            pivot = it;
        else if(((*it)[1] == (*pivot)[1]) && 
                ((*it)[0] < (*pivot)[0]))
            pivot = it;
    }
    return pivot == boundary.begin();
}

/*** ConvexHull::no_colinear_points
 * We require that no three vertices are colinear.
proposed algorithm:  We must be very careful about rounding here.
*/
bool
ConvexHull::no_colinear_points() const {

}

bool
ConvexHull::meets_invariants() const {
    return is_clockwise() && top_point_first() && no_colinear_points();
}

/*** ConvexHull::is_degenerate
 * We allow three degenerate cases: empty, 1 point and 2 points.  In many cases these should be handled explicitly.
 */
bool
ConvexHull::is_degenerate() const {
    return boundary.size() < 3;
}

/*** ConvexHull intersection(ConvexHull a, ConvexHull b);
 * find the intersection between two convex hulls.  The intersection is also a convex hull.
 * (Proof: take any two points both in a and in b.  Any point between them is in a by convexity,
 * and in b by convexity, thus in both.  Need to prove still finite bounds.)
 */
ConvexHull intersection(ConvexHull a, ConvexHull b) {

}

/*** ConvexHull merge(ConvexHull a, ConvexHull b);
 * find the smallest convex hull that surrounds a and b.
 */
ConvexHull merge(ConvexHull a, ConvexHull b) {

}

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
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/


