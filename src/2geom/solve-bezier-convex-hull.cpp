
#include <2geom/solver.h>
#include <2geom/choose.h>
#include <2geom/bezier.h>
#include <2geom/point.h>

#include <cmath>
#include <algorithm>

/*** Find the zeros of the bernstein function.  This method portions the control polygon taking the left root of the convex hull until coefficient 0 is 0.  This root is deflated out 
 */

namespace Geom{

template<class t>
static int SGN(t x) { return (x > 0 ? 1 : (x < 0 ? -1 : 0)); }

// Find left most convex hull crossing.  algorithm: sweeping from 0 to
// 1 each edge is either in the upper or lower edge set.  We only need
// to consider the line segment from the previous lower and upper
// point and the points are already in order.
namespace detail { namespace bezier_clipping {
void convex_hull (std::vector<Point> & P);
} } // defined in bezier-clipping.cpp

void find_left_convex_hull(Geom::Bezier const& bz) {
  const double dt = 1./bz.size();
  std::vector<Point> P;
  for(unsigned i = 0; i < bz.size(); i++) {
    P.push_back(Point(i*dt, bz[i]));
  }
  
  detail::bezier_clipping::convex_hull(P);
  
  Point p0 = P.back();
  double min_x = 1;
  for(unsigned i = 0; i < P.size(); i++) {
    Point p1 = P[i];
    double x = (-p0[1]) / (p1[1] - p0[1]);
    if(x >= 0 and x <= 1 and x < min_x) {
      min_x = x;
    }
    
  }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
