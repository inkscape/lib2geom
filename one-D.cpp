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

using std::string;
using std::vector;

static GtkWidget *canvas;


/* Takes two vectors and fills C with their convolution. */
template <typename T>
void convolve(std::vector<T> &A, std::vector<T> &B, std::vector<T> &C) {
    C.resize(A.size() + B.size(), BezOrd(0,0));
    fill(C.begin(), C.end(), 0);
    
    for(unsigned j = 0; j < B.size(); j++) {
        for(unsigned i = j; i < A.size()+j; i++) {
            C[i] += B[j]*A[i-j];
        }
    }
}

SBasis operator*(double k, SBasis const &a) {
    SBasis c;
    c.a.resize(a.size(), BezOrd(0,0));
    for(unsigned j = 0; j < a.size(); j++) {
        for(unsigned dim = 0; dim < 2; dim++)
            c[j][dim] += k*a[j][dim];
    }
    return c;
}

SBasis shift(SBasis const &a, int sh) {
    SBasis c = a;
    if(sh > 0) {
        c.a.insert(c.a.begin(), sh, BezOrd(0,0));
    } else {
        // truncate
    }
    return c;
}

SBasis shift(BezOrd const &a, int sh) {
    SBasis c;
    if(sh > 0) {
        c.a.insert(c.a.begin(), sh, BezOrd(0,0));
        c.a.push_back(a);
    } else {
        // truncate
    }
    return c;
}

SBasis multiply(SBasis const &a, SBasis const &b) {
    // c = {a0*b0 - shift(1, a.tri*b.tri), a1*b1 - shift(1, a.tri*b.tri)}
    
    // shift(1, a.tri*b.tri)
    SBasis c;
    c.a.resize(a.size() + b.size(), BezOrd(0,0));
    c.a[0] = BezOrd(0,0);
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            double tri = b[j].tri()*a[i-j].tri();
            for(unsigned dim = 0; dim < 2; dim++)
                c.a[i+1/*shift*/][dim] = -tri;
        }
    }
    for(unsigned j = 0; j < b.size(); j++) {
        for(unsigned i = j; i < a.size()+j; i++) {
            for(unsigned dim = 0; dim < 2; dim++)
                c[i][dim] += b[j][dim]*a[i-j][dim];
        }
    }
    return c;
}

SBasis integral(SBasis const &c) {
    SBasis a;
    a.a.resize(c.size() + 1, BezOrd(0,0));
    a.a[0] = BezOrd(0,0);
    
    for(unsigned k = 1; k < c.size() + 1; k++) {
        double ahat = -c[k-1].tri()/(2*k);
        a[k][0] = ahat;
        a[k][1] = ahat;
    }
    double atri = 0;
    for(int k = c.size()-1; k >= 0; k--) { // XXX: unsigned?
        atri = (c[k].hat() + (k+1)*atri)/(2*k+1);
        a[k][0] -= atri/2;
        a[k][1] += atri/2;
    }
    
    return a;
}

SBasis derivative(SBasis const &a) {
    SBasis c;
    c.a.resize(a.size(), BezOrd(0,0));
    
    for(unsigned k = 0; k < a.size(); k++) {
        double d = (2*k+1)*a[k].tri();
        for(unsigned dim = 0; dim < 2; dim++) {
            if(k+1 < a.size()) {
                if(dim)
                    c[k][dim] = d - (k+1)*a[k+1][dim];
                else
                    c[k][dim] = d + (k+1)*a[k+1][dim];
            }
        }
    }
    
    return c;
}

SBasis sqrt(SBasis const &a, int k) {
    SBasis c;
    
    c.a.push_back(BezOrd(sqrt(a[0][0]), sqrt(a[0][1])));
    SBasis r = a - multiply(c, c); // remainder
    
    for(unsigned i = 1; i <= k; i++) {
        BezOrd ci(r[i][0]/(2*c[0][0]), r[i][1]/(2*c[0][1]));
        SBasis cisi = shift(ci, i);
        r = r - multiply(shift((2*c + cisi), i), SBasis(ci));
        c = c + cisi;
    }
    
    return c;
}

// return a kth order approx to 1/a)
SBasis reciprocal(BezOrd const &a, int k) {
    SBasis res;
    double r_s0 = (a.tri()*a.tri())/(-a[0]*a[1]);
    double r_s0k = 1;
    for(int i = 0; i < k; i++) {
        res.a.push_back(BezOrd(r_s0k/a[0], r_s0k/a[1]));
        r_s0k *= r_s0;
    }
    return res;
}

SBasis divide(SBasis const &a, SBasis const &b, int k) {
    SBasis c;
    SBasis r = a; // remainder
    
    k++;
    r.a.resize(k, BezOrd(0,0));
    c.a.resize(k, BezOrd(0,0));

    for(unsigned i = 0; i < k; i++) {
        BezOrd ci(r[i][0]/b[0][0], r[i][1]/b[0][1]); //H0
        c[i] = c[i] + ci;
        r[i] = r[i] - ci;
        //r = r - multiply(ci,b);
    }
    
    return c;
}
BezOrd z0(0.5,1.);

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);

    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);
    
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    for(int i = 1; i < 4; i+=2) {
        cairo_move_to(cr, 0, i*height/4);
        cairo_line_to(cr, width, i*height/4);
        cairo_move_to(cr, i*width/4, 0);
        cairo_line_to(cr, i*width/4, height);
    }
    cairo_stroke(cr);
    SBasis one(BezOrd(1,1));
    assert(one.a.size() == 1);
    SBasis P0(z0), P1(BezOrd(3, 1));
    SBasis Q = P0;//multiply(P0, P1);
    {
        cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 0.5);
        cairo_set_line_width (cr, 3);

        for(int ti = 0; ti < width; ti++) {
            double t = (double(ti))/(width);
            double y = one.point_at(t)/(Q.point_at(t))/2;
            if(ti)
                cairo_line_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
            else
                cairo_move_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
        }    
        cairo_stroke(cr);
    }
    cairo_set_line_width (cr, 1);
    for(int order = 0; order < 10; order++) {
        cairo_set_source_rgba (cr, 0.5, 0, 0.5, 0.8);
        SBasis Z;
        Z.a.push_back(z0);
        SBasis C = divide(one, Q, order);//sqrt(z0, order);

        for(int ti = 0; ti < width; ti++) {
            double t = (double(ti))/(width);
            double y =  C.point_at(t)/2;
            if(ti)
                cairo_line_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
            else
                cairo_move_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
        }    
        cairo_stroke(cr);
    }

    for(int order = 0; order < 10; order++) {
        cairo_set_source_rgba (cr, 0.5, 0.5, 0, 0.8);
        SBasis Z;
        Z.a.push_back(z0);
        SBasis C = reciprocal(z0, order);

        for(int ti = 0; ti < width; ti++) {
            double t = (double(ti))/(width);
            double y =  C.point_at(t)/2;
            if(ti)
                cairo_line_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
            else
                cairo_move_to(cr, t*width/2 + width/4, 3*height/4 - y*height/2);
        }    
        cairo_stroke(cr);
    }

    cairo_move_to(cr, 250,250);
    

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
    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        handle_mouse(widget);
    }

    if(e->state & (GDK_BUTTON2_MASK)) {
        gtk_widget_queue_draw(widget);
    }
    
    return FALSE;
}

static gint mouse_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    if(e->button == 2) {
        gtk_widget_queue_draw(window);
    } else if(e->button == 1) {
        z0[1] /= 0.9;
    } else if(e->button == 3) {
        z0[1] *= 0.9;
    }
    gtk_widget_queue_draw(window);
    
    return FALSE;
}

static gint mouse_release_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
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
