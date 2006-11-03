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
#include "interactive-bits.h"
using std::string;
using std::vector;

static GtkWidget *canvas;

static double alpha = 1;

void draw_rat_bez(cairo_t *cr, Geom::Point* h, double w) {
    cairo_move_to(cr, h[0]);
    for(int i = 1; i <= 100; i++) {
        double t = i*0.01;
        
        Geom::Point p = (1-t)*(1-t)*h[0] + 
            2*(1-t)*t*w*h[1] + 
            t*t*h[2];
        double pw = (1-t)*(1-t) + 
            2*(1-t)*t*w + 
            t*t;
        cairo_line_to(cr, p/pw);
    }
    cairo_stroke(cr);
}

void draw_elip(cairo_t *cr, Geom::Point *h) {
    for(int i = 0; i < 4; i++)
        draw_line_seg(cr, h[i], h[i+1]);
    Geom::Point h2[3] = {h[0], h[1], (h[1] + h[2])/2};
    Geom::Point A = unit_vector(h2[0]-h2[1]);
    Geom::Point B = unit_vector(h2[2]-h2[1]);
    draw_rat_bez(cr, h2, dot(A, B));
    h2[0] = h2[2];
    h2[1] = h[2];
    h2[2] = (h[2] + h[3])/2;
    A = unit_vector(h2[0]-h2[1]);
    B = unit_vector(h2[2]-h2[1]);
    draw_rat_bez(cr, h2, dot(A, B));
    h2[0] = h2[2];
    h2[1] = h[3];
    h2[2] = h[4];
    A = unit_vector(h2[0]-h2[1]);
    B = unit_vector(h2[2]-h2[1]);
    draw_rat_bez(cr, h2, dot(A, B));
}    

const double sqrt3 = sqrt(3);

Geom::Point handles[] = {Geom::Point(220, 20+sqrt3*200), 
                         Geom::Point(20, 20+sqrt3*200), 
			 Geom::Point(220, 20), 
                         Geom::Point(420, 20+sqrt3*200), 
                         Geom::Point(220, 20+sqrt3*200)};
const int handles_n = 5;

Geom::Point* selected_handle = 0;


static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);
    cairo_t *cr = gdk_cairo_create (widget->window);
    cairo_set_line_width (cr, 1);

    for(int i = 0; i < handles_n; i++) {
        draw_handle(cr, handles[i]);
    }
    draw_elip(cr, handles);
    
    notify << "Alpha: " << alpha;

    {
        notify << std::ends;
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, notify.str().c_str());
        gdk_draw_layout(widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, height-20, layout);
    }
    
    cairo_destroy (cr);
    return TRUE;
}

static Geom::Point old_handle_pos;
static Geom::Point old_mouse_point;

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
        for(int i = 0; i < handles_n; i++) {
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
