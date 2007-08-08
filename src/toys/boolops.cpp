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

void cairo_paths(cairo_t *cr, Paths p) {
    for(Paths::iterator j = p.begin(); j != p.end(); j++) {
        cairo_path(cr, *j);
    }
}

void cairo_shape(cairo_t *cr, Shape s) {
    //cairo_set_source_rgba(cr, 1., 0., 0., .5);
    cairo_set_line_width(cr, 3);
    cairo_path(cr, s.getOuter());
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1);
    //cairo_set_source_rgba(cr, 0., 0., 1., .5);
    cairo_paths(cr, s.getHoles());
    cairo_stroke(cr);
}

void cairo_shapes(cairo_t *cr, Shapes s) {
    for(unsigned i = 0; i < s.size(); i++) {
        cairo_shape(cr, s[i]);
    }
}

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

Shape operator*(Shape const & sh, Matrix const &m) {
    Paths holes, h = sh.getHoles();
    for(Paths::iterator j = h.begin(); j != h.end(); j++) {
        holes.push_back((*j) * m);
    }
    return Shape(sh.getOuter() * m, holes);
}

Shapes operator*(Shapes const & sh, Matrix const &m) {
    Shapes ret;
    for(unsigned i = 0; i < sh.size(); i++) ret.push_back(sh[i] * m);
    return ret;
}

//true = clockwise, false = counter-clock
Path set_winding(Path const &p, bool dir) {
    Piecewise<D2<SBasis> > pw = p.toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    if(area < 0 && !dir) return p.reverse();
    if(area > 0 && dir) return p.reverse();
    return p; 
}

Shape cleanup(std::vector<Path> const &ps) {
    unsigned ix = outer_index(ps);
    
    Path outer = set_winding(ps[ix], false);

    Piecewise<D2<SBasis> > pw = outer.toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    
    Paths holes;
    for(unsigned i = 0; i < ps.size(); i++) {
        if(i != ix && contains(outer, ps[i].initialPoint()))
            holes.push_back(set_winding(ps[i], true));
    }
    return Shape(outer, holes) * Geom::Translate(-centre);
}

class BoolOps: public Toy {
    Path a, b;
    Shape as, bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Shape bst = bs * Geom::Translate(handles[0]);
        Path bt = bst.getOuter();
        //cairo_shape(cr, as);
        //cairo_shape(cr, bst);
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 0., 0., 0., 1);
        mark_crossings(cr, a, bt);
        cairo_stroke(cr);
        //draw_bounds(cr, a);
        //draw_bounds(cr, b);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        
        Shapes suni = shape_subtract(as, bst); //path_union(a, b);
        cairo_shapes(cr, suni);
        
        /*Shapes uni = path_union(a, bt);
        cairo_set_source_rgba(cr, 1., 0., 0., .5);
        cairo_shapes(cr, uni);
        
        Shapes sub = path_subtract(a, bt);
        cairo_set_source_rgba(cr, 0., 0., 0., .5);
        cairo_shapes(cr, sub * Geom::Translate(Point(10, 10)));
        cairo_stroke(cr);
        
        Paths inte = path_intersect(a, bt);
        cairo_set_source_rgba(cr, 0., 1., 0., .5);
        cairo_paths(cr, inte);
        */
        
        //std::cout.rdbuf(cout_buffer);

        *notify << "Red = Union exterior, Blue = Holes in union\n Green = Intersection\nSubtraction is meant to be shifted.\n";

        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
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
        
        handles.push_back(Point(100,100));
        
        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b); //path_subtract(path_b[0] * Geom::Translate(-centre), path_b[0] * Geom::Translate(-centre) * Scale(.5, .5)).front();
        
        Paths holes = bs.getHoles();
        holes.push_back(bs.getOuter() * Geom::Scale(.5, .5));
        
        bs = Shape(bs.getOuter(), holes);
        
        a = as.getOuter();
        b = bs.getOuter();
    }
    bool should_draw_bounds() {return false;}
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
