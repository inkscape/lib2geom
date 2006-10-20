#include "path-nearest.h"
#include <queue>
#include "s-basis.h"
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"
#include "path-sbasis.h"
#include "convex-cover.h"
#include "epsilon.h"

using namespace Geom;

struct element_point_distance{
    double d; // eucl distance squared
    Path::const_iterator e;
    ConvexHull ch;
    element_point_distance(Path::const_iterator e,
			   Point p) : e(e), ch((*e).begin(), (*e).end()) {
        if(ch.contains_point(p)) {
            d = 0;
        } else {
            Path::Elem::const_iterator it(e.begin());
            {
                Point delta = *it-p;
                d = dot(delta, delta);
            }
            for(; it != e.end(); it++) {
                Point delta = *it-p;
                double dd = dot(delta, delta);
                if(dd < d)
                    d = dd;
            }
        }
    }
    bool operator<(const element_point_distance & b) const {
        return (d < b.d);
    }
};

/*** find_nearest_location
 * this code assumes that the path is contained within the convex hull of the handles.
 */
Geom::Path::Location find_nearest_location(Geom::Path const & p, Geom::Point pt) {
    std::priority_queue<element_point_distance> pq;
    
    for(Geom::Path::const_iterator it(p.begin()), end(p.end()); it != end; ++it) {
        pq.push(element_point_distance(it, pt));
    }
    double best_guess = INFINITY;
    Path::Location best(p.begin(), 0);
    while(!pq.empty()) {
        element_point_distance epd = pq.top();
        
        if(epd.d > best_guess)
            break; // early terminate
        Path::Elem elm = *epd.e;
        multidim_sbasis<2> B = elem_to_sbasis(*epd.e) - pt;
        SBasis Bdist = dot(B, B);
        for(int i = 0; i <=10; i++) {
            double t = i/10.;
            if(Bdist(t) < best_guess) {
                best.it = epd.e;
                best.t = t;
            }
        }
/*
        std::vector<double> r = roots(derivative(Bdist));
        for(unsigned i = 0; i < r.size(); i++) {
            if(Bdist(r[i]) < best_guess) {
                best.it = epd.e;
                best.t = r[i];
            }
            }*/
	pq.pop();
    }
    
    return best;
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
