#include <cairo.h>
#include "path-cairo.h"
#include "path-builder.h"
#include "multidim-sbasis.h"
#include "sbasis-to-bezier.h"

using namespace Geom;

void cairo_curve(cairo_t *cr, Path2::Curve const& c) {
    if(Path2::LineSegment const* line_segment = dynamic_cast<Path2::LineSegment const*>(&c)) {
        cairo_line_to(cr, (*line_segment)[1][0], (*line_segment)[1][1]);
    }
    else if(Path2::QuadraticBezier const *quadratic_bezier = dynamic_cast<Path2::QuadraticBezier const*>(&c)) {
        Point b1 = (*quadratic_bezier)[0] + (2./3) * ((*quadratic_bezier)[1] - (*quadratic_bezier)[0]);
        Point b2 = b1 + (1./3) * ((*quadratic_bezier)[2] - (*quadratic_bezier)[0]);
        cairo_curve_to(cr, b1[0], b1[1], 
                       b2[0], b2[1], 
                       (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1]);
    }
    else if(Path2::CubicBezier const *cubic_bezier = dynamic_cast<Path2::CubicBezier const*>(&c)) {
        cairo_curve_to(cr, (*cubic_bezier)[1][0], (*cubic_bezier)[1][1], (*cubic_bezier)[2][0], (*cubic_bezier)[2][1], (*cubic_bezier)[3][0], (*cubic_bezier)[3][1]);
    }
//    else if(Path2::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Path2::SVGEllipticalArc *>(c)) {
//        //get at the innards and spit them out to cairo
//    }
    else {
        //this case handles sbasis as well as all other curve types
        Path2::Path sbasis_path;
        path_from_sbasis(sbasis_path, c.sbasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Path2::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            cairo_curve(cr, *iter);
        }
    }
}

void cairo_path(cairo_t *cr, Path2::Path const &p) {
    cairo_move_to(cr, p.initialPoint()[0], p.initialPoint()[1]);

    for(Path2::Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        cairo_curve(cr, *iter);
    }
    if(p.closed())
        cairo_close_path(cr);
}

void cairo_path(cairo_t *cr, std::vector<Path2::Path> const &p) {
    std::vector<Path2::Path>::const_iterator it;
    for(it = p.begin(); it != p.end(); it++) {
        cairo_path(cr, *it);
    }
}

void cairo_path(cairo_t *cr, Path const &p) {
    if(p.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Path::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Path::Elem elm = *iter;
        if(dynamic_cast<LineTo *>(iter.cmd())) {
            cairo_line_to(cr, elm.last()[0], elm.last()[1]);
        }  else if(dynamic_cast<QuadTo *>(iter.cmd())) {
            Point b1 = elm[0] + (2./3) * (elm[1] - elm[0]);
            Point b2 = b1 + (1./3) * (elm[2] - elm[0]);
            cairo_curve_to(cr, b1[0], b1[1], 
                           b2[0], b2[1], 
                           elm[2][0], elm[2][1]);
        }  else if(dynamic_cast<CubicTo *>(iter.cmd())) {
            cairo_curve_to(cr, elm[1][0], elm[1][1], 
                           elm[2][0], elm[2][1], 
                           elm[3][0], elm[3][1]);
        }
    }
    if(p.is_closed()) {
        cairo_close_path(cr);
    }
}

void cairo_PathSet(cairo_t *cr, PathSet const &p) {
    std::vector<Path> subpaths;
    
    for (std::vector<Path>::const_iterator it(p.begin()),
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
void cairo_path_handles(cairo_t *cr, Path const &p) {
    if(p.empty()) return;
    cairo_move_to(cr, p.initial_point()[0], p.initial_point()[1]);
    for(Path::const_iterator iter(p.begin()), end(p.end()); iter < end; ++iter) {
        Path::Elem elem = *iter;
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

void cairo_PathSet_handles(cairo_t *cr, PathSet const &p) {
    std::vector<Path> subpaths;
    
    for (std::vector<Path>::const_iterator it(p.begin()),
             iEnd(p.end());
         it != iEnd; ++it) {
        cairo_path_handles(cr, *it);
    }
}

void cairo_md_sb(cairo_t *cr, MultidimSBasis<2> const &B) {
    PathSetBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_PathSet(cr, pb.peek());
}

void cairo_md_sb_handles(cairo_t *cr, MultidimSBasis<2> const &B) {
    PathSetBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_PathSet_handles(cr, pb.peek());
}

//TODO: what's the diff between the next two funcs?

void cairo_sb2d(cairo_t* cr, std::vector<SBasis2d> const &sb2, Point dir, double width) {
    MultidimSBasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);// + BezOrd(u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);// + BezOrd(v);
        B[0] = extract_v(sb2[0], v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
}

void draw_sb2d(cairo_t* cr, SBasis2d const &sb2, Point dir, double width) {
    MultidimSBasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = dir[0]*extract_u(sb2, u) + BezOrd(u);
        B[1] = SBasis(BezOrd(0,1))+dir[1]*extract_u(sb2, u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = dir[1]*extract_v(sb2, v) + BezOrd(v);
        B[0] = SBasis(BezOrd(0,1)) + dir[0]*extract_v(sb2, v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
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
