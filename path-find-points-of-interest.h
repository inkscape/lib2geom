#include "path.h"
#include <vector>

/*** Routines in this group return a path that looks the same, but
 * include extra knots for certain points of interest. */

/*** find_vector_extreme_points
 * extreme points . dir.
 */

std::vector<Geom::Path::PathLocation>
find_vector_extreme_points(Geom::Path p, Geom::Point dir);

std::vector<Geom::Path::PathLocation>
find_inflection_points(Geom::Path p);
std::vector<Geom::Path::PathLocation>
find_flat_points(Geom::Path p);

Geom::Path::PathLocation dim_extreme_points(Geom::Path::PathElem e);

template <class F>
std::vector<Geom::Path::PathLocation> find_points(Geom::Path p, F f) {
    std::vector<Geom::Path::PathLocation> result;

    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); 
        iter != end; ++iter) {
        std::vector<Geom::Path::PathLocation> v = f(*iter);
        
        result.insert(result.end(), v.begin(), v.end());
    }
    return result;
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
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
