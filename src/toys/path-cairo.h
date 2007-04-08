#ifndef PATH_CAIRO
#define PATH_CAIRO

#include "s-basis.h"
#include "s-basis-2d.h"
#include "d2.h"
#include "pw.h"
#include "path2.h"
#include <vector>

typedef struct _cairo cairo_t;

void cairo_path(cairo_t *cr, Geom::Path2::Path const &p);
void cairo_path(cairo_t *cr, std::vector<Geom::Path2::Path> const &p);

void cairo_md_sb(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_md_sb_handles(cairo_t *cr, Geom::D2<Geom::SBasis> const &p);
void cairo_2dsb2d(cairo_t* cr, Geom::D2<Geom::SBasis2d> const &sb2, Geom::Point dir, double width);
void cairo_sb2d(cairo_t* cr, Geom::SBasis2d const &sb2, Geom::Point dir, double width);

void cairo_md_pw(cairo_t *cr, Geom::D2<Geom::Piecewise<Geom::SBasis> > const &p);
#endif
