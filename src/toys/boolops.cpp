#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework.cpp"
#include "transforms.h"
#include "sbasis-geometric.h"

#include <cstdlib>

using namespace Geom;


void cairo_region(cairo_t *cr, Region const &r) {
    double d = 5.;
    if(!r.isFill()) cairo_set_dash(cr, &d, 1, 0);
    cairo_path(cr, r);
    cairo_stroke(cr);
    cairo_set_dash(cr, &d, 0, 0);
}

void cairo_regions(cairo_t *cr, Regions const &p) {
    for(Regions::const_iterator j = p.begin(); j != p.end(); j++)
        cairo_region(cr, *j);
}

void cairo_shape(cairo_t *cr, Shape const &s) {
    cairo_regions(cr, s.getContent());
}

std::vector<Path> desanitize(Shape const & s) {
    return paths_from_regions(s.getContent());
}

void mark_crossings(cairo_t *cr, Shape const &a, Shape const &b) {
    const Regions ac = a.getContent();
    CrossingSet cc = crossings_between(a, b);
    for(unsigned j = 0; j < cc.size(); j++) {
        Crossings c = cc[j];
        for(Crossings::iterator i = c.begin(); i != c.end(); i++) {
            draw_cross(cr, Path(ac[i->a]).pointAt(i->ta));
            cairo_stroke(cr);
            //draw_text(cr, ac[i->a].getBoundary().pointAt(i->ta), i->dir ? "T" : "F");
        }
    }
}

Shape cleanup(std::vector<Path> const &ps) {
    Regions rs = regions_from_paths(ps);
    
    /* for(unsigned i = 0; i < rs.size(); i++) {
        Point exemplar = rs[i].getBoundary().initialPoint();
        for(unsigned j = 0; j < rs.size(); j++) {
            if(i != j && rs[j].contains(exemplar)) {
                if(rs[i].isFill()) rs[i] = rs[i].inverse();
                if(!rs[j].isFill()) rs[j] = rs[j].inverse();
                goto next;
            }
        }
        if(!rs[i].isFill()) rs[i] = rs[i].inverse();
        next: (void)0;
    }*/
    
    Piecewise<D2<SBasis> > pw = paths_to_pw(ps);
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    
    return Shape(rs) * Geom::Translate(-centre);
}

class BoolOps: public Toy {
    std::vector<Toggle> togs;
    Shape as, bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(handles[0]);
        Shape bst = bs * t;
        
        cairo_set_line_width(cr, 1);
        mark_crossings(cr, as, bst);
        
        unsigned ttl = 0, v = 1;
        for(unsigned i = 0; i < 4; i++, v*=2)
            if(togs[i].on) ttl += v; 
        /*
        Shape s = shape_boolean(as, bst, ttl);
        
        cairo_set_source_rgba(cr, 182./255, 200./255, 183./255, 1);
        if(!s.isFill()) {
            cairo_rectangle(cr, 0, 0, width, height);
            cairo_fill(cr);
            cairo_set_source_rgba(cr, 1, 1, 1, 1);
        }
        cairo_path(cr, desanitize(s));
        cairo_fill(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        cairo_shape(cr, s);
        */
        double x = width - 60, y = height - 60;
        
        cairo_shape(cr, as); cairo_shape(cr, bst);
        
        //Draw the info
        
        draw_text(cr, Point(x + 20, y - 34), "A");
        draw_text(cr, Point(x + 5, y - 18),  "T");
        draw_text(cr, Point(x + 32, y - 18), "F");
        
        draw_text(cr, Point(x - 25, y + 17), "B");
        draw_text(cr, Point(x - 15, y + 2),  "T");
        draw_text(cr, Point(x - 15, y + 28), "F");

        draw_text(cr, Point(width - 425, height - 70), "KEY:");
        draw_text(cr, Point(width - 425, height - 50), "T/F = Containment/Non-containment,");
        draw_text(cr, Point(width - 425, height - 30), "Q/W/A/S = The keys on the keyboard");
        
        Point p(x, y), d(25,25), xo(25,0), yo(0,25);
        togs[2].bounds = Rect(p,     p + d);
        togs[0].bounds = Rect(p + xo, p + xo + d);
        togs[1].bounds = Rect(p + yo, p + yo + d);
        togs[3].bounds = Rect(p + d, p + d + d);

        draw_toggles(cr, togs);

        //*notify << "Operation: " << (mode ? (mode == 1 ? "union" : (mode == 2 ? "subtract" : (mode == 3 ? "intersect" : "exclude"))) : "none");
        //*notify << "\nKeys:\n u = Union   s = Subtract   i = intersect   e = exclude   0 = none   a = invert A   b = invert B \n";
        
        //*notify << "A " << (as.isFill() ? "" : "not") << " filled, B " << (bs.isFill() ? "" : "not") << " filled..\n";
        
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'w') togs[0].toggle(); else
        if(e->keyval == 'a') togs[1].toggle(); else
        if(e->keyval == 'q') togs[2].toggle(); else
        if(e->keyval == 's') togs[3].toggle();
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(togs, e);
        Toy::mouse_pressed(e);
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        char *path_a_name="winding.svgd";
        char *path_b_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        std::vector<Path> paths_b = read_svgd(path_b_name);
             
        handles.push_back(Point(700,700));
        

        togs.push_back(Toggle("W", true));
        togs.push_back(Toggle("A", true));
        togs.push_back(Toggle("Q", true));
        togs.push_back(Toggle("S", false));
        
        paths_b[0] = paths_b[0].reverse();
        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b);
    }
    virtual bool should_draw_numbers() {return false;}
};

int main(int argc, char **argv) {
    init(argc, argv, new BoolOps());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
