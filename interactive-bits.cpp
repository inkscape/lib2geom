#include "interactive-bits.h"
//#include <pango/pangocairo.h>
#include "point-ops.h"

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

void draw_circ(cairo_t *cr, Geom::Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x, y, 3, 0, M_PI*2);
    cairo_stroke(cr);
}

void draw_ray(cairo_t *cr, Geom::Point h, Geom::Point dir) {
    draw_line_seg(cr, h, h+dir);
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

