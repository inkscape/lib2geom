#include <cairo.h>
#include "path-cairo.h"
#include "path-builder.h"
#include "multidim-sbasis.h"
#include "sbasis-to-bezier.h"

void cairo_path(cairo_t *cr, Geom::Path const &p) {
    if(p.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Geom::Path::Elem elm = *iter;
        if(dynamic_cast<Geom::LineTo *>(iter.cmd())) {
            cairo_line_to(cr, elm.last()[0], elm.last()[1]);
        }  else if(dynamic_cast<Geom::QuadTo *>(iter.cmd())) {
            Geom::Point b1 = elm[0] + (2./3) * (elm[1] - elm[0]);
            Geom::Point b2 = b1 + (1./3) * (elm[2] - elm[0]);
            cairo_curve_to(cr, b1[0], b1[1], 
                           b2[0], b2[1], 
                           elm[2][0], elm[2][1]);
        }  else if(dynamic_cast<Geom::CubicTo *>(iter.cmd())) {
            cairo_curve_to(cr, elm[1][0], elm[1][1], 
                           elm[2][0], elm[2][1], 
                           elm[3][0], elm[3][1]);
        }
    }
    if(p.is_closed()) {
        cairo_close_path(cr);
    }
}

void cairo_arrangement(cairo_t *cr, Geom::Arrangement const &p) {
    std::vector<Geom::Path> subpaths;
    
    for (std::vector<Geom::Path>::const_iterator it(p.begin()),
             iEnd(p.end());
         it != iEnd; ++it) {
        cairo_path(cr, *it);
    }
}

/*** This is really just for debugging porpoises. */
#include <sstream>
#include <iostream>
#include "interactive-bits.h"
#include <pango/pango.h>
#include <pango/pangocairo.h>
void cairo_path_handles(cairo_t *cr, Geom::Path const &p) {
    if(p.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Geom::Path::Elem elem = *iter;
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

void cairo_arrangement_handles(cairo_t *cr, Geom::Arrangement const &p) {
    std::vector<Geom::Path> subpaths;
    
    for (std::vector<Geom::Path>::const_iterator it(p.begin()),
             iEnd(p.end());
         it != iEnd; ++it) {
        cairo_path_handles(cr, *it);
    }
}

void cairo_md_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::ArrangementBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_arrangement(cr, pb.peek());
}

void cairo_md_sb_handles(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::ArrangementBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_arrangement_handles(cr, pb.peek());
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
