#include <cairo/cairo.h>
#include "path-cairo.h"

void cairo_sub_path(cairo_t *cr, Geom::SubPath const &p) {
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        Geom::SubPath::SubPathElem elm = *iter;
        switch(iter.cmd()) {
            case Geom::moveto:
                cairo_move_to(cr, elm.first()[0], elm.first()[1]);
                break;
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
        cairo_line_to(cr, p.handles[1][0], p.handles[1][1]);
        //cairo_fill(cr);
    } else
        ;//cairo_stroke(cr);
        
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
