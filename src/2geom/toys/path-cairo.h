#ifndef PATH_CAIRO
#define PATH_CAIRO


#include <cairo.h>
#include <2geom/line.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <2geom/path.h>
#include <2geom/convex-hull.h>
#include <vector>
#include <string>

typedef struct _cairo cairo_t;

void cairo_curve(cairo_t *cr, Geom::Curve const &c);
void cairo_rectangle(cairo_t *cr, Geom::Rect const &r);
void cairo_convex_hull(cairo_t *cr, Geom::ConvexHull const &r);
void cairo_path(cairo_t *cr, Geom::Path const &p);
void cairo_path(cairo_t *cr, Geom::PathVector const &p);
void cairo_path_stitches(cairo_t *cr, Geom::Path const &p);
void cairo_path_stitches(cairo_t *cr, Geom::PathVector const &p);

void cairo_d2_sb(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_d2_sb_handles(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_d2_sb2d(cairo_t* cr, Geom::D2<Geom::SBasis2d> const &sb2, Geom::Point dir, double width);
void cairo_sb2d(cairo_t* cr, Geom::SBasis2d const &sb2, Geom::Point dir, double width);

void cairo_d2_pw_sb(cairo_t *cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p);
void cairo_pw_d2_sb(cairo_t *cr, Geom::Piecewise<Geom::D2<Geom::SBasis> > const &p);


void draw_line(cairo_t *cr, const Geom::Line& l, const Geom::Rect& r);
void draw_line(cairo_t *cr, Geom::Point n, double d, Geom::Rect r);
void draw_line(cairo_t *cr, Geom::Point a, Geom::Point b, Geom::Rect r);

void draw_line_seg(cairo_t *cr, Geom::Point a, Geom::Point b);
void draw_line_seg_with_arrow(cairo_t *cr, Geom::Point a, Geom::Point b, double dangle = 15*M_PI/180, double radius = 20);
void draw_spot(cairo_t *cr, Geom::Point h);
void draw_handle(cairo_t *cr, Geom::Point h);
void draw_cross(cairo_t *cr, Geom::Point h);
void draw_circ(cairo_t *cr, Geom::Point h);
void draw_ray(cairo_t *cr, Geom::Point h, Geom::Point dir);
void draw_ray(cairo_t *cr, const Geom::Ray& ray, const Geom::Rect& r);
void draw_line_segment(cairo_t *cr, const Geom::LineSegment& ls, const Geom::Rect& r);

void cairo_move_to(cairo_t *cr, Geom::Point p1);
void cairo_line_to(cairo_t *cr, Geom::Point p1);
void cairo_curve_to(cairo_t *cr, Geom::Point p1, Geom::Point p2, Geom::Point p3);

// H in [0,360)
// S, V, R, G, B in [0,1]
void convertHSVtoRGB(const double H, const double S, const double V,
                     double& R, double& G, double& B);
#endif
