#include "point.h"
#include <cairo.h>
#include <string>

void draw_line_seg(cairo_t *cr, Geom::Point a, Geom::Point b);
void draw_spot(cairo_t *cr, Geom::Point h);
void draw_handle(cairo_t *cr, Geom::Point h);
void draw_cross(cairo_t *cr, Geom::Point h);
void draw_circ(cairo_t *cr, Geom::Point h);
void draw_ray(cairo_t *cr, Geom::Point h, Geom::Point dir);

void cairo_move_to(cairo_t *cr, Geom::Point p1);
void cairo_line_to(cairo_t *cr, Geom::Point p1);
void cairo_curve_to(cairo_t *cr, Geom::Point p1, Geom::Point p2, Geom::Point p3);
