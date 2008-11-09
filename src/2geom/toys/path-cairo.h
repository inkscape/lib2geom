#ifndef PATH_CAIRO
#define PATH_CAIRO

#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <2geom/path.h>
#include <2geom/convex-cover.h>
#include <vector>

typedef struct _cairo cairo_t;

void cairo_curve(cairo_t *cr, Geom::Curve const &c);
void cairo_rectangle(cairo_t *cr, Geom::Rect const &r);
void cairo_convex_hull(cairo_t *cr, Geom::ConvexHull const &r);
void cairo_path(cairo_t *cr, Geom::Path const &p);
void cairo_path(cairo_t *cr, std::vector<Geom::Path> const &p);
void cairo_path_stitches(cairo_t *cr, Geom::Path const &p);
void cairo_path_stitches(cairo_t *cr, std::vector<Geom::Path> const &p);

void cairo_d2_sb(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_d2_sb_handles(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_d2_sb2d(cairo_t* cr, Geom::D2<Geom::SBasis2d> const &sb2, Geom::Point dir, double width);
void cairo_sb2d(cairo_t* cr, Geom::SBasis2d const &sb2, Geom::Point dir, double width);

void cairo_d2_pw_sb(cairo_t *cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p);
void cairo_pw_d2_sb(cairo_t *cr, Geom::Piecewise<Geom::D2<Geom::SBasis> > const &p);
#endif
