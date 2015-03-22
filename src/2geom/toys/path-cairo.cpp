#include <cairo.h>
#include <2geom/toys/path-cairo.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/utils.h>
#include "toy-framework-2.h"
#include <sstream>

using namespace Geom;

void cairo_rectangle(cairo_t *cr, Rect const& r) {
    cairo_rectangle(cr, r.left(), r.top(), r.width(), r.height());
}

void cairo_convex_hull(cairo_t *cr, ConvexHull const& ch) {
    if(ch.empty()) return;
    cairo_move_to(cr, ch[ch.size()-1]);
    for(unsigned i = 0; i < ch.size(); i++) {
        cairo_line_to(cr, ch[i]);
    }
}

void cairo_curve(cairo_t *cr, Curve const& c) {
    if(!cairo_has_current_point(cr))
        cairo_move_to(cr, c.initialPoint());

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

            std::stringstream s;
            s << L1(c.finalPoint() - c.initialPoint());
            std::string ss = s.str();
            draw_text(cr, c.initialPoint()+Point(5,5), ss.c_str(), false, "Serif 6");

            //std::cout << c.finalPoint() - c.initialPoint() << std::endl;
        }
    }
}

void cairo_path_handles(cairo_t */*cr*/, Path const &/*p*/) {
    //TODO
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

void cairo_d2_sb2d(cairo_t* cr, D2<SBasis2d> const &sb2, Point /*dir*/, double width) {
    D2<SBasis> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);// + Linear(u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_d2_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);// + Linear(v);
        B[0] = extract_v(sb2[0], v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_d2_sb(cr, B);
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
        cairo_d2_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2, v)*dir[1] + Linear(v);
        B[0] = SBasis(Linear(0,1)) + extract_v(sb2, v)*dir[0];
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = B[i]*(width/2) + Linear(width/4);
        }
        cairo_d2_sb(cr, B);
    }
}

void cairo_d2_pw_sb(cairo_t *cr, D2<Piecewise<SBasis> > const &p) {
    cairo_pw_d2_sb(cr, sectionize(p));
}

void cairo_pw_d2_sb(cairo_t *cr, Piecewise<D2<SBasis> > const &p) {
    for(unsigned i = 0; i < p.size(); i++)
        cairo_d2_sb(cr, p[i]);
}


void draw_line_seg(cairo_t *cr, Geom::Point a, Geom::Point b) {
    cairo_move_to(cr, a[0], a[1]);
    cairo_line_to(cr, b[0], b[1]);
    cairo_stroke(cr);
}

void draw_spot(cairo_t *cr, Geom::Point h) {
    draw_line_seg(cr, h, h);
}

void draw_handle(cairo_t *cr, Geom::Point h) {
    double x = h[Geom::X];
    double y = h[Geom::Y];
    cairo_move_to(cr, x-3, y);
    cairo_line_to(cr, x+3, y);
    cairo_move_to(cr, x, y-3);
    cairo_line_to(cr, x, y+3);
}

void draw_cross(cairo_t *cr, Geom::Point h) {
    double x = h[Geom::X];
    double y = h[Geom::Y];
    cairo_move_to(cr, x-3, y-3);
    cairo_line_to(cr, x+3, y+3);
    cairo_move_to(cr, x+3, y-3);
    cairo_line_to(cr, x-3, y+3);
}

void draw_circ(cairo_t *cr, Geom::Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x, y, 3, 0, M_PI*2);
    cairo_stroke(cr);
}

void draw_ray(cairo_t *cr, Geom::Point h, Geom::Point dir) {
    draw_line_seg(cr, h, h+dir);
    Point unit = 3*unit_vector(dir),
          rot = rot90(unit);
    draw_line_seg(cr, h+dir, h + dir - unit + rot);
    draw_line_seg(cr, h+dir, h + dir - unit - rot);
}


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
/*
  void draw_string(GtkWidget *widget, string s, int x, int y) {
  PangoLayout *layout = gtk_widget_create_pango_layout(widget, s.c_str());
  cairo_t* cr = gdk_cairo_create (widget->window);
  cairo_move_to(cr, x, y);
  pango_cairo_show_layout(cr, layout);
  cairo_destroy (cr);
  }*/

// H in [0,360)
// S, V, R, G, B in [0,1]
void convertHSVtoRGB(const double H, const double S, const double V,
                     double& R, double& G, double& B) {
    int Hi = int(floor(H/60.)) % 6;
    double f = H/60. - Hi;
    double p = V*(1-S);
    double q = V*(1-f*S);
    double t = V*(1-(1-f)*S);
    switch(Hi) {
    case 0: R=V, G=t, B=p; break;
    case 1: R=q, G=V, B=p; break;
    case 2: R=p, G=V, B=t; break;
    case 3: R=p, G=q, B=V; break;
    case 4: R=t, G=p, B=V; break;
    case 5: R=V, G=p, B=q; break;
    }
}

void draw_line(cairo_t *cr, double a, double b, double c, const Geom::Rect& r) {
    Geom::Line l(a, b, c);
    boost::optional<Geom::LineSegment> seg = l.clip(r);
    if (seg) {
        cairo_move_to(cr, seg->initialPoint());
        cairo_line_to(cr, seg->finalPoint());
        cairo_stroke(cr);
    }
}


void draw_line(cairo_t* cr, const Geom::Line& l, const Geom::Rect& r)
{
    std::vector<double> coeff = l.coefficients();
    draw_line (cr, coeff[0], coeff[1], coeff[2], r);
}


void draw_line(cairo_t *cr, Geom::Point n, double dist, Geom::Rect r) {
    draw_line(cr, n[0], n[1], dist, r);
}


void draw_ray(cairo_t *cr, const Geom::Ray& ray, const Geom::Rect& r)
{
    LineSegment ls;

    for (size_t i = 0; i < 4; ++i)
    {
        ls.setInitial (r.corner(i));
        ls.setFinal (r.corner(i+1));
        OptCrossing cx = intersection (ls, ray);
        if (cx)
        {
            Point P = ray.pointAt ((*cx).tb);
            draw_line_seg (cr, ray.origin(), P);
            break;
        }
    }
}

void draw_line_segment(cairo_t *cr, const Geom::LineSegment& ls, const Geom::Rect& r)
{
    if(r.contains(ls[0])) {
        if(r.contains(ls[1])) {
            draw_line_seg(cr, ls[0], ls[1]);
        } else {
            draw_ray(cr, Geom::Ray(ls[0], ls[1]), r);
        }

    } else {
        if(r.contains(ls[1])) {
            draw_ray(cr, Geom::Ray(ls[1], ls[0]), r);
        } else {
            draw_line(cr, Geom::Line(ls[0], ls[1]), r);
        }

    }
}

void draw_line_seg_with_arrow(cairo_t *cr, Geom::Point a, Geom::Point b, double dangle, double radius) {
    double angle = atan2(a-b);
    cairo_move_to(cr, a);
    cairo_line_to(cr, b);
    
    cairo_move_to(cr, b);
    cairo_line_to(cr, Point::polar(angle + dangle, radius) + b);
    cairo_move_to(cr, b);
    cairo_line_to(cr, Point::polar(angle - dangle, radius) + b);
    cairo_stroke(cr);
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
