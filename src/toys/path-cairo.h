#ifndef PATH_CAIRO
#define PATH_CAIRO
#include "path2.h"
#include "s-basis-2d.h"
#include <vector>
#include "md-pw-sb.h"

typedef struct _cairo cairo_t;

void cairo_path(cairo_t *cr, Geom::Path2::Path const &p);
void cairo_path(cairo_t *cr, std::vector<Geom::Path2::Path> const &p);

void cairo_md_sb(cairo_t *cr, Geom::MultidimSBasis<2>  const &p);
void cairo_md_sb_handles(cairo_t *cr, Geom::MultidimSBasis<2> const &p);
void cairo_sb2d(cairo_t* cr, std::vector<Geom::SBasis2d> const &sb2, Geom::Point dir, double width);
void draw_sb2d(cairo_t* cr, Geom::SBasis2d const &sb2, Geom::Point dir, double width);

void cairo_md_pw(cairo_t *cr, Geom::md_pw_sb<2> const &p);
#endif
