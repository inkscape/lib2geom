#include <boost/python.hpp>
#include <cairo.h>
#include <toys/path-cairo.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/utils.h>
#include <sstream>
#include <pycairo/pycairo.h>
#include "cairo-helpers.h"

using namespace Geom;


void
cairo_move_to (cairo_t *cr, Geom::Point p1) {
    cairo_move_to(cr, p1[0], p1[1]);
}

void
cairo_line_to (cairo_t *cr, Geom::Point p1) {
    cairo_line_to(cr, p1[0], p1[1]);
}

void
cairo_curve_to (cairo_t *cr, Geom::Point p1, 
        Geom::Point p2, Geom::Point p3) {
    cairo_curve_to(cr, p1[0], p1[1],
               p2[0], p2[1],
               p3[0], p3[1]);
}

void cairo_rectangle(cairo_t *cr, Rect const& r) {
    cairo_rectangle(cr, r.left(), r.top(), r.width(), r.height());
}

void cairo_convex_hull(cairo_t *cr, ConvexHull const& ch) {
    if(ch.empty()) return;
    cairo_move_to(cr, ch.back());
    for(unsigned i = 0; i < ch.size(); i++) {
        cairo_line_to(cr, ch[i]);
    }
}

void cairo_curve(cairo_t *cr, Curve const& c) {
    if(LineSegment const* line_segment = dynamic_cast<LineSegment const*>(&c)) {
        cairo_line_to(cr, (*line_segment)[1][0], (*line_segment)[1][1]);
    }
    else if(QuadraticBezier const *quadratic_bezier = dynamic_cast<QuadraticBezier const*>(&c)) {
        std::vector<Point> points = quadratic_bezier->controlPoints();
        Point b1 = points[0] + (2./3) * (points[1] - points[0]);
        Point b2 = b1 + (1./3) * (points[2] - points[0]);
        cairo_curve_to(cr, b1[0], b1[1], 
                       b2[0], b2[1], 
                       points[2][0], points[2][1]);
    }
    else if(CubicBezier const *cubic_bezier = dynamic_cast<CubicBezier const*>(&c)) {
        std::vector<Point> points = cubic_bezier->controlPoints();
        cairo_curve_to(cr, points[1][0], points[1][1], points[2][0], points[2][1], points[3][0], points[3][1]);
    }
//    else if(EllipticalArc const *svg_elliptical_arc = dynamic_cast<EllipticalArc *>(c)) {
//        //TODO: get at the innards and spit them out to cairo
//    }
    else {
        //this case handles sbasis as well as all other curve types
        Path sbasis_path = cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            cairo_curve(cr, *iter);
        }
    }
}

void cairo_path(cairo_t *cr, Path const &p) {
    cairo_move_to(cr, p.initialPoint()[0], p.initialPoint()[1]);
    if(p.size() == 0) { // naked moveto
        cairo_move_to(cr, p.finalPoint()+Point(8,0));
        cairo_line_to(cr, p.finalPoint()+Point(-8,0));
        cairo_move_to(cr, p.finalPoint()+Point(0,8));
        cairo_line_to(cr, p.finalPoint()+Point(0,-8));
        return;
    }

    for(Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        cairo_curve(cr, *iter);
    }
    if(p.closed())
        cairo_close_path(cr);
}

void cairo_path_stitches(cairo_t *cr, Path const &p) {
    Path::const_iterator iter;
    for ( iter = p.begin() ; iter != p.end() ; ++iter ) {
        Curve const &c=*iter;
        if (dynamic_cast<Path::StitchSegment const *>(&c)) {
            cairo_move_to(cr, c.initialPoint()[X], c.initialPoint()[Y]);
            cairo_line_to(cr, c.finalPoint()[X], c.finalPoint()[Y]);
            
            //std::stringstream s;
            //s << L1(c.finalPoint() - c.initialPoint());
            //std::string ss = s.str();
            //draw_text(cr, c.initialPoint()+Point(5,5), ss.c_str(), false, "Serif 6");

            //std::cout << c.finalPoint() - c.initialPoint() << std::endl;
        }
    }
}

void cairo_path(cairo_t *cr, PathVector const &p) {
    PathVector::const_iterator it;
    for(it = p.begin(); it != p.end(); ++it) {
        cairo_path(cr, *it);
    }
}

void cairo_path_stitches(cairo_t *cr, PathVector const &p) {
    PathVector::const_iterator it;
    for ( it = p.begin() ; it != p.end() ; ++it ) {
        cairo_path_stitches(cr, *it);
    }
}


void cairo_d2_sb(cairo_t *cr, D2<SBasis> const &B) {
    cairo_path(cr, path_from_sbasis(B, 0.1));
}

void cairo_d2_pw_sb(cairo_t *cr, D2<Piecewise<SBasis> > const &p) {
    cairo_pw_d2_sb(cr, sectionize(p));
}

void cairo_pw_d2_sb(cairo_t *cr, Piecewise<D2<SBasis> > const &p) {
    for(unsigned i = 0; i < p.size(); i++)
        cairo_d2_sb(cr, p[i]);
}

#if PY_MAJOR_VERSION < 3
static Pycairo_CAPI_t *Pycairo_CAPI = 0;
#endif

cairo_t* cairo_t_from_object(boost::python::object cr) {
#if PY_MAJOR_VERSION < 3
  if(!Pycairo_CAPI)
    Pycairo_IMPORT;
#else
  import_cairo();
#endif
  PycairoContext* pcc = (PycairoContext*)cr.ptr();
  assert(PyObject_TypeCheck(pcc, &PycairoContext_Type));
  return PycairoContext_GET(pcc);
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
