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
#include "translate.h"
#include "translate-ops.h"
#include "s-basis-2d.h"
#include "path-builder.h"

using std::string;
using std::vector;

static GtkWidget *canvas;
std::ostringstream *note_p = 0;

std::vector<Geom::Point> handles;
Geom::Point *selected_handle;
Geom::Point old_handle_pos;
Geom::Point old_mouse_point;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

double uniform() {
    return double(rand()) / RAND_MAX;
}

void draw_cb(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::PathBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_path(cr, pb.peek());
}

void draw_sb2d(cairo_t* cr, vector<SBasis2d> const &sb2, Geom::Point dir, double width) {
    multidim_sbasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);// + BezOrd(u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        draw_cb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);// + BezOrd(v);
        B[0] = extract_v(sb2[0], v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        draw_cb(cr, B);
    }
}

SBasis compose(BezOrd2d const &a, multidim_sbasis<2> p) {
    SBasis sb;
    multidim_sbasis<2> omp;
    for(int dim = 0; dim < 2; dim++)
        omp[dim] = BezOrd(1) - p[dim];
    //std::cout << "a = " << a << std::endl;
    //for(int dim = 0; dim < 2; dim++)
    //    std::cout << p[dim] << ", " << omp[dim] << std::endl;
    sb = a[0]*multiply(omp[0], omp[1]) +
        a[1]*multiply(p[0], omp[1]) +
        a[2]*multiply(omp[0], p[1]) +
        a[3]*multiply(p[0], p[1]);
    //std::cout << sb << std::endl;
    return sb;
}

SBasis 
compose(SBasis2d const &fg, multidim_sbasis<2> p) {
    SBasis B;
    SBasis s[2];
    SBasis ss[2];
    for(int dim = 0; dim < 2; dim++) 
        s[dim] = multiply(p[dim], (BezOrd(1) - p[dim]));
    B = compose(fg[0], p);
    ss[0] = BezOrd(1);
    for(int vi = 0; vi < fg.vs; vi++) {
        ss[1] = ss[0];
        for(int ui = 0; ui < fg.us; ui++) {
            unsigned i = ui + vi*fg.us;
            if(vi || ui)
                B += multiply(ss[1], compose(fg[i], p));
            ss[1] = multiply(ss[1], s[1]);
        }
        ss[0] = multiply(ss[0], s[0]);
    }
    return B;
}


multidim_sbasis<2> 
compose(vector<SBasis2d> const &fg, multidim_sbasis<2> p) {
    multidim_sbasis<2> B;
    for(int dim = 0; dim < 2; dim++) {
        B[dim] = compose(fg[dim], p);
    }
    return B;
}

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
    for(int i = 1; i < 4; i+=2) {
        cairo_move_to(cr, 0, i*width/4);
        cairo_line_to(cr, width, i*width/4);
        cairo_move_to(cr, i*width/4, 0);
        cairo_line_to(cr, i*width/4, width);
    }
    vector<SBasis2d> sb2(2);
    for(int dim = 0; dim < 2; dim++) {
        sb2[dim].us = 2;
        sb2[dim].vs = 2;
        const int depth = sb2[dim].us*sb2[dim].vs;
        const int surface_handles = 4*depth;
        sb2[dim].resize(depth, BezOrd2d(0));
    }
    const int depth = sb2[0].us*sb2[0].vs;
    const int surface_handles = 4*depth;
    Geom::Point dir(1,-2);
    if(handles.empty()) {
        for(int vi = 0; vi < sb2[0].vs; vi++)
            for(int ui = 0; ui < sb2[0].us; ui++)
                for(int iv = 0; iv < 2; iv++)
                    for(int iu = 0; iu < 2; iu++)
                        handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                      (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        
        for(int i = 0; i < 4; i++)
            handles.push_back(Geom::Point(uniform()*width/4.,
                                          uniform()*width/4.));
    
    }
    
    for(int dim = 0; dim < 2; dim++) {
        Geom::Point dir(0,0);
        dir[dim] = 1;
        for(int vi = 0; vi < sb2[dim].vs; vi++)
            for(int ui = 0; ui < sb2[dim].us; ui++)
                for(int iv = 0; iv < 2; iv++)
                    for(int iu = 0; iu < 2; iu++) {
                        unsigned corner = iu + 2*iv;
                        unsigned i = ui + vi*sb2[dim].us;
                        Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                         (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
                        if(vi == 0 && ui == 0) {
                            base = Geom::Point(width/4., width/4.);
                        }
                        double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
                        sb2[dim][i][corner] = dl/(width/2)*pow(4,ui+vi);
                    }
    }
    draw_sb2d(cr, sb2, dir*0.1, width);
    multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin() + surface_handles);
    draw_cb(cr, B);
    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
    cairo_stroke(cr);
    B *= (4./width);
    multidim_sbasis<2> tB = compose(sb2, B);
    B = (width/2)*B + Geom::Point(width/4, width/4);
    //draw_cb(cr, B);
    tB = (width/2)*tB + Geom::Point(width/4, width/4);
    
    draw_cb(cr, tB);
    
    //notify << "bo = " << sb2.index(0,0); 
    
    cairo_set_source_rgba (cr, 0.5, 0.25, 0, 1);
    cairo_stroke(cr);
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
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
        for(int i = 0; i < handles.size(); i++) {
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
    Geom::Point mouse(e->x, e->y);
    old_handle_pos = mouse;
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

int main(int argc, char **argv) {
    
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

    gtk_window_set_title(GTK_WINDOW(window), "scroll layout");

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

    GtkWidget* pain = gtk_vpaned_new();
    gtk_widget_show (pain);
    gtk_box_pack_start(GTK_BOX(menubox), pain, TRUE, TRUE, 0);
    gtk_paned_add1(GTK_PANED(pain), canvas);

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
