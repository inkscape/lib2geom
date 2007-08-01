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
  Path::const_iterator iter = start;
  bool temp = true;
  for (; iter != start || temp
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
            std::cout << int(c) << "\n";
            goto cont;
        }
    } else {
        //inside bbox, use custom per-curve winding thingie
        int delt = iter->winding(p);
        wind += delt;
        std::cout << "n " << delt << "\n";
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
                        std::cout << "!!!!!" << " " << int(c) << "\n";
                    }
                    iter = next; // No increment, as the rest of the thing hasn't been counted.
                } else {
                    Coord ny = next->initialPoint()[Y];
                    if(cmp(y, ny) == initial_to_ray) {
                        //Is a continuation through the ray, so counts windingwise
                        wind += int(c);
                        std::cout << "!!!!!" << " " << int(c) << "\n";
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

Crossings to_crossings(std::vector<std::pair<double, double> > ts, Curve const &a, Curve const &b) { 
    Crossings ret;
    for(unsigned i = 0; i < ts.size(); i++) {
        /* If the absolute value of the winding is less on the increased t,
           then the dir is outside (true) */
        double at = ts[i].first;
        //TODO: better way to decide direction
        ret.push_back(Crossing(at, ts[i].second, std::abs(b.winding(a.pointAt(at + .01))) <
                                                 std::abs(b.winding(a.pointAt(at - .01)))));
    }
}

Crossings Curve::crossingsWith(Curve const &c) const {
    std::vector<Point> a_points, b_points;
    
    if(LineSegment const* bez = dynamic_cast<LineSegment const*>(&c))
        a_points = bez->points();
    else if(QuadraticBezier const* bez = dynamic_cast<QuadraticBezier const*>(&c))
        a_points = bez->points();
    else if(CubicBezier const* bez = dynamic_cast<CubicBezier const*>(&c))
        a_points = bez->points();
    else
        a_points = sbasis_to_bezier(toSBasis());
        
    if(LineSegment const* bez = dynamic_cast<LineSegment const*>(&c))
        b_points = bez->points();
    else if(QuadraticBezier const* bez = dynamic_cast<QuadraticBezier const*>(&c))
        b_points = bez->points();
    else if(CubicBezier const* bez = dynamic_cast<CubicBezier const*>(&c))
        b_points = bez->points();
    else
        b_points = sbasis_to_bezier(c.toSBasis());
    
    return to_crossings(find_intersections(a_points, b_points), *this, c);
}

std::vector<Rect> curve_bounds(Path const &x) {
    std::vector<Rect> ret;
    for(Path::const_iterator it = x.begin(); it != x.end_closed(); it++)
        ret.push_back(it->boundsFast());
}

Crossings crossings(Path const &a, Path const &b) {
    Crossings crossings;

    std::vector<Rect> bounds_a = curve_bounds(a), bounds_b = curve_bounds(b);
    for(unsigned i = 0; i < bounds_a.size(); i++) {
        for(unsigned j = 0; j < bounds_b.size(); j++) {
            if(bounds_a[i].intersects(bounds_b[j])) {
                Crossings curve_crossings = a[i].crossingsWith(b[j]);
                crossings.insert(crossings.end(), curve_crossings.begin(), curve_crossings.end());
            }
        }
    }
}

}
