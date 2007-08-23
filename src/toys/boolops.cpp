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

double rand_d() { return rand() % 100 / 100.0; }
void cairo_region(cairo_t *cr, Region const &r) {
    cairo_set_source_rgba(cr, 0, 0, 0, 1); //rand_d(), rand_d(), rand_d(), .75);
    double d = 5.;
    if(!r.isFill()) cairo_set_dash(cr, &d, 1, 0);
    cairo_path(cr, r.getBoundary());
    cairo_stroke(cr);
    cairo_set_dash(cr, &d, 0, 0);
}

void cairo_regions(cairo_t *cr, Regions const &p) {
    srand(0); 
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
            draw_cross(cr, ac[i->a].getBoundary().pointAt(i->ta));
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
    Region a, b;
    unsigned mode;
    bool rev;
    Shape as, bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(handles[0]);
        Shape bst = bs * t;
        Region bt = Region(b.getBoundary() * t, b.isFill());
        
        cairo_set_line_width(cr, 1);
        mark_crossings(cr, as, bst);
        
        Shape s;
        switch(mode) {
        case 0:
            cairo_shape(cr, as);
            cairo_shape(cr, bst);
            goto skip;
        case 1:
            s = shape_union(as, bst);
            break;
        case 2:
            s = shape_subtract(as, bst);
            break;
        case 3:
            s = shape_intersect(as, bst);
            break;
        case 4:
            s = shape_exclude(as, bst);
            break;
        }
        if(mode<4) cairo_shape(cr, s); else cairo_path(cr, desanitize(s));
        cairo_fill(cr);
        skip:
        *notify << "Operation: " << (mode ? (mode == 1 ? "union" : (mode == 2 ? "subtract" : (mode == 3 ? "intersect" : "exclude"))) : "none");
        *notify << "\nKeys:\n u = Union   s = Subtract   i = intersect   e = exclude   0 = none";
        
        //*notify << "A " << (as.isFill() ? "" : "not") << " filled, B " << (bs.isFill() ? "" : "not") << " filled..\n";
        //*notify << "rev = " << (rev ? "true" : "false");
        
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        //if(e->keyval == 'r') rev = !rev; else
        //if(e->keyval == 'a') as = as.inverse(); else
        //if(e->keyval == 'b') bs = bs.inverse();
        if(e->keyval == '0') mode = 0; else
        if(e->keyval == 'u') mode = 1; else
        if(e->keyval == 's') mode = 2; else
        if(e->keyval == 'i') mode = 3; else
        if(e->keyval == 'e') mode = 4;
        redraw();
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
        
        mode = 0; rev = false;
        
        handles.push_back(Point(700,700));

        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b).inverse();
        //bs = shape_subtract(bs, bs * Scale(.5, .5)).front();
        a = as.getContent().front();
        b = bs.getContent().front();
    }
    int should_draw_bounds() {return 0;}
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
