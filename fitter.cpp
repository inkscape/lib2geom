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
#include "geom.h"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using namespace Geom;

static GtkWidget *canvas;


BezOrd z0(0.5,1.);

std::vector<Geom::Point> handles;
Geom::Point *selected_handle;
Geom::Point old_handle_pos;
Geom::Point old_mouse_point;


class sufficient_stats{
public:
    double Sx, Sy, Sxx, Syy, Sxy;
    
    void
    include(const double x, const double y) {
        Sx += x;
        Sy += y;
        Sxx += x*x;
        Syy += y*y;
        Sxy += x*y;
    }
};

sufficient_stats
operator-(sufficient_stats const & a, sufficient_stats const &b) {
    sufficient_stats ss;
    ss.Sx = a.Sx - b.Sx;
    ss.Sy = a.Sy - b.Sy;
    ss.Sxx = a.Sxx - b.Sxx;
    ss.Sxy = a.Sxy - b.Sxy;
    ss.Syy = a.Syy - b.Syy;
    return ss;
}

class fit{
public:
    vector<Point> input;
    vector<Point> solution;
    
    void
    draw(cairo_t* cr) {
        assert(solution.size() > 1); {
            cairo_move_to(cr, solution[0]);
            for(int i = 1; i < solution.size(); i++) {
                cairo_line_to(cr, solution[i]);
            }
        }
    }
    
    void endpoints() {
        solution.push_back(input[0]);
        solution.push_back(input.back());
    }
    
    void arbitrary();
    void linear_reg();
    
    // sufficient stats from start to each point
    vector<sufficient_stats> ac_ss;
    
    /*** Compute the least squares error for a line between two points on the line. */
    double measure(unsigned from, unsigned to, double & mean) {
        sufficient_stats ss = ac_ss[to+1] - ac_ss[from];
        
        Point n = unit_vector(rot90(input[to] - input[from]));
        double dist = dot(n, input[from]); // distance from origin
        
        mean = n[0]*ss.Sx + n[1]*ss.Sy;
        double N = (to + 1 - from);
        return n[0]*n[0]*ss.Sxx
            + n[1]*n[1]*ss.Syy
            + 2*n[0]*n[1]*ss.Sxy
            - 2*dist*mean + N*dist*dist;
    }
    
    /*** Compute the best line for the points, given normal. */
    double best_angled_line(unsigned from, unsigned to,
                            Point n,
                            double & mean) {
        sufficient_stats ss = ac_ss[to+1] - ac_ss[from];
        double N = (to + 1 - from);
        
        mean = (n[0]*ss.Sx + n[1]*ss.Sy);
        double dist = mean/N;
        return n[0]*n[0]*ss.Sxx
            + n[1]*n[1]*ss.Syy
            + 2*n[0]*n[1]*ss.Sxy
            - 2*dist*mean + N*dist*dist;
    }
    
    double simple_measure(unsigned from, unsigned to, double & mean) {
        Point n = unit_vector(rot90(input[to] - input[from]));
        double dist = dot(n, input[from]); // distance from origin
        double error = 0;
        mean = 0;
        for(int l = from; l <= to; l++) {
            double d = dot(input[l], n) - dist;
            mean += dot(input[l], n);
            error += d*d;
        }
        return error;
    }

    void simple_dp();
    void C_simple_dp();

    void C_endpoint(double & berr);
    
    fit(vector<Point> const & in)
        :input(in) {
        sufficient_stats ss;
        ss.Sx = ss.Sy = ss.Sxx = ss.Sxy = ss.Syy = 0;
        ac_ss.push_back(ss);
        for(unsigned l = 0; l < input.size(); l++) {
            ss.include(input[l][0], input[l][1]);
            ac_ss.push_back(ss);
        }
    }
    
};


void fit::arbitrary() {
    /*solution.resize(input.size());
      copy(input.begin(), input.end(), solution.begin());*/
    // normals
    Geom::Point angles[] = {Point(0,1), Point(1,0), Point(1,1), Point(1,-1)};
    int N = sizeof(angles)/sizeof(Point);
    
    double best_error = INFINITY;
    double best_mean = 0;
    int best_angle = 0;
    for(int i = 0; i < N; i++) {
        Point angle = angles[i];
        double mean = 0;
        double error = 0;
        for(int l = 0; l < input.size(); l++) {
            mean += dot(input[i], angle);
        }
        mean /= input.size();
        for(int l = 0; l < input.size(); l++) {
            double d = dot(input[i], angle) - mean;
            error += d*d;
        }
        if(error < best_error) {
            best_mean = mean;
            best_error = error;
            best_angle = i;
        }
    }
    Point angle = angles[best_angle];
    solution.push_back(angle*best_mean + dot(input[0], rot90(angle))*rot90(angle));
    solution.push_back(angle*best_mean + dot(input.back(), rot90(angle))*rot90(angle));
}

class reg_line{
public:
    Point parallel, centre, normal;
    double Sr, Srr;
    unsigned n;
};

template<class T>
reg_line
line_best_fit(T b, T e) {
    double Sx = 0,
        Sy = 0, 
        Sxx = 0, 
        Syy = 0, 
        Sxy = 0;
    unsigned n = e - b;
    reg_line rl;
    rl.n = n;
    for(T p = b; p != e; p++) {
        Sx += (*p)[0];
        Sy += (*p)[1];
        Sxx += (*p)[0]*(*p)[0];
        Syy += (*p)[1]*(*p)[1];
        Sxy += (*p)[0]*(*p)[1];
    }
    
    rl.parallel = unit_vector(Point(n*Sxx - Sx*Sx,
                 n*Sxy - Sx*Sy));
    rl.normal = rot90(rl.parallel);
    rl.centre = Point(Sx/n, Sy/n);
    rl.Sr = rl.Srr = 0;
    for(T p = b; p != e; p++) {
        double r = dot(rl.parallel, (*p) - rl.centre);
        rl.Sr += fabs(r);
        rl.Srr += r*r;
    }
    return rl;
}

void fit::linear_reg() {
    reg_line rl = line_best_fit(input.begin(),
                  input.end());
    solution.push_back(rl.centre + dot(rl.parallel, input[0] - rl.centre)*rl.parallel);
    solution.push_back(rl.centre + dot(rl.parallel, input.back() - rl.centre)*rl.parallel);
}

void fit::simple_dp() {
    const unsigned N = input.size();
    vector<unsigned> prev(N);
    vector<double> penalty(N);
    const double bend_pen = 100;
    
    for(unsigned i = 1; i < input.size(); i++) {
        double mean;
        double best = measure(0, i, mean);
        unsigned best_prev = 0;
        for(unsigned j = 1; j < i; j++) {
            double err = penalty[j] + bend_pen + measure(j, i, mean);
            if(err < best) {
                best = err;
                best_prev = j;
            }
        }
        penalty[i] = best;
        prev[i] = best_prev;
    }
    
    unsigned i = prev.size()-1;
    while(i > 0) {
        solution.push_back(input[i]);
        i = prev[i];
    }
    solution.push_back(input[i]);
    reverse(solution.begin(), solution.end());
}

void fit::C_endpoint(double & berr) {
    Geom::Point angles[] = {Point(0,1), Point(1,0), Point(1,1), Point(1,-1)};
    int C = sizeof(angles)/sizeof(Point);
    for(unsigned c = 0; c < C; c++) {
        angles[c] = unit_vector(angles[c]);
    }
    const unsigned N = input.size();
    
    double best_mean;
    double best = best_angled_line(0, N-1, angles[0], best_mean);
    unsigned best_dir = 0;
    for(unsigned c = 1; c < C; c++) {
        double m;
        double err = best_angled_line(0, N-1, angles[c], m);
        if(err < best) {
            best = err;
            best_mean = m;
            best_dir = c;
        }

    }
    berr = best;
    Point dir = angles[best_dir];
    Point dirT = rot90(dir);
    Point centre = dir*best_mean/N;
    
    solution.push_back(centre + dot(dirT, input[0] - centre)*dirT);
    solution.push_back(centre + dot(dirT, input.back() - centre)*dirT);
}

void fit::C_simple_dp() {
    Geom::Point angles[] = {Point(0,1), Point(1,0), Point(1,1), Point(1,-1)};
    int C = sizeof(angles)/sizeof(Point);
    for(unsigned c = 0; c < C; c++) {
        angles[c] = unit_vector(angles[c]);
    }
    const unsigned N = input.size();
    
    vector<int> prev(N);
    vector<double> penalty(N);
    vector<unsigned> dir(N);
    vector<double> mean(N);
    const double bend_pen = 0;
    
    for(unsigned i = 1; i < input.size(); i++) {
        double best_mean;
        double best = best_angled_line(0, i, angles[0], best_mean);
        unsigned best_prev = 0;
        unsigned best_dir = 0;
        for(unsigned c = 1; c < C; c++) {
            double m;
            double err = best_angled_line(0, i, angles[c], m);
            if(err < best) {
                best = err;
                best_mean = m;
                best_dir = c;
                best_prev = 0;
            }

        }
        for(unsigned j = 1; j < i; j++) {
            for(unsigned c = 0; c < C; c++) {
                double m;
                if(c == dir[j])
                    continue;
                double err = penalty[j] + bend_pen + 
                    best_angled_line(j, i, angles[c], m);
                if(err < best) {
                    best = err;
                    best_mean = m;
                    best_dir = c;
                    best_prev = j;
                }

            }
        }
        penalty[i] = best;
        prev[i] = best_prev;
        dir[i] = best_dir;
        mean[i] = best_mean;
    }
    
    prev[0] = -1;
    unsigned i = prev.size()-1;
    unsigned pi = i;
    while(i > 0) {
        Point bdir = angles[dir[i]];
        Point bdirT = rot90(bdir);
        Point centre = bdir*mean[i]/N;
        solution.push_back(centre + dot(bdirT, input[i] - centre)*bdirT);
        solution.push_back(centre + dot(bdirT, input[pi] - centre)*bdirT);
        pi = i;
        i = prev[i];
    }
    /*Point a = angles[dir[i]];
    Point aT = rot90(a);
    solution.push_back(a*mean[i] + 
    dot(input[i], aT)*aT);*/
    reverse(solution.begin(), solution.end());
}


static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    for(int i = 0; i < handles.size(); i++) {
        draw_circ(cr, handles[i]);
    }
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 1);
    
    fit f(handles);
    double berr;
    f.simple_dp();
    f.draw(cr);
    cairo_stroke(cr);
    notify << f.solution.size();
    //notify << "arc length = " << arc.point_at(1) - arc.point_at(0) << std::endl;
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
    Geom::Point start_point(uniform()*100, uniform()*100);
    double step = 50;
    for(int i = 0; i < 10; i++) {
        handles.push_back(start_point);
        start_point += Geom::Point(uniform()*step, 0.5*uniform()*step);
    }
/*
    for(int i = 0; i < 10; i++) {
        handles.push_back(start_point);
        start_point += Geom::Point(0.5*uniform()*step, uniform()*step);
        }*/
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
