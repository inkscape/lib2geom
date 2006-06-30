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
#include "path-ops.h"
#include "matrix-rotate-ops.h"
#include "matrix-translate-ops.h"
#include "path-cairo.h"
#include <pango/pangocairo.h>

#include "path-metric.h"

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

using std::string;
using std::vector;

static GtkWidget *canvas;
static GdkGC *dash_gc;
static GdkGC *plain_gc;
Geom::SubPath display_path;
Geom::SubPath original_curve;

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

//line_intersection
/*static kind
segment_intersect(Geom::Point const &p00, Geom::Point const &p01,
				 Geom::Point const &p10, Geom::Point const &p11,
				 Geom::Point &result)
*/

void draw_elip(cairo_t *cr, Geom::Point *h) {
    draw_line_seg(cr, h[0], h[1]);
    draw_line_seg(cr, h[3], h[4]);
    draw_line_seg(cr, h[3], h[2]);
    draw_line_seg(cr, h[2], h[1]);

    Geom::Point c;
    line_twopoint_intersect(h[0], h[1], h[3], h[4], c);
    draw_handle(cr, c);
    
    Geom::Point old;
    for(int i = 0; i <= 100; i++) {
        double t = i/100.0;
        
        Geom::Point n = (1-t)*h[0] + t*h[3];
        Geom::Point c1, c2;
        line_twopoint_intersect(2*c-n, n, h[0], h[2], c1);
        line_twopoint_intersect(2*c-n, n, h[4], h[2], c2);
        Geom::Point six;
        line_twopoint_intersect(c1, h[3], c2, h[1], six);
        if(i)
            draw_line_seg(cr, old, six);
        old = six;
    }
}

void draw_path(cairo_t *cr, Geom::SubPath const & p) {
    path_to_polyline pl(p, 1);
    
    Geom::Point old(pl.handles[0]);
    
    for(unsigned i = 1; i < pl.handles.size(); i++) {
        Geom::Point p(pl.handles[i]);
        draw_line_seg(cr, old, p);
        old = p;
    }
    
}

#include "centroid.h"

Geom::Point path_centroid_polyline(Geom::SubPath const & p, double &area) {
    path_to_polyline pl(p, 1);
    Geom::Point centr;
    Geom::centroid(pl.handles,  centr, area);
    
    return centr;
}

Geom::SubPath::Location param(Geom::SubPath const & p, double t) {
    double T = t*(p.size()-1);
    //std::cout << T << ", " <<  T-int(T) << std::endl;
    return Geom::SubPath::Location(p.indexed_elem(int(T)), T - int(T));
}

Geom::Point* selected_handle = 0;
Geom::Point gradient_vector(20,20);

bool rotater  = false, evolution= false, half_stroking=false;
bool involution = false;
bool equal_arc = false;

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
    Geom::SubPath edges;
    edges.cmd.push_back(Geom::quadto);
    edges.handles.push_back(Geom::Point(0,0));
    edges.handles.push_back(Geom::Point(0,200));
    edges.handles.push_back(Geom::Point(0,400));
    edges.cmd.push_back(Geom::quadto);
    edges.handles.push_back(Geom::Point(200,400));
    edges.handles.push_back(Geom::Point(400,400));
    edges.cmd.push_back(Geom::quadto);
    edges.handles.push_back(Geom::Point(300,300));
    edges.handles.push_back(Geom::Point(400,0));
    edges.cmd.push_back(Geom::quadto);
    edges.handles.push_back(Geom::Point(200,40));
    edges.handles.push_back(Geom::Point(0,0));
    
    for(double u = 0; u <= 1; u+=0.125) {
        Geom::Point L = edges.point_at(Geom::SubPath::Location(edges.begin(), u));
        Geom::Point R = edges.point_at(Geom::SubPath::Location(edges.indexed_elem(2), 1-u));
        for(double v = 0; v < 1.; v+=0.125) {
            Geom::Point T = edges.point_at(Geom::SubPath::Location(edges.indexed_elem(1), v));
            Geom::Point B = edges.point_at(Geom::SubPath::Location(edges.indexed_elem(3), 1-v));
            double U = (T[Geom::Y] - B[Geom::Y]) / 400.;
            Geom::Point V = (1-U)*L + U*R;
            if(v == 0)
                cairo_move_to(cr, V);
            else
                cairo_line_to(cr, V);
        }
    }
    cairo_stroke(cr);
    cairo_sub_path(cr, display_path);
    cairo_stroke(cr);
    
    cairo_set_source_rgba (cr, 0.5, 0.7, 0.3, 0.8);
    cairo_sub_path(cr, original_curve);
    
    notify << "pathwise Area: " ;

    {
        notify << std::ends;
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, notify.str().c_str());
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, 0, height-logical_extent.height);
        pango_cairo_show_layout(cr, layout);
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
        if(Geom::L2(mouse - gradient_vector) < 5) {
            selected_handle = &gradient_vector;
            old_handle_pos = mouse - gradient_vector;
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

#include "path-find-points-of-interest.h"
#include "cubic_bez_util.h"
#include "poly.h"
#include "path-poly-fns.h"
#include <gsl/gsl_integration.h>

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer) {
    gint ret = FALSE;
    if (event->keyval == ' ') {
        ret = TRUE;
    } else if (event->keyval == 'l') {
        ret = TRUE;
    } else if (event->keyval == 'q') {
        exit(0);
    } else if (event->keyval == 'r') {
        rotater = !rotater;
    } else if (event->keyval == 'e') {
        // print out elliptic integrals and solutions
    } else if (event->keyval == 'v') {
        evolution = !evolution;
    } else if (event->keyval == 'n') {
        involution = !involution;
    } else if (event->keyval == 's') {
        half_stroking = !half_stroking;
    } else if (event->keyval == 'a') {
        equal_arc = !equal_arc;
        
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

#include "translate-ops.h"
#include "scale-ops.h"
#include "translate-scale-ops.h"

static gboolean idler(GtkWidget* widget) {
    if(rotater)
        gtk_widget_queue_draw(widget);
    return TRUE;
}

static void
on_open_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //TODO: show open dialog, get filename
    
    char const *const filename = "banana.svgd";

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }
    display_path = read_svgd(f).subpaths.front();
    
    gtk_widget_queue_draw(canvas); // globals are probably evil
}

static void
on_about_activate(GtkMenuItem *menuitem, gpointer user_data) {
    
}


int main(int argc, char **argv) {
    char const *const filename = (argc >= 2
                                  ? argv[1]
                                  : "toy.svgd");
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return 1;
    }
    display_path = read_svgd(f).subpaths.front();

    Geom::Rect r = display_path.bbox();
    
    display_path = display_path*Geom::translate(-r.min());
    Geom::scale sc(r.max() - r.min());
    display_path = display_path*(sc.inverse()*Geom::scale(500,500));
    original_curve = display_path;
    
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();
    GtkWidget *menubox;
    GtkWidget *menubar;
    GtkWidget *menuitem;
    GtkWidget *menu;
    GtkWidget *open;
    GtkWidget *separatormenuitem;
    GtkWidget *quit;
    GtkWidget *menuitem2;
    GtkWidget *menu2;
    GtkWidget *about;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new ();
 
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "text toy");

    menubox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (menubox);
    gtk_container_add (GTK_CONTAINER (window), menubox);

    menubar = gtk_menu_bar_new ();
    gtk_widget_show (menubar);
    gtk_box_pack_start (GTK_BOX (menubox), menubar, FALSE, FALSE, 0);

    menuitem = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem);

    menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    open = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_widget_show (open);
    gtk_container_add (GTK_CONTAINER (menu), open);

    separatormenuitem = gtk_separator_menu_item_new ();
    gtk_widget_show (separatormenuitem);
    gtk_container_add (GTK_CONTAINER (menu), separatormenuitem);
    gtk_widget_set_sensitive (separatormenuitem, FALSE);

    quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
    gtk_widget_show (quit);
    gtk_container_add (GTK_CONTAINER (menu), quit);

    menuitem2 = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (menuitem2);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem2);

    menu2 = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

    about = gtk_menu_item_new_with_mnemonic ("_About");
    gtk_widget_show (about);
    gtk_container_add (GTK_CONTAINER (menu2), about);

    g_signal_connect ((gpointer) open, "activate",
                    G_CALLBACK (on_open_activate),
                    NULL);
    g_signal_connect ((gpointer) quit, "activate",
                    gtk_main_quit,
                    NULL);
    g_signal_connect ((gpointer) about, "activate",
                    G_CALLBACK (on_about_activate),
                    NULL);

    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);


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

    //GtkWidget *vb = gtk_vbox_new(0, 0);


    //gtk_container_add(GTK_CONTAINER(window), vb);

    gtk_box_pack_start(GTK_BOX(menubox), canvas, TRUE, TRUE, 0);

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
    
    g_idle_add((GSourceFunc)idler, canvas);

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
