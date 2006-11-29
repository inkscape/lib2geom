#include "maybe.h"
#include "path.h"
#include "path-to-polyline.h"
#include "read-svgd.h"
#include "path-find-points-of-interest.h"
#include "rotate.h"
#include "rotate-ops.h"
#include "matrix.h"
#include "transformation-ops.h"
#include "arc-length.h"
#include "path-intersect.h"
#include "path-ops.h"
#include "path-cairo.h"
#include "path-nearest.h"
#include "convex-cover.h"
#include "path-metric.h"
#include "translate-ops.h"
#include "scale-ops.h"
#include "centroid.h"

#include "toy-framework.h"

class ToyCairo: public Toy {

Geom::Path display_path;
Geom::Path original_curve;

Geom::Path::Location param(Geom::Path const & p, double t) {
    double T = t*(p.size()-1);
    //std::cout << T << ", " <<  T-int(T) << std::endl;
    return Geom::Path::Location(p.indexed_elem(int(T)), T - int(T));
}

void draw_convex_hull(cairo_t *cr, Geom::ConvexHull const & ch) {
    cairo_move_to(cr, ch.boundary.back());
    for(int i = 0; i < ch.boundary.size(); i++) {
        cairo_line_to(cr, ch.boundary[i]);
    }
    cairo_stroke(cr);
}

void draw_convex_cover(cairo_t *cr, Geom::Path const & p) {
    Geom::ConvexCover cc(p);
    
    for(int i = 0; i < cc.cc.size(); i++) {
        draw_convex_hull(cr, cc.cc[i]);
    }
}

void draw_evolute(cairo_t *cr, Geom::Path const & p) {
    int i = 0;
    for(double t = 0; t <= 1.0; t+= 1./1024) {
        Geom::Path::Location pl = param(p, t);
        
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

void draw_involute(cairo_t *cr, Geom::Path const & p) {
#ifdef HAVE_GSL
    int i = 0;
    double sl = arc_length_integrating(p, 1e-3);
    for(double s = 0; s < sl; s+= 5.0) {
        Geom::Path::Location pl = 
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
#endif
}

void draw_stroke(cairo_t *cr, Geom::Path const & p) {
#ifdef HAVE_GSL
    int i = 0;
    for(double t = 0; t <= 1.0; t+= 1./1024) {
        Geom::Path::Location pl = param(p, t);
        
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        Geom::Point pt = pos + 10*rot90(unit_vector(tgt));
        if(i)
            cairo_line_to(cr, pt);
        else 
            cairo_move_to(cr, pt);
        i++;
    }
#endif
}

Geom::Point gradient_vector;

bool rotater, evolution, half_stroking, involution, equal_arc;

void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 1);
    std::vector<Geom::Point> const & hand(display_path.get_handles());
    for(int i = 0; i < hand.size(); i++) draw_handle(cr, hand[i]);
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
    draw_convex_cover(cr, display_path);

#ifdef HAVE_GSL
    if(equal_arc) {
        cairo_save(cr);
        cairo_set_source_rgba (cr, 0.5, 0, 0.5, 0.8);
        double sl = arc_length_integrating(display_path, 1e-3);
        for(double s = 0; s < sl; s+= 50.0) {
            Geom::Path::Location pl = 
                natural_parameterisation(display_path, s, 1e-3);
            
            draw_circ(cr, display_path.point_at(pl));
        }
        cairo_restore(cr);
    }
#endif
    Geom::Path::HashCookie hash_cookie = display_path;
    {
        draw_line_seg(cr, Geom::Point(10,10), handles[0]);
        std::ostringstream gradientstr;
        gradientstr << "gradient: " << handles[0] - Geom::Point(10,10);
        gradientstr << std::ends;
        cairo_move_to(cr, handles[0][0], handles[0][1]);
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
            cairo_rotate(cr, atan2(handles[0]));
            pango_cairo_show_layout(cr, layout);
        }
        cairo_restore(cr);
    }    
    display_path.set_closed(true);
    cairo_path(cr, display_path);
    
    cairo_set_source_rgba (cr, 0.5, 0.7, 0.3, 0.8);
    //cairo_sub_path(cr, original_curve);
    
    double dist = INFINITY;

    cairo_save(cr);
    cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);

    //Geom::Path::Location pl = 
    //    display_path.nearest_location(old_mouse_point, dist);
    Geom::Path::Location pl = find_nearest_location(display_path, old_mouse_point);
    
    assert(hash_cookie == display_path);
    {
        Geom::Point pos, tgt, acc;
        display_path.point_tangent_acc_at (pl, pos, tgt, acc);
        *notify << pos << "\n";
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
#ifdef HAVE_GSL
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
#endif
        
    }
    //notify << "sub path length: " << arc_length_subdividing(display_path, 1e-3) << "\n";
    cairo_restore(cr);
/*    
      cairo_save(cr);
      cairo_set_source_rgba (cr, 0.25, 0.25, 0., 0.8);

    Geom::Path pth = display_path.subpath(display_path.indexed_elem(2), display_path.end());
    pth = pth*Geom::translate(Geom::Point(30, 30));
    draw_path(cr, pth);
    Bezier a, b;
    const int curve_seg = 3;
    Geom::Path::Elem ai(*display_path.indexed_elem(curve_seg)), bi(*pth.indexed_elem(curve_seg));
    
    for(int i = 0; i < 4; i++) {
        a.p[i] = ai[i];
        b.p[i] = bi[i];
    }
    cairo_restore(cr);
    
    std::vector<std::pair <double, double> > ts = Geom::FindIntersections(a, b);
    cairo_save(cr);
    cairo_set_source_rgba (cr, 0, 0.5, 0., 0.8);
    for(int i = 0; i < ts.size(); i++) {
        Geom::Path::Location pl(display_path.indexed_elem(curve_seg), ts[i].first);
        
        draw_handle(cr, display_path.point_at(pl));
        Geom::Path::Location p2(pth.indexed_elem(curve_seg), ts[i].second);
        
        draw_circ(cr, display_path.point_at(p2));
    }
    cairo_restore(cr);
    */
    
    if(0) {
        vector<Geom::Path::Location> pts = 
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
        vector<Geom::Path::Location> pts = 
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
    *notify << "Area: " << area;
    
    Geom::Point cntr;
    assert(hash_cookie == display_path);
    centroid(display_path, cntr, area);
    draw_circ(cr, cntr);
    cairo_move_to(cr, cntr[0], cntr[1]);
    cairo_show_text (cr, "center of the path");
    *notify << "pathwise Area: " << area << ", " << cntr;
        
    assert(hash_cookie == display_path);
  
//TODO:Remove?    
/*    double x = widget->allocation.x + widget->allocation.width / 2;
    double y = widget->allocation.y + widget->allocation.height / 2;

    double radius = std::min (widget->allocation.width / 2,
                              widget->allocation.height / 2) - 5;
*/
    Toy::draw(cr, notify, width, height, save);
}

virtual void key_hit(GdkEventKey *e) {
    if (e->keyval == 'q') {
        exit(0);
    } else if (e->keyval == 'r') {
        rotater = !rotater;
    } else if (e->keyval == 'v') {
        evolution = !evolution;
    } else if (e->keyval == 'n') {
        involution = !involution;
    } else if (e->keyval == 's') {
        half_stroking = !half_stroking;
    } else if (e->keyval == 'a') {
        equal_arc = !equal_arc;    
    } else if (e->keyval == 'd') {
        write_svgd(stderr, display_path);
    }
}

/*TODO:Integrate?
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
*/
    public:
    ToyCairo (char const *const file) {
        FILE* f = fopen(file, "r");
        if (!f) perror(file);
        
        display_path = read_svgd(f).front();
        Geom::Rect r = display_path.bbox();
    
        display_path = display_path*Geom::translate(-r.min());
        Geom::scale sc(r.max() - r.min());
        display_path = display_path*(sc.inverse()*Geom::scale(500,500));
        original_curve = display_path;

        handles.push_back(Geom::Point(20,20));
        rotater = evolution = half_stroking = involution = equal_arc = false;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "Toy Cairo", new ToyCairo(argc >= 2 ? argv[1] : "toy.svgd"));
    return 0;
    //g_idle_add((GSourceFunc)idler, canvas);
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
