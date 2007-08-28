#include "path-intersection.h"

#include "ord.h"

//for path_direction:
#include "sbasis-geometric.h"

namespace Geom {

/* This function computes the winding of the path, given a reference point.
 * Positive values correspond to counter-clockwise in the mathematical coordinate system,
 * and clockwise in screen coordinates.  This particular implementation casts a ray in
 * the positive x direction.  It iterates the path, checking for intersection with the
 * bounding boxes.  If an intersection is found, the initial/final Y value of the curve is
 * used to derive a delta on the winding value.  If the point is within the bounding box,
 * the curve specific winding function is called.
 */
int winding(Path const &path, Point p) {
  //start on a segment which is not a horizontal line with y = p[y]
  Path::const_iterator start;
  for(Path::const_iterator iter = path.begin(); ; ++iter) {
    if(iter == path.end_closed()) { return 0; }
    if(iter->initialPoint()[Y]!=p[Y])  { start = iter; break; }
    if(iter->finalPoint()[Y]!=p[Y])    { start = iter; break; }
    if(iter->boundsFast().height()!=0.){ start = iter; break; }
  }
  int wind = 0;
  bool starting = true;
  for (Path::const_iterator iter = start; iter != start || starting
       ; ++iter, iter = (iter == path.end_closed()) ? path.begin() : iter )
  {
    starting = false;
    Rect bounds = iter->boundsFast();
    Coord x = p[X], y = p[Y];
    
    if(x > bounds.right() || !bounds[Y].contains(y)) continue; //ray doesn't intersect box
    
    Point final = iter->finalPoint();
    Point initial = iter->initialPoint();
    Cmp final_to_ray = cmp(final[Y], y);
    Cmp initial_to_ray = cmp(initial[Y], y);
    
    // if y is included, these will have opposite values, giving order.
    Cmp c = cmp(final_to_ray, initial_to_ray); 
    if(x < bounds.left()) {
        // ray goes through bbox
        // winding delta determined by position of endpoints
        if(final_to_ray != EQUAL_TO) {
            wind += int(c); // GT = counter-clockwise = 1; LT = clockwise = -1; EQ = not-included = 0
            //std::cout << int(c) << " ";
            goto cont;
        }
    } else {
        //inside bbox, use custom per-curve winding thingie
        int delt = iter->winding(p);
        wind += delt;
        //std::cout << "n" << delt << " ";
    }
    //Handling the special case of an endpoint on the ray:
    if(final[Y] == y) {
        //Traverse segments until it breaks away from y
        //99.9% of the time this will happen the first go
        Path::const_iterator next = iter;
        next++;
        for(; ; next++) {
            if(next == path.end_closed()) next = path.begin();
            Rect bnds = next->boundsFast();
            //TODO: X considerations
            if(bnds.height() > 0) {
                //It has diverged
                if(bnds.contains(p)) {
                    const double fudge = 0.01;
                    if(cmp(y, next->valueAt(fudge, Y)) == initial_to_ray) {
                        wind += int(c);
                        std::cout << "!!!!!" << int(c) << " ";
                    }
                    iter = next; // No increment, as the rest of the thing hasn't been counted.
                } else {
                    Coord ny = next->initialPoint()[Y];
                    if(cmp(y, ny) == initial_to_ray) {
                        //Is a continuation through the ray, so counts windingwise
                        wind += int(c);
                        std::cout << "!!!!!" << int(c) << " ";
                    }
                    iter = ++next;
                }
                goto cont;
            }
            if(next==start) return wind;
        }
        //Looks like it looped, which means everything's flat
        return 0;
    }
    
    cont:(void)0;
  }
  return wind;
}

/* This function should only be applied to simple paths (regions), as otherwise
 * a boolean winding direction is undefined.  It returns true for fill, false for
 * hole.  Defaults to using the sign of area when it reaches funny cases.
 */
bool path_direction(Path const &p) {
    //could probably be more efficient, but this is a quick job
    double y = p.initialPoint()[Y];
    double x = p.initialPoint()[X];
    Cmp res = cmp(p[0].finalPoint()[Y], y);
    for(unsigned i = 1; i < p.size(); i++) {
        Cmp final_to_ray = cmp(p[i].finalPoint()[Y], y);
        Cmp initial_to_ray = cmp(p[i].initialPoint()[Y], y);
        // if y is included, these will have opposite values, giving order.
        Cmp c = cmp(final_to_ray, initial_to_ray);
        if(c != EQUAL_TO) {
            std::vector<double> rs = p[i].roots(y, Y);
            for(unsigned j = 0; j < rs.size(); j++) {
                double nx = p[i].valueAt(rs[j], X);
                if(nx > x) {
                    x = nx;
                    res = c;
                }
            }
        } else if(final_to_ray == EQUAL_TO) {
            goto doh;
        }
    }
    return res > 0;
    
    doh:
        //Otherwise fallback on area
        Piecewise<D2<SBasis> > pw = p.toPwSb();
        double area;
        Point centre;
        Geom::centroid(pw, centre, area);
        return area > 0;
}

//pair intersect code based on njh's pair-intersect

// A little sugar for appending a list to another
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

/* Finds the intersection between the lines defined by A0 & A1, and B0 & B1.
 * Returns through the last 3 parameters, returning the t-values on the lines
 * and the cross-product of the deltas (a useful byproduct).  The return value
 * indicates if the time values are within their proper range on the line segments.
 */
bool
linear_intersect(Point A0, Point A1, Point B0, Point B1,
                 double &tA, double &tB, double &det) {
    // kramers rule as cross products
    Point Ad = A1 - A0,
          Bd = B1 - B0,
           d = B0 - A0;
    det = cross(Ad, Bd);
    if( 1.0 + det == 1.0 )
        return false;
    else
    {
        double detinv = 1.0 / det;
        tA = cross(d, Bd) * detinv;
        tB = cross(d, Ad) * detinv;
        return tA >= 0. && tA <= 1. && tB >= 0. && tB <= 1.;
    }
}

/* This uses the local bounds functions of curves to generically intersect two.
 * It passes in the curves, time intervals, and keeps track of depth, while
 * returning the results through the Crossings parameter.
 */
void pair_intersect(Curve const & A, double Al, double Ah, 
                    Curve const & B, double Bl, double Bh,
                    Crossings &ret,  unsigned depth=0) {
   // std::cout << depth << "(" << Al << ", " << Ah << ")\n";
    Rect Ar = A.boundsLocal(Interval(Al, Ah));
    if(Ar.isEmpty()) return;

    Rect Br = B.boundsLocal(Interval(Bl, Bh));
    if(Br.isEmpty()) return;
    
    if(!Ar.intersects(Br)) return;
    
    //Checks the general linearity of the function
    if((depth > 12) || (A.boundsLocal(Interval(Al, Ah), 1).maxExtent() < 0.1 
                    &&  B.boundsLocal(Interval(Bl, Bh), 1).maxExtent() < 0.1)) {
        double tA, tB, c;
        if(linear_intersect(A.pointAt(Al), A.pointAt(Ah), 
                            B.pointAt(Bl), B.pointAt(Bh), 
                            tA, tB, c)) {
            tA = tA * (Ah - Al) + Al;
            tB = tB * (Bh - Bl) + Bl;
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    if(depth > 12) return;
    double mid = (Bl + Bh)/2;
    pair_intersect(B, Bl, mid,
                    A, Al, Ah,
                    ret, depth+1);
    pair_intersect(B, mid, Bh,
                    A, Al, Ah,
                    ret, depth+1);
}

// A simple wrapper around pair_intersect
Crossings SimpleCrosser::crossings(Curve const &a, Curve const &b) {
    Crossings ret;
    pair_intersect(a, 0, 1, b, 0, 1, ret);
    return ret;
}

/* Takes two paths and time ranges on them, with the invariant that the
 * paths are monotonic on the range.  Splits A when the linear intersection
 * doesn't exist or is inaccurate.  Uses the fact that it is monotonic to
 * do very fast bounds. 
 */
void mono_pair(Path const &A, double Al, double Ah,
               Path const &B, double Bl, double Bh,
               Crossings &ret, double tol, unsigned depth = 0) {
    if( Al >= Ah || Bl >= Bh) return;
    std::cout << " " << depth << "[" << Al << ", " << Ah << "]" << "[" << Bl << ", " << Bh << "]";

    //Split B
    Point A0 = A.pointAt(Al), A1 = A.pointAt(Ah),
          B0 = B.pointAt(Bl), B1 = B.pointAt(Bh);
    //inline code that this implies? (without rect/interval construction)
    if(!Rect(A0, A1).intersects(Rect(B0, B1)) || A0 == A1 || B0 == B1) return;
    
    double ltA, ltB, c;
    double tA, tB;
    if(linear_intersect(A0, A1, B0, B1, ltA, ltB, c)) {
        tA = ltA * (Ah - Al) + Al;
        tB = ltB * (Bh - Bl) + Bl;
        double dist = LInfty(A.pointAt(tA) - B.pointAt(tB));
        //std::cout << dist;
        if(depth >= 12 || dist <= tol) {
            if(depth % 2)
                ret.push_back(Crossing(tB, tA, c < 0));
            else
                ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
    }
    tB = (Bl + Bh) / 2;
    if(depth < 12) {
        mono_pair(B, Bl, tB,
                  A, Al, Ah,
                  ret, tol, depth+1);
        mono_pair(B, tB, Bh,
                  A, Al, Ah,
                  ret, tol, depth+1);
    }
}

// This returns the times when the x or y derivative is 0 in the curve.
std::vector<double> curve_mono_splits(Curve const &d) {
    std::vector<double> rs = d.roots(0, X);
    append(rs, d.roots(0, Y));
    std::sort(rs.begin(), rs.end());
    return rs;
}

// Convenience function to add a value to each entry in a vector of doubles.
std::vector<double> offset_doubles(std::vector<double> const &x, double offs) {
    std::vector<double> ret;
    for(unsigned i = 0; i < x.size(); i++) {
        ret.push_back(x[i] + offs);
    }
    return ret;
}

/* Finds all the monotonic splits for a path.  Only includes the split between
 * curves if they switch derivative directions at that point.
 */
std::vector<double> path_mono_splits(Path const &p) {
    std::vector<double> ret;
    if(p.empty()) return ret;
    ret.push_back(0);
    
    Curve* deriv = p[0].derivative();
    append(ret, curve_mono_splits(*deriv));
    delete deriv;
    
    bool pdx, pdy;  //Previous derivative direction
    for(unsigned i = 0; i <= p.size(); i++) {
        deriv = p[i].derivative();
        std::vector<double> spl = offset_doubles(curve_mono_splits(*deriv), i);
        delete deriv;
        bool dx = p[i].initialPoint()[0] > (spl.empty()? p[i].finalPoint()[X] :
                                                         p.valueAt(spl.front(), X));
        bool dy = p[i].initialPoint()[1] > (spl.empty()? p[i].finalPoint()[Y] :
                                                         p.valueAt(spl.front(), Y));
        //The direction changed, include the split time
        if(dx != pdx || dy != pdy) {
            ret.push_back(i);
            pdx = dx; pdy = dy;
        }
        append(ret, spl);
    }
    return ret;
}

/* Applies path_mono_splits to multiple paths, and returns the results such that 
 * time-set i corresponds to Path i.
 */
std::vector<std::vector<double> > paths_mono_splits(std::vector<Path> const &ps) {
    std::vector<std::vector<double> > ret;
    for(unsigned i = 0; i < ps.size(); i++)
        ret.push_back(path_mono_splits(ps[i]));
    return ret;
}

std::vector<std::vector<Rect> > split_bounds(std::vector<Path> const &p, std::vector<std::vector<double> > splits) {
    std::vector<std::vector<Rect> > ret;
    for(unsigned i = 0; i < p.size(); i++) {
        std::vector<Rect> res;
        for(unsigned j = 1; j < splits[i].size(); j++)
            res.push_back(Rect(p[i].pointAt(splits[i][j-1]), p[i].pointAt(splits[i][j])));
        ret.push_back(res);
    }
    return ret;
}

CrossingSet MonoCrosser::crossings(std::vector<Path> const &a, std::vector<Path> const &b) {
    if(b.empty()) return CrossingSet(a.size(), Crossings());
    CrossingSet results(a.size() + b.size(), Crossings());
    if(a.empty()) return results;

    std::vector<std::vector<double> > splits_a = paths_mono_splits(a), splits_b = paths_mono_splits(b);
    std::vector<std::vector<Rect> > bounds_a = split_bounds(a, splits_a), bounds_b = split_bounds(b, splits_b);
    
    std::vector<Rect> bounds_a_union, bounds_b_union; 
    for(unsigned i = 0; i < bounds_a.size(); i++) bounds_b_union.push_back(union_list(bounds_a[i]));
    for(unsigned i = 0; i < bounds_b.size(); i++) bounds_b_union.push_back(union_list(bounds_b[i]));

    std::vector<std::vector<unsigned> > cull = sweep_bounds(bounds_b_union, bounds_b_union);
    Crossings n;
    for(unsigned i = 0; i < cull.size(); i++) {
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            unsigned jc = j + a.size();
            Crossings res;
            std::vector<std::vector<unsigned> > cull2 = sweep_bounds(bounds_a[i], bounds_b[j]);
            for(unsigned k = 0; k < cull2.size(); k++) {
                for(unsigned lx = 0; lx < cull2[k].size(); lx++) {
                    unsigned l = cull2[k][lx];
                    mono_pair(a[i], splits_a[i][k-1], splits_a[i][k],
                              b[j], splits_b[j][l-1], splits_b[j][l],
                              res, .1);
                }
            }
            
            for(unsigned k = 0; k < res.size(); k++) { res[k].a = i; res[k].b = jc; }
            
            merge_crossings(results[i], res, i);
            merge_crossings(results[i], res, jc);
        }
    }

    return results;
}

CrossingSet crossings_among(std::vector<Path> const &p) {
    CrossingSet results(p.size(), Crossings());
    if(p.empty()) return results;
    
    std::vector<std::vector<double> > splits = paths_mono_splits(p);
    std::vector<std::vector<Rect> > prs = split_bounds(p, splits);
    std::vector<Rect> rs;
    for(unsigned i = 0; i < prs.size(); i++) rs.push_back(union_list(prs[i]));
    
    std::vector<std::vector<unsigned> > cull = sweep_bounds(rs);

    for(unsigned i = 0; i < cull.size(); i++) {
        for(unsigned jx = 0; jx < cull[i].size(); jx++) {
            unsigned j = cull[i][jx];
            Crossings res;
            std::vector<std::vector<unsigned> > cull2 = sweep_bounds(prs[i], prs[j]);
            for(unsigned k = 0; k < cull2.size(); k++) {
                for(unsigned lx = 0; lx < cull2[k].size(); lx++) {
                    unsigned l = cull2[k][lx];
                    mono_pair(p[i], splits[i][k-1], splits[i][k],
                              p[j], splits[j][l-1], splits[j][l],
                              res, .1);
                }
            }
            
            for(unsigned k = 0; k < res.size(); k++) { res[k].a = i; res[k].b = j; }
            
            merge_crossings(results[i], res, i);
            merge_crossings(results[j], res, j);
        }
    }
    
    return results;
}

}
