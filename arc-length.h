#include "path.h"

/*** Routines in this group return a path that looks the same, but
 * include extra knots for certain points of interest. */

/*** find_vector_extreme_points
 * extreme points . dir.
 */

double arc_length_subdividing(Geom::SubPath const & p, double tol);
double arc_length_integrating(Geom::SubPath const & p, double tol);
double arc_length_integrating(Geom::SubPath const & p, Geom::SubPath::SubPathLocation const & pl, double tol);

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
