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

void cairo_regions(cairo_t *cr, Regions p) {
    for(Regions::iterator j = p.begin(); j != p.end(); j++) {
        if(j->fill()) cairo_set_source_rgb(cr, 1., 0., 0.); else cairo_set_source_rgb(cr, 0., 0., 1.);
        cairo_path(cr, j->boundary());
        cairo_stroke(cr);
    }
}

/*
void mark_crossings(cairo_t *cr, Path const &a, Path const &b) {
    Crossings c = crossings(a, b);
    for(Crossings::iterator i = c.begin(); i != c.end(); i++) {
        draw_cross(cr, a.pointAt(i->ta));
        draw_text(cr, a.pointAt(i->ta), i->dir ? "T" : "F");
    }
}

void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

double rand_d() { return rand() % 100 / 100.0; }
void draw_bounds(cairo_t *cr, Path p) {
    srand(0); 
    for(Path::iterator it = p.begin(); it != p.end(); it++) {
        Rect bounds = it->boundsFast();
        cairo_set_source_rgba(cr, rand_d(), rand_d(), rand_d(), .5);
        draw_rect(cr, bounds.min(), bounds.max());
        cairo_stroke(cr);
    }
}

*/

Shape cleanup(std::vector<Path> const &ps) {
    Regions rs = regions_from_paths(ps);

    unsigned ix = outer_index(rs);
    if(ix == rs.size()) ix = 0;

    Region outer;
    for(int i = 0; i < rs.size(); i++) {
        if(i == ix) {
            outer = !rs[i].fill() ? rs[i].inverse() : rs[i];
            rs.erase(rs.begin() + i);
            i--;
        } else {
            rs[i] = rs[i].fill() ? rs[i].inverse() : rs[i];
        }
    }

    Piecewise<D2<SBasis> > pw = outer.boundary().toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    
    return Shape(outer, rs) * Geom::Translate(-centre);
}

class BoolOps: public Toy {
    Region a, b;
    bool rev;
    Shape as, bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(handles[0]);
        Shape bst = bs * t;
        Region bt = Region(b.boundary() * t, b.fill());
        //cairo_shape(cr, as);
        //cairo_shape(cr, bst);
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 0., 0., 0., 1);
        //mark_crossings(cr, a, bt);
        cairo_stroke(cr);
        //draw_bounds(cr, a);
        //draw_bounds(cr, b);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        
        //Shapes suni = shape_subtract(as, bst); //path_union(a, b);
        //cairo_shapes(cr, suni);
        
        Regions cont = region_boolean(rev, a, bt);
        cairo_regions(cr, cont);
        
        //used to check if it's right
        for(int i = 0; i < cont.size(); i++) {
            if(path_direction(cont[i].boundary()) != cont[i].fill()) std::cout << "wrong!\n";
        }
        
        /*Shapes uni = path_union(a, bt);
        cairo_set_source_rgba(cr, 1., 0., 0., .5);
        cairo_shapes(cr, uni);
        */
        /*Shapes sub = path_subtract(a, bt);
        cairo_set_source_rgba(cr, 0., 0., 0., .5);
        cairo_shapes(cr, sub);
        cairo_stroke(cr);
        
        std::vector<Path> inte = path_intersect(a, bt);
        cairo_set_source_rgba(cr, 0., 1., 0., .5);
        cairo_paths(cr, inte);
        cairo_stroke(cr);
        */
        //std::cout.rdbuf(cout_buffer);

        //*notify << "Red = Union exterior, Blue = Holes in union\n Green = Intersection\nSubtraction is meant to be shifted.\n";

        *notify << "a " << (a.fill() ? "" : "not") << " filled\n";
        *notify << "b " << (b.fill() ? "" : "not") << " filled\n";
        *notify << "rev = " << (rev ? "true" : "false") << "\n";
        
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'a') a = Region(a.boundary().reverse(), !a.fill());
        if(e->keyval == 'b') b = Region(b.boundary().reverse(), !b.fill());
        if(e->keyval == 'r') rev = !rev;
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
        
        paths_b[0] = paths_b[0] * Geom::Scale(Point(.75, .75));
        rev = false;
        handles.push_back(Point(100,100));
              
        //Paths holes = bs.getHoles();
        //holes.push_back(bs.getOuter() * Geom::Scale(.5, .5));
        
        //bs = Shape(bs.getOuter(), holes);
        Geom::Matrix m = Geom::Translate(Point(300, 300));
        std::cout << m.flips() << "\n";
        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b); //path_subtract(path_b[0] * Geom::Translate(-centre), path_b[0] * Geom::Translate(-centre) * Scale(.5, .5)).front();
        
        a = as.getOuter();
        b = bs.getOuter();
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
