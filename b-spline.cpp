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
using std::string;
using std::vector;

static GtkWidget *canvas;

static double alpha = 1;

vector<Geom::Point> handles;
vector<float> knots;

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

void draw_rat_bez(cairo_t *cr, Geom::Point* h, double w) {
    printf("%g\n", w);
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

int find_span(int p, double u) {
    const int n = handles.size();
    if(u >= knots.back())
        return n;
    int lo = p;
    int hi = n+1;
    int mid = (lo + hi)/2;
    while((lo < hi) && ((u < knots[mid]) || (u >= knots[mid+1]))) {
        if(u < knots[mid])
            hi = mid;
        else
            lo = mid;
        mid = (lo + hi)/2;
    }
    return mid;
}

void basis_fns(int i, double u, int p, double *N) {
    const int n = handles.size();
    N[0] = 1;
    static vector<double> left, right;
    left.resize(p+1);
    right.resize(p+1);
    for(int j = 1; j <= p; j++) {
        left[j] = u-knots[i+1-j];
        right[j] = knots[i+j]-u;
        double saved = 0.0;
        for(int r = 0; r < j; r++) {
            double temp = N[r]/(right[r+1]+left[j-r]);
            N[r] = saved + right[r+1]*temp;
            saved = left[j-r]*temp;
        }
        N[j] = saved;
    }
}

Geom::Point
curve_point(double u) {
    const int n = handles.size();
    const int p = 3;
    int span = find_span(p, u);
    vector<double> N;
    N.resize(p+1);
    basis_fns(span, u, p, &N[0]);
    Geom::Point C;
    for(int i = 0; i <= p; i++)
        C += N[i]*handles[span-p+i];
    //std::cout << C << " ";
    return C;
}

void draw_bspline(cairo_t *cr) {
    for(int i = 0; i < handles.size()-1; i++)
        draw_line_seg(cr, handles[i], handles[i+1]);
    
    Geom::Point C = curve_point(0);
    cairo_move_to(cr, C);
    for(int i = 0; i < 100; i++) {
        double u = double(i+1)/100;
        Geom::Point C = curve_point(u);
        cairo_line_to(cr, C);
        
    }
    cairo_stroke(cr);
}    

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

    for(int i = 0; i < handles.size(); i++) {
        draw_handle(cr, handles[i]);
    }
    draw_bspline(cr);
    
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
    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer) {
    gint ret = FALSE;
    if (event->keyval == ' ') {
        ret = TRUE;
    } else if (event->keyval == 'l') {
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

unsigned small_rand() {
    return (rand() & 0xff) + 1;
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

int main(int argc, char **argv) {
    for(int i = 0; i < 8; i++) {
        handles.push_back(Geom::Point(small_rand(), small_rand()));
        knots.push_back(uniform());
    }
    knots[0] = 0;
    /*knots[1] = 0;
    knots[2] = 0;
    knots[3] = 1;
    knots[4] = 1;
    knots[5] = 1;*/
    knots.push_back(1.0);
    
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
