#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <2geom/shape.h>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/ord.h>

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

void show_stitches(cairo_t *cr, Shape const &s) {
    Regions const &p = s.getContent();
    cairo_set_source_rgb(cr, 1, 0, 0);
    for (Regions::const_iterator j = p.begin(); j != p.end() ; ++j ) {
        cairo_path_stitches(cr, *j);
        cairo_stroke(cr);
    }
}

void mark_crossings(cairo_t *cr, Shape const &a, Shape const &b) {
    const Regions ac = a.getContent();
    Crossings c = crossings(Path(a[0]), Path(b[0]));
    //for(unsigned j = 0; j < cc.size(); j++) {
        //Crossings c = cc[j];
        for(Crossings::iterator i = c.begin(); i != c.end(); i++) {
            draw_cross(cr, Path(ac[i->a]).pointAt(i->ta));
            cairo_stroke(cr);
            //draw_text(cr, ac[i->a].getBoundary().pointAt(i->ta), i->dir ? "T" : "F");
        }
    //}
}

void mark_crossings(cairo_t* cr, std::vector<Path> &a) {
    CrossingSet crs = crossings_among(a);
    for(unsigned i = 0; i < crs.size(); i++) {
        for(unsigned j = 0; j < crs[i].size(); j++) {
            Crossing cur = crs[i][j];
            draw_cross(cr, a[i].pointAt(cur.a == i ? cur.ta : cur.tb));
            cairo_stroke(cr);
        }
    }
}

Shape cleanup(std::vector<Path> const &ps) {
    Piecewise<D2<SBasis> > pw = paths_to_pw(ps);
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    
    if(fabs(area) > 1)
        return sanitize(ps) * Geom::Translate(-centre);
    else
        return sanitize(ps);
}

class BoolOps: public Toy {
    std::vector<Toggle> toggles;
    Shape as, bs;
    PointHandle p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(p.pos);
        Shape bst = bs * t;
        
        cairo_set_line_width(cr, 1);
        //mark_crossings(cr, as, bst);
        
        /* std::vector<Path> ps;
        ps.push_back(bst[0]);
        std::vector<Path> ap = paths_from_regions(as.getContent());
        ps.insert(ps.end(), ap.begin(), ap.begin()+1);
        
        Shape rgs = sanitize(ps);
        cairo_shape(cr, rgs); */
               
        unsigned ttl = 0, v = 1;
        for(unsigned i = 0; i < 4; i++, v*=2)
            if(toggles[i].on) ttl += v; 
        
        
        Shape s = boolop(as, bst, ttl);

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        
        cairo_set_source_rgba(cr, 182./255, 200./255, 183./255, 1);
        cairo_set_line_width(cr, 1);
        if(!s.isFill()) {
            cairo_rectangle(cr, 0, 0, width, height);
            cairo_fill(cr);
            cairo_set_source_rgba(cr, 1, 1, 1, 1);
        }
        cairo_path(cr, desanitize(s));
        cairo_fill(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        cairo_shape(cr, s);

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_set_line_width(cr, 8);
        show_stitches(cr, s);

        double x = width - 60, y = height - 60;
        
        //cairo_shape(cr, as);
        //cairo_shape(cr, bst);
        
        //Draw the info

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
        
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
        toggles[2].bounds = Rect(p,     p + d);
        toggles[0].bounds = Rect(p + xo, p + xo + d);
        toggles[1].bounds = Rect(p + yo, p + yo + d);
        toggles[3].bounds = Rect(p + d, p + d + d);

        draw_toggles(cr, toggles);

        //*notify << "Operation: " << (mode ? (mode == 1 ? "union" : (mode == 2 ? "subtract" : (mode == 3 ? "intersect" : "exclude"))) : "none");
        //*notify << "\nKeys:\n u = Union   s = Subtract   i = intersect   e = exclude   0 = none   a = invert A   b = invert B \n";
        
        //*notify << "A " << (as.isFill() ? "" : "not") << " filled, B " << (bs.isFill() ? "" : "not") << " filled..\n";
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'w') toggles[0].toggle(); else
        if(e->keyval == 'a') toggles[1].toggle(); else
        if(e->keyval == 'q') toggles[2].toggle(); else
        if(e->keyval == 's') toggles[3].toggle();
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        const char *path_a_name="winding.svgd";
        const char *path_b_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        std::vector<Path> paths_b = read_svgd(path_b_name);
             
        p = PointHandle(Point(300,300));
        handles.push_back(&p);
        
        toggles.push_back(Toggle("W", true));
        toggles.push_back(Toggle("A", true));
        toggles.push_back(Toggle("Q", true));
        toggles.push_back(Toggle("S", false));
        
        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b);
        //bs = boolop(bs, bs * Geom::Scale(.5), BOOLOP_SUBTRACT_A_B);
    }
    //virtual bool should_draw_numbers() {return false;}
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
