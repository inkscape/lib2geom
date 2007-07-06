#ifndef PATH_CAIRO
#define PATH_CAIRO

#include "sbasis.h"
#include "sbasis-2d.h"
#include "d2.h"
#include "piecewise.h"
#include "path.h"
#include <vector>

typedef struct _cairo cairo_t;

void cairo_path(cairo_t *cr, Geom::Path const &p);
void cairo_path(cairo_t *cr, std::vector<Geom::Path> const &p);

void cairo_md_sb(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_md_sb_handles(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_2dsb2d(cairo_t* cr, Geom::D2<Geom::SBasis2d> const &sb2, Geom::Point dir, double width);
void cairo_sb2d(cairo_t* cr, Geom::SBasis2d const &sb2, Geom::Point dir, double width);

void cairo_d2_pw(cairo_t *cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p);
void cairo_pw_d2(cairo_t *cr, Geom::Piecewise<Geom::D2<Geom::SBasis> > const &p);
#endif
