#ifndef PATH_CAIRO
#define PATH_CAIRO
#include "path.h"
typedef struct _cairo cairo_t;

void cairo_sub_path(cairo_t *cr, Geom::SubPath const &p);
void cairo_path(cairo_t *cr, Geom::Path const &p);
void cairo_path_handles(cairo_t *cr, Geom::Path const &p);

#endif
