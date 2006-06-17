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
Geom::SubPath display_path;

static Geom::Point old_handle_pos;
static Geom::Point old_mouse_point;

void draw_line_seg(GdkWindow* w, Geom::Point a, Geom::Point b) {
    gdk_draw_line(w, plain_gc, int(a[0]), int(a[1]), int(b[0]), int(b[1]));
}

void draw_spot(GdkWindow* w, Geom::Point h) {
    draw_line_seg(w, h, h);
}

void draw_handle(GdkWindow* w, Geom::Point h, string name = string("")) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    gdk_draw_line(w, plain_gc, x-3, y, x+3, y);
    gdk_draw_line(w, plain_gc, x, y-3, x, y+3);
    //templayout.set_text(name);
    //w.draw_layout(g, x, y, templayout);
}

void draw_circ(GdkWindow* w, Geom::Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    gdk_draw_arc(w, plain_gc, 0, x-3, y-3, 6, 6, 0, 360*64);
}

void draw_ray(GdkWindow* w, Geom::Point h, Geom::Point dir) {
    draw_line_seg(w, h, h+dir);
}

//line_intersection
/*static kind
segment_intersect(Geom::Point const &p00, Geom::Point const &p01,
				 Geom::Point const &p10, Geom::Point const &p11,
				 Geom::Point &result)
*/

void draw_elip(GdkWindow* w, Geom::Point *h) {
    draw_line_seg(w, h[0], h[1]);
    draw_line_seg(w, h[3], h[4]);
    draw_line_seg(w, h[3], h[2]);
    draw_line_seg(w, h[2], h[1]);

    Geom::Point c;
    line_twopoint_intersect(h[0], h[1], h[3], h[4], c);
    draw_handle(w, c);
    
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
            draw_line_seg(w, old, six);
        old = six;
    }
}

void draw_path(GdkWindow* w, Geom::SubPath p) {
    path_to_polyline pl(p, 1);
    
    Geom::Point old(pl.handles[0]);
    
    for(unsigned i = 1; i < pl.handles.size(); i++) {
        Geom::Point p(pl.handles[i]);
        draw_line_seg(w, old, p);
        old = p;
    }
    
}

Geom::Point* selected_handle = 0;


static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);

    for(int i = 0; i < display_path.handles.size(); i++) {
        draw_handle(widget->window, display_path.handles[i]);
    }
    draw_path(widget->window, display_path);
    //draw_elip(widget->window, handles);
    
    Geom::Point dir(1,1);
    /*
    vector<Geom::SubPath::Location> pts = find_vector_extreme_points(display_path, dir);
    
    for(int i = 0; i < pts.size(); i++) {
        draw_circ(widget->window, display_path.point_at(pts[i]));
        }*/
    
    double dist = INFINITY;

    Geom::SubPath::Location pl = 
        display_path.nearest_location(old_mouse_point, dist);
    {
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        draw_circ(widget->window, pos);
        double kurvature = dot(acc, rot90(tgt))/pow(Geom::L2(tgt),3);
        
        if(fabs(kurvature) > 0.001)
            draw_ray(widget->window, pos, 
                     (1./kurvature)*Geom::unit_vector(rot90(tgt)));
        else // just normal
            draw_ray(widget->window, pos, rot90(tgt));
            
    }
    Geom::SubPath pth = display_path.subpath(display_path.begin(), display_path.end());
    pth = pth*Geom::translate(Geom::Point(30, 30));
    draw_path(widget->window, pth);
    Bezier a, b;
    const int curve_seg = 3;
    Geom::SubPath::Elem ai(*display_path.indexed_elem(curve_seg)), bi(*pth.indexed_elem(curve_seg));
    
    for(int i = 0; i < 4; i++) {
        a.p[i] = ai[i];
        b.p[i] = bi[i];
    }
    
    std::vector<std::pair <double, double> > ts = Geom::FindIntersections(a, b);
    for(int i = 0; i < ts.size(); i++) {
        Geom::SubPath::Location pl(display_path.indexed_elem(curve_seg), ts[i].first);
        
        draw_handle(widget->window, display_path.point_at(pl));
        Geom::SubPath::Location p2(pth.indexed_elem(curve_seg), ts[i].second);
        
        draw_circ(widget->window, display_path.point_at(p2));
    }
    
    
    /*
    vector<Geom::SubPath::Location> pts = 
        find_inflection_points(display_path);
  
    for(int i = 0; i < pts.size(); i++) {
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pts[i], pos, tgt, acc);
        //tgt *= 0.1;
        //acc *= 0.1;
        draw_handle(widget->window, display_path.point_at(pts[i]));
        draw_circ(widget->window, pos);
        
        //draw_ray(widget->window, pos+tgt, acc);
    }
    */
    notify << "path length: " << arc_length_integrating(display_path, 1e3) << "\n";
    

    {
        notify << std::ends;
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, notify.str().c_str());
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        
        gdk_draw_layout(widget->window,
                        widget->style->fg_gc[GTK_STATE_NORMAL],
                        0, height-logical_extent.height, layout);
    }
    
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

#include "translate-ops.h"
#include "scale-ops.h"
#include "translate-scale-ops.h"


int main(int argc, char **argv) {
    char const *const filename = (argc >= 2
                                  ? argv[1]
                                  : "toy.svgd");
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return 1;
    }
    display_path = read_svgd(f);

    Geom::Rect r = display_path.bbox();
    
    display_path = display_path*Geom::translate(-r.min());
    Geom::scale sc(r.max() - r.min());
    display_path = display_path*(sc.inverse()*Geom::scale(500,500));
    
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
