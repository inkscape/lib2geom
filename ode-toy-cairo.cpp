#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "maybe.h"
#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "geom.h"
#include "path.h"
#include "path-to-polyline.h"
#include "read-svgd.h"
#include "path-find-points-of-interest.h"
#include "rotate.h"
#include "rotate-ops.h"
#include "arc-length.h"
#include "path-intersect.h"

using std::string;
using std::vector;

static GtkWidget *canvas;
static GdkGC *dash_gc;
static GdkGC *plain_gc;
Geom::Path display_path;

static Geom::Point old_handle_pos;
static Geom::Point old_mouse_point;

void draw_line_seg(cairo_t *cr, Geom::Point a, Geom::Point b) {
    cairo_move_to(cr, a[0], a[1]);
    cairo_line_to(cr, b[0], b[1]);
    cairo_stroke(cr);
}

void draw_spot(cairo_t *cr, Geom::Point h) {
    draw_line_seg(cr, h, h);
}

void draw_handle(cairo_t *cr, Geom::Point h, string name = string("")) {
    double x = h[Geom::X];
    double y = h[Geom::Y];
    cairo_move_to(cr, x-3, y);
    cairo_line_to(cr, x+3, y);
    cairo_move_to(cr, x, y-3);
    cairo_line_to(cr, x, y+3);
    //templayout.set_text(name);
    //w.draw_layout(g, x, y, templayout);
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

void draw_path(cairo_t *cr, Geom::Path p) {
    if(p.empty()) return;
    path_to_polyline pl(p, 1);
    
    Geom::Point old(pl.handles[0]);
    
    for(unsigned i = 1; i < pl.handles.size(); i++) {
        Geom::Point p(pl.handles[i]);
        draw_line_seg(cr, old, p);
        old = p;
    }
    
}

Geom::Point* selected_handle = 0;


#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
     
int
func (double t, const double y[], double f[],
      void *params)
{
    double mu = *(double *)params;
    f[0] = y[1];
    f[1] = -y[0] - mu*y[1]*(y[0]*y[0] - 1);
    return GSL_SUCCESS;
}
     
int
jac (double t, const double y[], double *dfdy, 
     double dfdt[], void *params)
{
    double mu = *(double *)params;
    gsl_matrix_view dfdy_mat 
        = gsl_matrix_view_array (dfdy, 2, 2);
    gsl_matrix * m = &dfdy_mat.matrix; 
    gsl_matrix_set (m, 0, 0, 0.0);
    gsl_matrix_set (m, 0, 1, 1.0);
    gsl_matrix_set (m, 1, 0, -2.0*mu*y[0]*y[1] - 1.0);
    gsl_matrix_set (m, 1, 1, -mu*(y[0]*y[0] - 1.0));
    dfdt[0] = 0.0;
    dfdt[1] = 0.0;
    return GSL_SUCCESS;
}

void
hmm (cairo_t *cr)
{
    const gsl_odeiv_step_type * T 
        = gsl_odeiv_step_rk8pd;
     
    gsl_odeiv_step * s 
        = gsl_odeiv_step_alloc (T, 2);
    gsl_odeiv_control * c 
        = gsl_odeiv_control_y_new (1e-6, 0.0);
    gsl_odeiv_evolve * e 
        = gsl_odeiv_evolve_alloc (2);
     
    double mu = 10;
    gsl_odeiv_system sys = {func, jac, 2, &mu};
     
    double t = 0.0, t1 = 100.0;
    double h = 1e-6;
    double y[2] = { 1.0, 0.0 };
     
    cairo_move_to(cr, 10*y[0]+100, 10*y[1]+100);
    for (int i = 1; i <= 100; i++)
    {
        double ti = i * t1 / 100.0;
        
        while (t < ti)
        {
            gsl_odeiv_evolve_apply (e, c, s,
                                    &sys,
                                    &t, ti, &h,
                                    y);
            cairo_line_to(cr, 10*y[0]+100, 10*y[1]+100);
        }
        
        //printf ("%.5e %.5e %.5e\n", t, y[0], y[1]);
    }
    cairo_stroke(cr);

     
    gsl_odeiv_evolve_free (e);
    gsl_odeiv_control_free (c);
    gsl_odeiv_step_free (s);
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    cairo_set_line_width (cr, 1);
    //draw (clock, cr);

    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);

    for(int i = 0; i < display_path.handles.size(); i++) {
        draw_handle(cr, display_path.handles[i]);
    }
    draw_path(cr, display_path);
    //draw_elip(cr, handles);
    
    Geom::Point dir(1,1);
    /*
    vector<Geom::Path::PathLocation> pts = find_vector_extreme_points(display_path, dir);
    
    for(int i = 0; i < pts.size(); i++) {
        draw_circ(cr, display_path.point_at(pts[i]));
        }*/
    
    double dist = INFINITY;
    cairo_set_line_width (cr, 0.1);
    hmm(cr);
    cairo_save(cr);
    cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);

    cairo_restore(cr);

    notify << "Area: " << 30.;
    

    {
        notify << std::ends;
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, notify.str().c_str());
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, 0, height-logical_extent.height);
        cairo_show_text (cr, notify.str().c_str());
    }
    
    cairo_destroy (cr);
    
    return TRUE;
}

static void handle_mouse(GtkWidget* widget) {
    gtk_widget_queue_draw (widget);
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    Geom::Point mouse(e->x, e->y);
    
    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        if(selected_handle) {
            *selected_handle = mouse - old_handle_pos;
            
        }
        handle_mouse(widget);
    }

    if(e->state & (GDK_BUTTON2_MASK)) {
        gtk_widget_queue_draw(widget);
    }
    
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    Geom::Point mouse(e->x, e->y);
    if(e->button == 1 || e->button == 3) {
        for(int i = 0; i < display_path.handles.size(); i++) {
            if(Geom::L2(mouse - display_path.handles[i]) < 5) {
                selected_handle = &display_path.handles[i];
                old_handle_pos = mouse - display_path.handles[i];
            }
        }
        handle_mouse(window);
    } else if(e->button == 2) {
        gtk_widget_queue_draw(window);
    }
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_release_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    selected_handle = 0;
    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer) {
    gint ret = FALSE;
    if (event->keyval == ' ') {
        ret = TRUE;
    } else if (event->keyval == 'l') {
        ret = TRUE;
    } else if (event->keyval == 'd') {
        write_svgd(stderr, display_path);
        ret = TRUE;
    } else if ('0' <= event->keyval
               && event->keyval <= '9') {
        ret = TRUE;
    } else if ('h' == event->keyval) {
        ret = TRUE;
    }

    if (ret) {
        gtk_widget_queue_draw(widget);
    }

    return ret;
}

static gint
delete_event_cb(GtkWidget* window, GdkEventAny* e, gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char **argv) {
    
    gtk_init (&argc, &argv);

    gdk_rgb_init();

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "text toy");

    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete_event",
                       GTK_SIGNAL_FUNC(delete_event_cb),
                       NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_signal_connect(GTK_OBJECT (canvas),
                       "expose_event",
                       GTK_SIGNAL_FUNC(expose_event),
                       0);
    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK |
                                   GDK_BUTTON_RELEASE_MASK |
                                   GDK_KEY_PRESS_MASK    |
                                   GDK_POINTER_MOTION_MASK));
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_press_event",
                       GTK_SIGNAL_FUNC(mouse_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_release_event",
                       GTK_SIGNAL_FUNC(mouse_release_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "motion_notify_event",
                       GTK_SIGNAL_FUNC(mouse_motion_event),
                       0);
    gtk_signal_connect(GTK_OBJECT(canvas),
                       "key_press_event",
                       GTK_SIGNAL_FUNC(key_release_event),
                       0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    GtkWidget *vb = gtk_vbox_new(0, 0);


    gtk_container_add(GTK_CONTAINER(window), vb);

    gtk_box_pack_start(GTK_BOX(vb), canvas, TRUE, TRUE, 0);

    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    gtk_widget_show_all(window);

    dash_gc = gdk_gc_new(canvas->window);
    gint8 dash_list[] = {4, 4};
    gdk_gc_set_dashes(dash_gc, 0, dash_list, 2);
    GdkColor colour;
    colour.red = 0xffff;
    colour.green = 0xffff;
    colour.blue = 0xffff;

    plain_gc = gdk_gc_new(canvas->window);
    
    //gdk_gc_set_rgb_fg_color(dash_gc, &colour);
    gdk_rgb_find_color(gtk_widget_get_colormap(canvas), &colour);
    gdk_window_set_background(canvas->window, &colour);
    gdk_gc_set_line_attributes(dash_gc, 1, GDK_LINE_ON_OFF_DASH,
                               GDK_CAP_BUTT,GDK_JOIN_MITER);

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    gtk_main();

    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
