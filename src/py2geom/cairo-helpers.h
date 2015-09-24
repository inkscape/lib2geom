#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <2geom/path.h>
#include <2geom/convex-hull.h>
#include <vector>

typedef struct _cairo cairo_t;

void cairo_move_to(cairo_t *cr, Geom::Point p1);
void cairo_line_to(cairo_t *cr, Geom::Point p1);
void cairo_curve_to(cairo_t *cr, Geom::Point p1, Geom::Point p2, Geom::Point p3);

void cairo_curve(cairo_t *cr, Geom::Curve const &c);
void cairo_rectangle(cairo_t *cr, Geom::Rect const &r);
void cairo_convex_hull(cairo_t *cr, Geom::ConvexHull const &r);
void cairo_path(cairo_t *cr, Geom::Path const &p);
void cairo_path(cairo_t *cr, Geom::PathVector const &p);
void cairo_path_stitches(cairo_t *cr, Geom::Path const &p);
void cairo_path_stitches(cairo_t *cr, Geom::PathVector const &p);

void cairo_d2_sb(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_d2_pw_sb(cairo_t *cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p);
void cairo_pw_d2_sb(cairo_t *cr, Geom::Piecewise<Geom::D2<Geom::SBasis> > const &p);

cairo_t* cairo_t_from_object(boost::python::object cr);
