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
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>
     

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

void draw_sb2d(cairo_t* cr, SBasis2d const &sb2, Geom::Point dir, double width) {
    multidim_sbasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = dir[0]*extract_u(sb2, u) + BezOrd(u);
        B[1] = SBasis(BezOrd(0,1))+dir[1]*extract_u(sb2, u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        draw_cb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = dir[1]*extract_v(sb2, v) + BezOrd(v);
        B[0] = SBasis(BezOrd(0,1)) + dir[0]*extract_v(sb2, v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        draw_cb(cr, B);
    }
}

class curve_min{
public:
    multidim_sbasis<2> &B;
    multidim_sbasis<2> out;
    SBasis2d& sb2;
    unsigned n;
    unsigned par;
    curve_min(multidim_sbasis<2> &B,
              SBasis2d& sb2) :B(B), sb2(sb2) {}
};

double
my_f (const gsl_vector *v, void *params)
{
    curve_min &p = *(curve_min*)params;
    for(int dim = 0; dim < 2; dim++) {
        p.out[dim] = p.B[dim];
        double s = 1;
        for(int i = 0; i < p.n; i++) {
            p.out[dim] += shift(BezOrd(gsl_vector_get(v, 2*dim + 2*i)*s,
                                       gsl_vector_get(v, 2*dim + 2*i + 1)*s), i+1);
            s *= 0.25;
        }
    }
    SBasis l = compose(p.sb2, p.out);
    l = integral(multiply(l, l));
    return l[0][1] - l[0][0];
}

double fn1 (double x, void * params)
{
    curve_min &p = *(curve_min*)params;
    if((p.par &1) == 0)
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(x,0), p.par/2 + 1);
        }
    else 
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(0,x), p.par/2 + 1);
        }
    SBasis l = compose(p.sb2, p.out);
    l = integral(multiply(l, l));
    return l[0][1] - l[0][0];
}

double fn2 (double x, void * params)
{
    curve_min &p = *(curve_min*)params;
    if((p.par &1) == 0)
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(x), p.par/2 + 1);
        }
    else 
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(Hat(0), Tri(x)), p.par/2 + 1);
        }
    SBasis l = compose(p.sb2, p.out);
    l = integral(multiply(l, l));
    return l[0][1] - l[0][0];
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
    SBasis2d sb2;
    sb2.us = 2;
    sb2.vs = 2;
    const int depth = sb2.us*sb2.vs;
    const int surface_handles = 4*depth;
    sb2.resize(depth, BezOrd2d(0));
    vector<Geom::Point> display_handles(surface_handles);
    Geom::Point dir(1,-2);
    if(handles.empty()) {
        for(int vi = 0; vi < sb2.vs; vi++)
        for(int ui = 0; ui < sb2.us; ui++)
        for(int iv = 0; iv < 2; iv++)
            for(int iu = 0; iu < 2; iu++) {
                Geom::Point p((2*(iu+ui)/(2.*ui+1)+1),
                              (2*(iv+vi)/(2.*vi+1)+1));
                if(ui == 0 && vi == 0) {
                    if(iu == 0 && iv == 0)
                        p[1] -= 0.1;
                    if(iu == 1 && iv == 1)
                        p[1] += 0.1;
                    //if(vi == sb2.vs - 1)
                    //    p[1] += 0.1;
                }
                handles.push_back((width/4.)*p);
            }
        
        handles.push_back(Geom::Point(3*width/4.,
                                      width/4.) + 30*dir);
        for(int i = 0; i < 2; i++)
            handles.push_back(Geom::Point(3*width/8.,
                                          (1+6*i)*width/8.));

    
    }
    dir = (handles[surface_handles] - Geom::Point(3*width/4., width/4.)) / 30;
    cairo_move_to(cr, 3*width/4., width/4.);
    cairo_line_to(cr, handles[surface_handles]);
    for(int vi = 0; vi < sb2.vs; vi++)
        for(int ui = 0; ui < sb2.us; ui++)
    for(int iv = 0; iv < 2; iv++)
        for(int iu = 0; iu < 2; iu++) {
            unsigned corner = iu + 2*iv;
            unsigned i = ui + vi*sb2.us;
            Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                             (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
            double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
            display_handles[corner+4*i] = dl*dir + base;
            sb2[i][corner] = dl*10/(width/2)*pow(4,ui+vi);
        }
    draw_sb2d(cr, sb2, dir*0.1, width);
    cairo_set_source_rgba (cr, 0., 0., 0, 0.7);
    cairo_stroke(cr);
/*
    for(int ui = 0; ui <= 1; ui++) {
        SBasis sb = extract_u(sb2, ui);
        vector<double> r = roots(sb);
        std::cout << "sbasis sub (%d, 0): ";
        std::copy(r.begin(), r.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
        
    }
    for(int vi = 0; vi <= 1; vi++) {
        SBasis sb = extract_v(sb2, vi);
        vector<double> r = roots(sb);
        std::cout << "sbasis sub (0, %d): ";
        std::copy(r.begin(), r.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
    }
*/
    multidim_sbasis<2> B = bezier_to_sbasis<2, 1>(handles.begin() + surface_handles+1);
    B += Geom::Point(-width/4., -width/4.);
    B *= (2./width);

    for(int dim = 0; dim < 2; dim++) {
        B[dim] += shift(BezOrd(0.1), 1);
    }
#if 1
// *** Minimiser
    curve_min cm(B, sb2);
    for(cm.par = 0; cm.par < 8; cm.par++) {
    int status;
    int iter = 0, max_iter = 100;
    const gsl_min_fminimizer_type *T;
    gsl_min_fminimizer *s;
    double m = 0.0;
    double a = -100.0, b = 100.0;
    gsl_function F;
     
    F.function = &fn1;
    F.params = &cm;
     
    T = gsl_min_fminimizer_brent;
    s = gsl_min_fminimizer_alloc (T);
    gsl_min_fminimizer_set (s, &F, m, a, b);
    
    do
    {
        iter++;
        status = gsl_min_fminimizer_iterate (s);
     
        m = gsl_min_fminimizer_x_minimum (s);
        a = gsl_min_fminimizer_x_lower (s);
        b = gsl_min_fminimizer_x_upper (s);
     
        status 
            = gsl_min_test_interval (a, b, 0.001, 0.0);
     
    }
    while (status == GSL_CONTINUE && iter < max_iter);
    cm.B = cm.out;
    }
    
#else
    const gsl_multimin_fminimizer_type *T =
        gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer *s = NULL;
    gsl_vector *ss, *x;
    gsl_multimin_function minex_func;
    
    size_t iter = 0, i;
    int status;
    double size;
    curve_min cm(B, sb2);
     
    /* Initial vertex size vector */
    cm.n = 1;
    size_t np = cm.n*4;
    ss = gsl_vector_alloc (np);
    gsl_vector_set_all (ss, 0.1);
     
    /* Starting point */
    x = gsl_vector_alloc (np);
    gsl_vector_set_all (x, 0.0);
     
    /* Initialize method and iterate */
    minex_func.f = &my_f;
    minex_func.n = np;
    minex_func.params = (void *)&cm;
     
    s = gsl_multimin_fminimizer_alloc (T, np);
    gsl_multimin_fminimizer_set (s, &minex_func, x, ss);
     
    do
    {
        iter++;
        status = gsl_multimin_fminimizer_iterate(s);
     
        if (status)
            break;
     
        size = gsl_multimin_fminimizer_size (s);
        status = gsl_multimin_test_size (size, 1e-2);
        /*
        if (status == GSL_SUCCESS)
        {
            printf ("converged to minimum at\n");
        }
     
        printf ("%5d ", iter);
        for (i = 0; i < np; i++)
        {
            printf ("%10.3e ", gsl_vector_get (s->x, i));
        }
        printf ("f() = %7.3f size = %.3f\n", s->fval, size);*/
    }
    while (status == GSL_CONTINUE && iter < 100);
     
    gsl_vector_free(x);
    gsl_vector_free(ss);
    gsl_multimin_fminimizer_free (s);
     
#endif
    B = cm.out;
     
    draw_cb(cr, (width/2)*B + Geom::Point(width/4., width/4.));
    SBasis l = compose(sb2, B);
    notify << "l = " << l << std::endl ;
    l = integral(multiply(l, l));
    notify << "cost = " << l[0][1] - l[0][0] ;
    
    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
    cairo_stroke(cr);
    
    for(int i = 0; i < display_handles.size(); i++) {
        draw_circ(cr, display_handles[i]);
    }
    
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
