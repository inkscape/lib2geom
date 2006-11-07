#include "path.h"
#include <ctype.h>
#include <vector>
#include <cassert>

#include "point-fns.h"
#include "path-builder.h"

class path_to_polyline{
public:
    Geom::PathSetBuilder pb;
    double tol;
    
    path_to_polyline(const Geom::Path &p, double tol = 1.0);
    
    void line_to_polyline(Geom::Path::Elem e);
    
    void quad_to_polyline(Geom::Path::Elem e);
    void cubic_to_polyline(Geom::Path::Elem e);
    
    operator Geom::PathSet();
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

    
