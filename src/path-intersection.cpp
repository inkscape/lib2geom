#include "path-intersection.h"

#include "basic-intersection.h"
#include "bezier-to-sbasis.h"
#include "ord.h"

namespace Geom {

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
  bool temp = true;
  for (Path::const_iterator iter = start; iter != start || temp
       ; ++iter, iter = (iter == path.end_closed()) ? path.begin() : iter )
  {
    temp = false;
    Rect bounds = iter->boundsFast();
    Coord x = p[X], y = p[Y];
    if(x > bounds.right() || !bounds[Y].contains(y)) continue;
    Point final = iter->finalPoint();
    Point initial = iter->initialPoint();
    Cmp final_to_ray = cmp(final[Y], y);
    Cmp initial_to_ray = cmp(initial[Y], y);
    // if y is included, these will have opposite values, giving order.
    Cmp c = cmp(final_to_ray, initial_to_ray); 
    if(x < bounds.left()) {    //ray goes through bbox
        // winding determined by position of endpoints
        if(final_to_ray != EQUAL_TO) {
            wind += int(c); // GT = counter-clockwise = 1; LT = clockwise = -1; EQ = not-included = 0
            std::cout << int(c) << " ";
            goto cont;
        }
    } else {
        //inside bbox, use custom per-curve winding thingie
        int delt = iter->winding(p);
        wind += delt;
        std::cout << "n" << delt << " ";
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

/*
int path_direction(Path p) {
    int wind = 0, max = 0;
    for (Path::const_iterator iter = p.begin(); iter != p.end(); ++iter) {
        iter->initialPoint()
        
    }
}*/

/*
//Finds intervals larger than a particular width, which contain a crossing
std::vector<std::pair<Interval, Interval> > break_down(Curve const & a, Curve const & b, double width) {
    std::vector<std::pair<Interval, Interval> > res;
    int prev = b.winding(a.initialPoint());
    for(double t = width; t <= 1; t += width) {
        int val = b.winding(a.valueAt(t)
        if(val != prev) res.push_back(std::pair<Interval, Interval>(Interval(t-width,t),
                                     Interval();
    }
}
*/
/*
Crossings crossings_recurse(Curve const &a, Curve const &b,
                            std::vector<Interval> a_i, std::vector<Interval> b_i) {
    Crossings ret;
    a_bounds = a.boundsFast(); 
    b_bounds = b.boundsFast();
    if(a_bounds.intersects(b_bounds)) {
        
    }
    return ret;
}
*/

//pair intersect code copied from njh's pair-intersect

/** Given two linear md_sb(assume they are linear even if they're not)
    find the ts at the intersection. */
bool
linear_pair_intersect(D2<SBasis> A, double Al, double Ah, 
                      D2<SBasis> B, double Bl, double Bh,
                      double &tA, double &tB) {
    Rect Ar = bounds_local(A, Interval(Al, Ah));
    // kramers rule here
    Point A0 = A(Al);
    Point A1 = A(Ah);
    Point B0 = B(Bl);
    Point B1 = B(Bh);
    double xlk = A1[X] - A0[X];
    double ylk = A1[Y] - A0[Y];
    double xnm = B1[X] - B0[X];
    double ynm = B1[Y] - B0[Y];
    double xmk = B0[X] - A0[X];
    double ymk = B0[Y] - A0[Y];
    double det = xnm * ylk - ynm * xlk;
    if( 1.0 + det == 1.0 )
        return false;
    else
    {
        double detinv = 1.0 / det;
        double s = ( xnm * ymk - ynm *xmk ) * detinv;
        double t = ( xlk * ymk - ylk * xmk ) * detinv;
        if( ( s < 0.0 ) || ( s > 1.0 ) || ( t < 0.0 ) || ( t > 1.0 ) )
            return false;
        tA = Al + s * ( Ah - Al );
        tB = Bl + t * ( Bh - Bl );
        return true;
    }
}

void pair_intersect(std::vector<double> &Asects,
                    std::vector<double> &Bsects,
                    D2<SBasis> A, double Al, double Ah, 
                    D2<SBasis> B, double Bl, double Bh, unsigned depth=0) {

    // we'll split only A, and swap args
    Rect Ar = bounds_local(A, Interval(Al, Ah));
    if(Ar.isEmpty()) return;

    Rect Br = bounds_local(B, Interval(Bl, Bh));
    if(Br.isEmpty()) return;
    
    if((depth > 12) || Ar.intersects(Br)) {
        double Ate = 0;
        double Bte = 0;
        for(unsigned d = 0; d < 2; d++) {
            Interval bs = bounds_local(A[d], Interval(Al, Ah), 1); //only 1?
            Ate = std::max(Ate, bs.extent());
        }
        for(unsigned d = 0; d < 2; d++) {
            Interval bs = bounds_local(B[d], Interval(Bl, Bh), 1);
            Bte = std::max(Bte, bs.extent());
        }

        if((depth > 12)  || ((Ate < 0.1) && 
           (Bte < 0.1))) {
            double tA, tB;
            if(linear_pair_intersect(A, Al, Ah, 
                                     B, Bl, Bh, 
                                     tA, tB)) {
                std::cout << "intersects " << tA << ", " << tB << "\n";
                Asects.push_back(tA);
                Bsects.push_back(tB);
            }
            
        } else {
            double mid = (Al + Ah)/2;
            pair_intersect(Bsects, Asects,
                           B, Bl, Bh,
                           A, Al, mid, depth+1);
            pair_intersect(Bsects, Asects,
                           B, Bl, Bh,
                           A, mid, Ah, depth+1);
        }
    }
}

Crossings to_crossings(std::vector<std::pair<double, double> > ts, Curve const &a, Curve const &b) { 
    Crossings ret;
    for(unsigned i = 0; i < ts.size(); i++) {
        /* If the absolute value of the winding is less on the increased t,
           then the dir is outside (true) */
        double at = ts[i].first, bt = ts[i].second;
        ret.push_back(Crossing(at, bt, cross(a.pointAt(at) - a.pointAt(at + .05),
                                             b.pointAt(bt) - b.pointAt(bt + .05)) > 0));
    }
    return ret;
}

template<typename A, typename B>
std::vector<std::pair<A, B> > zip(std::vector<A> const &a, std::vector<B> &b) {
    std::vector<std::pair<A, B> > ret;
    for(unsigned i = 0; i < a.size() && i < b.size(); i++)
        ret.push_back(std::pair<A, B>(a[i], b[i]));
    return ret;
}

Crossings Curve::crossingsWith(Curve const &c) const {
    std::vector<double> asects, bsects;
    pair_intersect(asects, bsects, toSBasis(), 0, 1, c.toSBasis(), 0, 1);
    return to_crossings(zip(asects, bsects), *this, c);
}

std::vector<Rect> curve_bounds(Path const &x) {
    std::vector<Rect> ret;
    for(Path::const_iterator it = x.begin(); it != x.end_closed(); ++it)
        ret.push_back(it->boundsFast());
    return ret;
}

Crossings crossings(Path const &a, Path const &b) {
    Crossings ret;

    std::vector<Rect> bounds_a = curve_bounds(a), bounds_b = curve_bounds(b);
    for(unsigned i = 0; i < bounds_a.size(); i++) {
        for(unsigned j = 0; j < bounds_b.size(); j++) {
            if(bounds_a[i].intersects(bounds_b[j])) {
                Crossings cc = a[i].crossingsWith(b[j]);
                for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
                    ret.push_back(Crossing(it->ta + i, it->tb + j, it->dir));
                }
            }
        }
    }
    return ret;
}

}
