#include <cairo.h>
#include "path-cairo.h"

void cairo_sub_path(cairo_t *cr, Geom::SubPath const &p) {
    if(p.empty()) return;
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
    if(p.is_closed()) {
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

/*** This is really just for debugging porpoises. */
#include <sstream>
#include <iostream>
#include "interactive-bits.h"
#include <pango/pango.h>
#include <pango/pangocairo.h>
void cairo_sub_path_handles(cairo_t *cr, Geom::SubPath const &p) {
    if(p.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Geom::SubPath::Elem elem = *iter;
        for(int i = 0; i < elem.size(); i++) {
            std::ostringstream notify;
            notify << i;
            draw_cross(cr, elem[i]);
            cairo_move_to(cr, elem[i]);
            PangoLayout* layout = pango_cairo_create_layout (cr);
            pango_layout_set_text(layout, 
                                  notify.str().c_str(), -1);

            PangoFontDescription *font_desc = pango_font_description_new();
            pango_font_description_set_family(font_desc, "Sans");
            const int size_px = 10;
            pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
            pango_layout_set_font_description(layout, font_desc);
            PangoRectangle logical_extent;
            pango_layout_get_pixel_extents(layout,
                                           NULL,
                                           &logical_extent);
            pango_cairo_show_layout(cr, layout);
        }
    }
}

void cairo_path_handles(cairo_t *cr, Geom::Path const &p) {
    std::vector<Geom::SubPath> subpaths;
    
    for (std::vector<Geom::SubPath>::const_iterator it(p.begin()),
             iEnd(p.end());
         it != iEnd; ++it) {
        cairo_sub_path_handles(cr, *it);
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
