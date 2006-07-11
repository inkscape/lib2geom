#include <cairo/cairo.h>
#include "path-cairo.h"

void cairo_sub_path(cairo_t *cr, Geom::SubPath const &p) {
    if(p.cmd.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Geom::SubPath::Elem elm = *iter;
        switch(iter.cmd()) {
            case Geom::lineto:
                cairo_line_to(cr, elm.last()[0], elm.last()[1]);
                break;
            case Geom::quadto:
            {
                Geom::Point b1 = elm[0] + (2./3) * (elm[1] - elm[0]);
                Geom::Point b2 = b1 + (1./3) * (elm[2] - elm[0]);
                cairo_curve_to(cr, b1[0], b1[1], 
                               b2[0], b2[1], 
                               elm[2][0], elm[2][1]);
                break;
            }
            case Geom::cubicto:
                cairo_curve_to(cr, elm[1][0], elm[1][1], 
                               elm[2][0], elm[2][1], 
                               elm[3][0], elm[3][1]);
                break;
            default:
                break;
        }
    }
    if(p.closed) {
        cairo_close_path(cr);
    }
}

void cairo_path(cairo_t *cr, Geom::Path const &p) {
    std::vector<Geom::SubPath> subpaths;
    
    for (std::vector<Geom::SubPath>::const_iterator it(p.begin()),
             iEnd(p.end());
         it != iEnd; ++it) {
        cairo_sub_path(cr, *it);
    }
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
