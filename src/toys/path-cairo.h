#ifndef PATH_CAIRO
#define PATH_CAIRO
#include "path.h"

typedef struct _cairo cairo_t;

void cairo_sub_path(cairo_t *cr, Geom::Path const &p);
void cairo_PathSet(cairo_t *cr, Geom::PathSet const &p);
void cairo_PathSet_handles(cairo_t *cr, Geom::PathSet const &p);
void cairo_sub_path_handles(cairo_t *cr, Geom::PathSet const &p);

void cairo_md_sb(cairo_t *cr, multidim_sbasis<2>  const &p);
void cairo_md_sb_handles(cairo_t *cr, multidim_sbasis<2> const &p);

#endif
