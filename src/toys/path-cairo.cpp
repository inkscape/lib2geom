#include <cairo.h>
#include "path-cairo.h"
#include "sbasis-to-bezier.h"
#include "utils.h"

using namespace Geom;

void cairo_curve(cairo_t *cr, Curve const& c) {
    if(LineSegment const* line_segment = dynamic_cast<LineSegment const*>(&c)) {
        cairo_line_to(cr, (*line_segment)[1][0], (*line_segment)[1][1]);
    }
    else if(QuadraticBezier const *quadratic_bezier = dynamic_cast<QuadraticBezier const*>(&c)) {
        std::vector<Point> points = quadratic_bezier->points();
        Point b1 = points[0] + (2./3) * (points[1] - points[0]);
        Point b2 = b1 + (1./3) * (points[2] - points[0]);
        cairo_curve_to(cr, b1[0], b1[1], 
                       b2[0], b2[1], 
                       points[2][0], points[2][1]);
    }
    else if(CubicBezier const *cubic_bezier = dynamic_cast<CubicBezier const*>(&c)) {
        std::vector<Point> points = cubic_bezier->points();
        cairo_curve_to(cr, points[1][0], points[1][1], points[2][0], points[2][1], points[3][0], points[3][1]);
    }
//    else if(SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<SVGEllipticalArc *>(c)) {
//        //TODO: get at the innards and spit them out to cairo
//    }
    else {
        //this case handles sbasis as well as all other curve types
        Path sbasis_path;
        path_from_sbasis(sbasis_path, c.toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            cairo_curve(cr, *iter);
        }
    }
}

void cairo_path(cairo_t *cr, Path const &p) {
    cairo_move_to(cr, p.initialPoint()[0], p.initialPoint()[1]);

    for(Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        cairo_curve(cr, *iter);
    }
    if(p.closed())
        cairo_close_path(cr);
}

void cairo_path_handles(cairo_t *cr, Path const &p) {
    //TODO
}

void cairo_path(cairo_t *cr, std::vector<Path> const &p) {
    std::vector<Path>::const_iterator it;
    for(it = p.begin(); it != p.end(); it++) {
        cairo_path(cr, *it);
    }
}

#if 0
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
#endif

void cairo_md_sb(cairo_t *cr, D2<SBasis> const &B) {
    Path pb;
    path_from_sbasis(pb, B, 0.1);
    cairo_path(cr, pb);
}

void cairo_2dsb2d(cairo_t* cr, D2<SBasis2d> const &sb2, Point dir, double width) {
    D2<SBasis> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);// + Linear(u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);// + Linear(v);
        B[0] = extract_v(sb2[0], v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
}

void cairo_sb2d(cairo_t* cr, SBasis2d const &sb2, Point dir, double width) {
    D2<SBasis> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2, u)*dir[0] + Linear(u);
        B[1] = SBasis(Linear(0,1)) + extract_u(sb2, u)*dir[1];
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2, v)*dir[1] + Linear(v);
        B[0] = SBasis(Linear(0,1)) + extract_v(sb2, v)*dir[0];
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
}

void cairo_d2_pw(cairo_t *cr, D2<Piecewise<SBasis> > const &p) {
    cairo_pw_d2(cr, sectionize(p));
}

void cairo_pw_d2(cairo_t *cr, Piecewise<D2<SBasis> > const &p) {
    for(unsigned i = 0; i < p.size(); i++)
        cairo_md_sb(cr, p[i]);
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
