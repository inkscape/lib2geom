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
#include "s-basis.h"
#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "interactive-bits.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path.h"
#include "path-cairo.h"
#include <iterator>
#include "multidim-sbasis.h"
#include "path-builder.h"
#include "geom.h"

using std::string;
using std::vector;

static GtkWidget *canvas;
std::ostringstream *note_p = 0;

BezOrd z0(0.5,1.);

std::vector<Geom::Point> handles;
Geom::Point *selected_handle;
Geom::Point old_handle_pos;
Geom::Point old_mouse_point;

unsigned total_pieces;

void draw_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    cairo_move_to(cr, point_at(B, 0));
    for(int ti = 1; ti <= 30; ti++) {
        double t = (double(ti))/(30);
        cairo_line_to(cr, point_at(B, t));
    }
}



void draw_cb(cairo_t *cr, multidim_sbasis<2> const &B) {
    std::vector<Geom::Point> bez = sbasis2_to_bezier(B, 2);
    cairo_move_to(cr, bez[0]);
    cairo_curve_to(cr, bez[1], bez[2], bez[3]);
}

void draw_elip() {
    Geom::Point c;
    Geom::Point h[1];
    line_twopoint_intersect(h[0], h[1], h[3], h[4], c);
    
    Geom::Point old;
    for(int i = 0; i <= 100; i++) {
        double t = i/100.0;
        
        Geom::Point n = (1-t)*h[0] + t*h[3];
        Geom::Point c1, c2;
        line_twopoint_intersect(2*c-n, n, h[0], h[2], c1);
        line_twopoint_intersect(2*c-n, n, h[4], h[2], c2);
        Geom::Point six;
        line_twopoint_intersect(c1, h[3], c2, h[1], six);
        old = six;
    }
}

double sinC(double t) { return t - sin(t);}
double cosC(double t) { return 1 - cos(t);}
double cos2(double t) { return cos(t)*cos(t);}
double tanC(double t) { return sinC(t) / cosC(t);}

const double w = 1./3;
const double cwp = cos(w*M_PI);
const double swp = sin(w*M_PI);
double phi(double t, double w) { return sin(w*t) - w*sin(t); }
double phih(double t, double w) { return sin(w*t) + w*sin(t); }
double b4(double t, double w) {return phi(t/2,w)*phih(t/2,w)/(swp*swp);}
double b3(double t, double w) {return cwp*phi(t,w)/(2*swp) - cwp*cwp*b4(t,w); }
double b2(double t, double w) {return 2*w*w*sin(t/2)*sin(t/2);}
double b1(double t, double w) {return b3(2*M_PI - t, w);}
double b0(double t, double w) {return b4(2*M_PI - t, w);}
static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    int width = 256;
    int height = 256;
    std::ostringstream notify;
    note_p = &notify;
    gdk_drawable_get_size(widget->window, &width, &height);
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    for(int i = 0; i < handles.size(); i++) {
        std::ostringstream notify;
        notify << i;
        draw_circ(cr, handles[i]);
        cairo_move_to(cr, handles[i]);
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        pango_cairo_show_layout(cr, layout);
    }
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    for(int i = 1; i < 4; i+=2) {
        cairo_move_to(cr, 0, i*height/4);
        cairo_line_to(cr, width, i*height/4);
        cairo_move_to(cr, i*width/4, 0);
        cairo_line_to(cr, i*width/4, height);
    }
    cairo_stroke(cr);
    
    std::vector<Geom::Point> e_h = handles;
    for(int i = 0; i < 5; i++) {
        Geom::Point p = e_h[i];
        
        if(i)
            cairo_line_to(cr, p);
        else 
            cairo_move_to(cr, p);
    }
    cairo_stroke(cr);
    for(int i = 1; i < 4; i++) {
        //e_h[i] = (e_h[i]-e_h[0])*3 + e_h[0];
    }
    
    multidim_sbasis<2> B;
    const double alpha = M_PI;
    SBasis C = cos(0, alpha, 10);
    SBasis S = sin(0, alpha, 10);
    SBasis Cw = cos(0, w*alpha, 10);
    SBasis Sw = sin(0, w*alpha, 10);
    SBasis one(BezOrd(1,1));
    SBasis X(BezOrd(0,alpha));
    
    typedef double (* F)(double,double);
    F basis[5] = {b4, b3, b2, b1, b0};
                       
    
    for(int ti = 0; ti <= 30; ti++) {
        double t = 2*M_PI*(double(ti))/(30);
        Geom::Point p(0,0);
        
        for(unsigned i  = 0; i < 5; i++)
            p += basis[i](t,w)*e_h[i];
        
        if(ti)
            cairo_line_to(cr, p);
        else 
            cairo_move_to(cr, p);
    }
    cairo_stroke(cr);
    for(int i = 0; i < 0; i++) {
        for(int ti = 0; ti <= 30; ti++) {
            double t = 2*M_PI*(double(ti))/(30);
            double t1 = (double(ti))/(30);
            Geom::Point p(width/4 + (width/2)*t1, 3*height/4 - (width/2)*basis[i](t,w));
            
            if(ti)
                cairo_line_to(cr, p);
            else 
                cairo_move_to(cr, p);
        }
    }
    /*
    for(unsigned dim  = 0; dim < 2; dim++) {
        B[dim] = BezOrd(0,0);
        for(unsigned i  = 0; i < 5; i++) {
            B[dim] += handles[i][dim]*basis[i];
        }
        }*/
    {
        //Geom::PathBuilder pb;
        //subpath_from_sbasis(pb, B, 1);
    //cairo_path(cr, pb.peek());
    //cairo_path_handles(cr, pb.peek());
    }
    
    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
    cairo_stroke(cr);
    
    
    {
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, 0, height-logical_extent.height);
        pango_cairo_show_layout(cr, layout);
    }
    cairo_destroy (cr);
    note_p = 0;
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
        for(unsigned i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < 5) {
                selected_handle = &handles[i];
                old_handle_pos = mouse - handles[i];
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
    } else if (event->keyval == 'q') {
        exit(0);
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

static void
on_open_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //TODO: show open dialog, get filename
    
    char const *const filename = "banana.svgd";

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }
    
    gtk_widget_queue_draw(canvas); // globals are probably evil
}

static void
on_about_activate(GtkMenuItem *menuitem, gpointer user_data) {
    
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

int main(int argc, char **argv) {
    /*for(int i = 0; i < 5; i++)
      handles.push_back(Geom::Point(uniform()*400, uniform()*400));*/
    
    double sc = 30;
    Geom::Point c(6*sc, 6*sc);
    handles.push_back(sc*Geom::Point(0,0)+c);
    handles.push_back(sc*Geom::Point(tan(w*M_PI)/w, 0)+c);
    handles.push_back(sc*Geom::Point(0, 1/(w*w))+c);
    handles.push_back(sc*Geom::Point(-tan(w*M_PI)/w, 0)+c);
    handles.push_back(sc*Geom::Point(0,0)+c);
    
    
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

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    //g_idle_add((GSourceFunc)idler, canvas);

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
