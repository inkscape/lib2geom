#include "convex-cover.h"
#include <algorithm>

/** Todo:
    + modify graham scan to work top to bottom, rather than around angles
    + intersection
    + minimum distance between convex hulls
    + maximum distance between convex hulls
    + hausdorf metric?
    + check all degenerate cases carefully
    + check all algorithms meet all invariants
    + generalise rotating caliper algorithm (iterator/circulator?)
*/

namespace Geom{

/*** SignedTriangleArea
 * returns the area of the triangle defined by p0, p1, p2.  A clockwise triangle has positive area.
 */
double
SignedTriangleArea(Point p0, Point p1, Point p2) {
    return cross((p1 - p0), (p2 - p0));
}

class angle_cmp{
public:
    Point o;
    angle_cmp(Point o) : o(o) {}
    
    bool
    operator()(Point a, Point b) {
        Point da = a - o;
        Point db = b - o;
        
#if 1
        double aa = da[0];
        double ab = db[0];
        if((da[1] == 0) && (db[1] == 0))
            return da[0] < db[0];
        if(da[1] == 0)
            return true; // infinite tangent
        if(db[1] == 0)
            return false; // infinite tangent
        aa = da[0] / da[1];
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
            return L2sq(da) < L2sq(db);
        return false;
    }
};

void
ConvexHull::find_pivot() {
    // Find pivot P;
    unsigned pivot = 0;
    for(unsigned i = 1; i < boundary.size(); i++)
        if(boundary[i] <= boundary[pivot])
            pivot = i;
    
    std::swap(boundary[0], boundary[pivot]);
}

void
ConvexHull::angle_sort() {
// sort points by angle (resolve ties in favor of point farther from P);
// we leave the first one in place as our pivot
    std::sort(boundary.begin()+1, boundary.end(), angle_cmp(boundary[0]));
}

void
ConvexHull::graham_scan() {
    unsigned stac = 2;
    for(int i = 2; i < boundary.size(); i++) {
        double o = SignedTriangleArea(boundary[stac-2], 
                                      boundary[stac-1], 
                                      boundary[i]);
        if(o == 0) { // colinear - dangerous...
            stac--;
        } else if(o < 0) { // anticlockwise
        } else { // remove concavity
            while(o >= 0 && stac > 2) {
                stac--;
                o = SignedTriangleArea(boundary[stac-2], 
                                       boundary[stac-1], 
                                       boundary[i]);
            }
        }
        boundary[stac++] = boundary[i];
    }
    boundary.resize(stac);
}

void
ConvexHull::graham() {
    find_pivot();
    angle_sort();
    graham_scan();
}

//Mathematically incorrect mod, but more useful.
int mod(int i, int l) {
    return i >= 0 ? 
           i % l : (i % l) + l;
}
//OPT: usages can often be replaced by conditions

/*** ConvexHull::left
 * Tests if a point is left (outside) of a particular segment, n. */
bool
ConvexHull::is_left(Point p, int n) {
    return SignedTriangleArea((*this)[n], (*this)[n+1], p) > 0;
}

/*** ConvexHull::find_positive
 * May return any number n where the segment n -> n + 1 (possibly looped around) in the hull such
 * that the point is on the wrong side to be within the hull.  Returns -1 if it is within the hull.*/
int
ConvexHull::find_left(Point p) {
    int l = boundary.size(); //Who knows if C++ is smart enough to optimize this?
    for(int i = 0; i < l; i++) {
        if(is_left(p, i)) return i;
    }
    return -1;
}
//OPT: do a spread iteration - quasi-random with no repeats and full coverage. 

/*** ConvexHull::contains_point
 * In order to test whether a point is inside a convex hull we can travel once around the outside making
 * sure that each triangle made from an edge and the point has positive area. */
bool
ConvexHull::contains_point(Point p) {
    return find_left(p) == -1;
}

/*** ConvexHull::add_point
 * to add a point we need to find whether the new point extends the boundary, and if so, what it
 * obscures.  Tarjan?  Jarvis?*/
void
ConvexHull::merge(Point p) {
    std::vector<Point> out;

    int l = boundary.size();

    if(l < 2) {
        boundary.push_back(p);
        return;
    }

    bool pushed = false;

    bool pre = is_left(p, -1);
    for(int i = 0; i < l; i++) {
        bool cur = is_left(p, i);
        if(pre) {
            if(cur) {
                if(!pushed) {
                    out.push_back(p);
                    pushed = true;
                }
                continue;
            }
            else if(!pushed) {
                out.push_back(p);
                pushed = true;
            }
        }
        out.push_back(boundary[i]);
        pre = cur;
    }
    
    boundary = out;
}
//OPT: quickly find an obscured point and find the bounds by extending from there.  then push all points not within the bounds in order.
  //OPT: use binary searches to find the actual starts/ends, use known rights as boundaries.  may require cooperation of find_left algo.

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
//OPT: since the Y values are orderly there should be something like a binary search to do this.

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


/* Here we really need a rotating calipers implementation.  This implementation is slow and incorrect.
   This incorrectness is a problem because it throws off the algorithms.  Perhaps I will come up with
   something better tomorrow.*/
std::pair< std::vector<int>, std::vector<int> > bridges(ConvexHull a, ConvexHull b) {
    std::vector<int> abridges;
    std::vector<int> bbridges;

    for(int ia = 0; ia < a.boundary.size(); ia++) {
        for(int ib = 0; ib < b.boundary.size(); ib++) {
            Point d = b[ib] - a[ia];
            Geom::Coord e = cross(d, a[ia - 1] - a[ia]), f = cross(d, a[ia + 1] - a[ia]);
            Geom::Coord g = cross(d, b[ib - 1] - a[ia]), h = cross(d, b[ib + 1] - a[ia]);
            if       (e > 0 && f > 0 && g > 0 && h > 0) {
                abridges.push_back(ia);
                abridges.push_back(ib);
            } else if(e < 0 && f < 0 && g < 0 && h < 0) {
                bbridges.push_back(ib);
                bbridges.push_back(ia);
            }
        }
    }
    return make_pair(abridges, bbridges);
}

std::vector<Point> bridge_points(ConvexHull a, ConvexHull b) {
    std::vector<Point> ret;
    std::pair< std::vector<int>, std::vector<int> > indices = bridges(a, b);
    assert((indices.first.size() & 1) == 0);
    assert((indices.second.size() & 1) == 0);
    for(int i = 0; i < indices.first.size(); i += 2) {
        ret.push_back(a[indices.first[i]]);
        ret.push_back(b[indices.first[i + 1]]);
    }
    for(int i = 0; i < indices.second.size(); i += 2) {
        ret.push_back(b[indices.second[i]]);
        ret.push_back(a[indices.second[i + 1]]);
    }
    return ret;
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

/* Here's how it works at the moment:
   The bridge pair is retrieved:
      A->B  A->B    B->A  B->A
   { {0, 1, 3, 0}, {2, 3, 0, 0} }

   1. a. if a is on top then it is traversed until the first bridge to b.
      b. if b is on top then it is traversed until the first bridge to a.
   2. the current hull is traversed until the next bridge (index variables store where in the list we are).
   3. the next hull and bridge is begun - back to 2
   4. repeat until you run out of bridges, and then until you run out of points.
*/
ConvexHull merge(ConvexHull a, ConvexHull b) {
    ConvexHull ret;

    std::pair< std::vector<int>, std::vector<int> > bpair = bridges(a, b);
    
    int abi = 0, bbi = 0; //A and B bridge indexes
    int nexti = 0; //Next index

    if(a.boundary[0][1] > b.boundary[0][1]) goto start_b;
    while(true) {
        if(abi < bpair.first.size()) {
            for(int i = nexti; i <= bpair.first[abi]; i++)
                ret.boundary.push_back(a[i]);
            nexti = bpair.first[abi + 1];
        } else {
            for(int i = nexti; i < a.boundary.size(); i++)
                ret.boundary.push_back(a[i]);
            break;
        }
        abi += 2;

        start_b:

        if(bbi < bpair.second.size()) {
            for(int i = nexti; i <= bpair.second[bbi]; i++)
                ret.boundary.push_back(b[i]);
            nexti = bpair.second[bbi + 1];
        } else {
            for(int i = nexti; i < b.boundary.size(); i++)
                ret.boundary.push_back(b[i]);
            break;
        }
        bbi += 2;        
    }
    return ret;
}

ConvexHull graham_merge(ConvexHull a, ConvexHull b) {
    ConvexHull result;
    
    // we can avoid the find pivot step because of top_point_first
    if(b.boundary[0] <= a.boundary[0])
        std::swap(a, b);
    
    result.boundary = a.boundary;
    result.boundary.insert(result.boundary.end(), 
                           b.boundary.begin(), b.boundary.end());
    
/** if we modified graham scan to work top to bottom as proposed in lect754.pdf we could replace the
 angle sort with a simple merge sort type algorithm. furthermore, we could do the graham scan
 online, avoiding a bunch of memory copies.  That would probably be linear. -- njh*/
    result.angle_sort();
    result.graham_scan();
    
    return result;
}

ConvexCover::ConvexCover(SubPath const &sp) : path(&sp) {
    cc.reserve(sp.size());
    for(Geom::SubPath::const_iterator it(sp.begin()), end(sp.end()); it != end; ++it) {
        cc.push_back(ConvexHull((*it).begin(), (*it).end()));
    }
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


