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
#include "interactive-bits.h"

#include "path-metric.h"

using std::string;
using std::vector;

static GtkWidget *canvas;
static GdkGC *dash_gc;
static GdkGC *plain_gc;
Geom::SubPath display_path;
Geom::SubPath original_curve;

static Geom::Point old_handle_pos;
static Geom::Point old_mouse_point;

#include "centroid.h"

Geom::SubPath::Location param(Geom::SubPath const & p, double t) {
    double T = t*(p.size()-1);
    //std::cout << T << ", " <<  T-int(T) << std::endl;
    return Geom::SubPath::Location(p.indexed_elem(int(T)), T - int(T));
}

void draw_evolute(cairo_t *cr, Geom::SubPath const & p) {
    int i = 0;
    for(double t = 0; t <= 1.0; t+= 1./1024) {
        Geom::SubPath::Location pl = param(p, t);
        
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        double kurvature = dot(acc, rot90(tgt))/pow(Geom::L2(tgt),3);
        
        Geom::Point pt = pos + 10*rot90(unit_vector(tgt));
        if(fabs(kurvature) > 0.0001) {
            Geom::Point kurv_vector = (1./kurvature)*Geom::unit_vector(rot90(tgt));
            kurv_vector += pos;
            //kurvature = fabs(kurvature);
            pt = kurv_vector;
        }
        if(i)
            cairo_line_to(cr, pt);
        else 
            cairo_move_to(cr, pt);
        i++;
    }
}

void draw_involute(cairo_t *cr, Geom::SubPath const & p) {
    int i = 0;
    double sl = arc_length_integrating(p, 1e-3);
    for(double s = 0; s < sl; s+= 5.0) {
        Geom::SubPath::Location pl = 
            natural_parameterisation(display_path, s, 1e-3);
        
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        Geom::Point pt = pos - 0.1*s*unit_vector(tgt);
        if(i)
            cairo_line_to(cr, pt);
        else 
            cairo_move_to(cr, pt);
        i++;
    }
}

void draw_stroke(cairo_t *cr, Geom::SubPath const & p) {
    int i = 0;
    for(double t = 0; t <= 1.0; t+= 1./1024) {
        Geom::SubPath::Location pl = param(p, t);
        
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        Geom::Point pt = pos + 10*rot90(unit_vector(tgt));
        if(i)
            cairo_line_to(cr, pt);
        else 
            cairo_move_to(cr, pt);
        i++;
    }
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

    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);

    std::vector<Geom::Point> const & hand(display_path.get_handles());
    for(int i = 0; i < hand.size(); i++) {
        draw_handle(cr, hand[i]);
    }
    if(rotater) {
        Geom::Point cntr;
        double area;
        centroid(display_path, cntr, area);
        Geom::Matrix m(Geom::translate(-cntr));
        m = (m * Geom::rotate(0.01)) * Geom::translate(cntr);
        display_path = display_path * m;
    }
    if(evolution) {
        draw_evolute(cr, display_path);
    }
    if(involution) {
        draw_involute(cr, display_path);
    }
    if(half_stroking) {
        draw_stroke(cr, display_path);
    }
    if(equal_arc) {
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0.5, 0, 0.5, 0.8);
        double sl = arc_length_integrating(display_path, 1e-3);
        for(double s = 0; s < sl; s+= 50.0) {
            Geom::SubPath::Location pl = 
                natural_parameterisation(display_path, s, 1e-3);
            
            draw_circ(cr, display_path.point_at(pl));
        }
        cairo_restore(cr);
    }
    Geom::SubPath::HashCookie hash_cookie = display_path;
    {
        draw_line_seg(cr, Geom::Point(10,10), gradient_vector);
        draw_handle(cr, gradient_vector);
        std::ostringstream gradientstr;
        gradientstr << "gradient: " << gradient_vector - Geom::Point(10,10);
        gradientstr << std::ends;
        cairo_move_to(cr, gradient_vector[0], gradient_vector[1]);
        cairo_save(cr);
        {
            PangoLayout* layout = pango_cairo_create_layout (cr);
            pango_layout_set_text(layout, 
                                  gradientstr.str().c_str(), -1);

            PangoFontDescription *font_desc = pango_font_description_new();
            pango_font_description_set_family(font_desc, "Sans");
            const int size_px = 10;
            pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
            pango_layout_set_font_description(layout, font_desc);
            PangoRectangle logical_extent;
            pango_layout_get_pixel_extents(layout,
                                           NULL,
                                           &logical_extent);
            cairo_rotate(cr, atan2(gradient_vector));
            pango_cairo_show_layout(cr, layout);
        }
        cairo_restore(cr);
    }    
    display_path.set_closed(true);
    cairo_sub_path(cr, display_path);
    
    cairo_set_source_rgba (cr, 0.5, 0.7, 0.3, 0.8);
    //cairo_sub_path(cr, original_curve);
    
    double dist = INFINITY;

    cairo_save(cr);
    cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);

    Geom::SubPath::Location pl = 
        display_path.nearest_location(old_mouse_point, dist);
    assert(hash_cookie == display_path);
    {
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        draw_circ(cr, pos);
        double kurvature = dot(acc, rot90(tgt))/pow(Geom::L2(tgt),3);
        
        if(fabs(kurvature) > 0.001) {
            Geom::Point kurv_vector = (1./kurvature)*Geom::unit_vector(rot90(tgt));
            draw_ray(cr, pos, kurv_vector);
            cairo_new_sub_path(cr);
            double c_angle = atan2(kurv_vector)+M_PI;
            kurv_vector += pos;
            kurvature = fabs(kurvature);
            double angle_delta = std::min(100*kurvature, M_PI);
            cairo_arc(cr, kurv_vector[0], kurv_vector[1], (1./kurvature), c_angle-angle_delta, c_angle+angle_delta);
            
        } else // just normal
            draw_ray(cr, pos, 50*Geom::unit_vector(rot90(tgt)));
        std::ostringstream gradientstr;
        gradientstr << "arc_position: " << arc_length_integrating(display_path, pl, 1e-3);
        gradientstr << std::ends;
        cairo_move_to(cr, pos[0]+5, pos[1]);
        {
            PangoLayout* layout = pango_cairo_create_layout (cr);
            pango_layout_set_text(layout, 
                                  gradientstr.str().c_str(), -1);

            PangoFontDescription *font_desc = pango_font_description_new();
            pango_font_description_set_family(font_desc, "Sans");
            const int size_px = 12;
            pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
            pango_layout_set_font_description(layout, font_desc);
            PangoRectangle logical_extent;
            pango_layout_get_pixel_extents(layout,
                                           NULL,
                                           &logical_extent);
            cairo_save(cr);
            cairo_rotate(cr, atan2(rot90(tgt)));
            pango_cairo_show_layout(cr, layout);
            cairo_restore(cr);
        }
        
    }
    //notify << "sub path length: " << arc_length_subdividing(display_path, 1e-3) << "\n";
    cairo_restore(cr);
/*    
      cairo_save(cr);
      cairo_set_source_rgba (cr, 0.25, 0.25, 0., 0.8);

    Geom::SubPath pth = display_path.subpath(display_path.indexed_elem(2), display_path.end());
    pth = pth*Geom::translate(Geom::Point(30, 30));
    draw_path(cr, pth);
    Bezier a, b;
    const int curve_seg = 3;
    Geom::SubPath::Elem ai(*display_path.indexed_elem(curve_seg)), bi(*pth.indexed_elem(curve_seg));
    
    for(int i = 0; i < 4; i++) {
        a.p[i] = ai[i];
        b.p[i] = bi[i];
    }
    cairo_restore(cr);
    
    std::vector<std::pair <double, double> > ts = Geom::FindIntersections(a, b);
    cairo_save(cr);
    cairo_set_source_rgba (cr, 0, 0.5, 0., 0.8);
    for(int i = 0; i < ts.size(); i++) {
        Geom::SubPath::Location pl(display_path.indexed_elem(curve_seg), ts[i].first);
        
        draw_handle(cr, display_path.point_at(pl));
        Geom::SubPath::Location p2(pth.indexed_elem(curve_seg), ts[i].second);
        
        draw_circ(cr, display_path.point_at(p2));
    }
    cairo_restore(cr);
    */
    
    if(0) {
        vector<Geom::SubPath::Location> pts = 
            find_vector_extreme_points(display_path, gradient_vector-Geom::Point(10,10));
        
        for(int i = 0; i < pts.size(); i++) {
            Geom::Point pos, tgt, acc;
            draw_handle(cr, display_path.point_at(pts[i]));
            //display_path.point_tangent_acc_at (pts[i], pos, tgt, acc);
            //draw_circ(cr, pos);
        }
    }
    
    if(0) { // probably busted
        cairo_stroke(cr);
  
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0., 0.25, 0.25, 0.8);
        assert(hash_cookie == display_path);
        vector<Geom::SubPath::Location> pts = 
            find_maximal_curvature_points(display_path);
        
        assert(hash_cookie == display_path);
        for(int i = 0; i < pts.size(); i++) {
            Geom::Point pos, tgt, acc;
            draw_circ(cr, display_path.point_at(pts[i]));
        }
        cairo_stroke(cr);
        cairo_restore(cr);
    }
    double area = 0;
    
    assert(hash_cookie == display_path);
    notify << "Area: " << area;
    
    Geom::Point cntr;
    assert(hash_cookie == display_path);
    centroid(display_path, cntr, area);
    draw_circ(cr, cntr);
    cairo_move_to(cr, cntr[0], cntr[1]);
    cairo_show_text (cr, "center of the path");
    notify << "pathwise Area: " << area << ", " << cntr;

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
    
    assert(hash_cookie == display_path);
    
    double x = widget->allocation.x + widget->allocation.width / 2;
    double y = widget->allocation.y + widget->allocation.height / 2;

    double radius = std::min (widget->allocation.width / 2,
                              widget->allocation.height / 2) - 5;

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
        std::vector<Geom::Point> handles = display_path.get_handles();
        for(int i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < 5) {
                selected_handle = &handles[i];
                old_handle_pos = mouse - handles[i];
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
    display_path = read_svgd(f).front();
    
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
    display_path = read_svgd(f).front();

    Geom::Rect r = display_path.bbox();
    
    display_path = display_path*Geom::translate(-r.min());
    Geom::scale sc(r.max() - r.min());
    display_path = display_path*(sc.inverse()*Geom::scale(500,500));
    original_curve = display_path;
    
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();
    GtkAccelGroup *accel_group = gtk_accel_group_new ();
 
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "text toy");

    GtkWidget *menubox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (menubox);
    gtk_container_add (GTK_CONTAINER (window), menubox);

    GtkWidget *menubar = gtk_menu_bar_new ();
    gtk_widget_show (menubar);
    gtk_box_pack_start (GTK_BOX (menubox), menubar, FALSE, FALSE, 0);

    GtkWidget *menuitem = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem);

    GtkWidget *menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    GtkWidget *open = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_widget_show (open);
    gtk_container_add (GTK_CONTAINER (menu), open);

    GtkWidget *separatormenuitem = gtk_separator_menu_item_new ();
    gtk_widget_show (separatormenuitem);
    gtk_container_add (GTK_CONTAINER (menu), separatormenuitem);
    gtk_widget_set_sensitive (separatormenuitem, FALSE);

    GtkWidget *quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
    gtk_widget_show (quit);
    gtk_container_add (GTK_CONTAINER (menu), quit);

    GtkWidget *menuitem2 = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (menuitem2);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem2);

    GtkWidget *menu2 = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

    GtkWidget *about = gtk_menu_item_new_with_mnemonic ("_About");
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
