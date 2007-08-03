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

void cairo_shapes(cairo_t *cr, Shapes s) {
    for(unsigned i = 0; i < s.size(); i++) {
        cairo_set_source_rgba(cr, 1., 0., 0., .5);
        cairo_path(cr, s[i].getOuter());
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, 0., 0., 1., .5);
        cairo_paths(cr, s[i].getHoles());
        cairo_stroke(cr);
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

class BoolOps: public Toy {
    Point centre;
    vector<Path> path_a, path_b;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        //Paths none;
        //Shape a(path_a.front(), none), b(path_b.front(), none);
        Path a(path_a.front());
        Path b(path_b.front() * Geom::Translate(handles[0]-centre));
        cairo_path(cr, a);
        cairo_path(cr, b);
        cairo_stroke(cr);
        
        /*Path port = a.portion(handles[1][X] / 100., handles[2][X] / 100.);
        cairo_set_source_rgba(cr, 0., 1., 0., 1.);
        cairo_path(cr, port);
        cairo_stroke(cr); */
        
        mark_crossings(cr, a, b);
        draw_bounds(cr, a);
        draw_bounds(cr, b);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        cairo_set_line_width(cr, 5);
        
        Shapes uni = path_union(a, b);
        cairo_set_source_rgba(cr, 1., 0., 0., .5);
        cairo_shapes(cr, uni);
        cairo_stroke(cr);
        
         Paths inte = path_intersect(a, b);
        cairo_set_source_rgba(cr, 0., 1., 0., .5);
        cairo_paths(cr, inte);
        cairo_stroke(cr);
        //std::cout.rdbuf(cout_buffer);

        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        char *path_a_name="winding.svgd";
        char *path_b_name="monk.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];
        path_a = read_svgd(path_a_name);
        path_b = read_svgd(path_b_name);
        handles.push_back(Point(300,300));
        //handles.push_back(Point(200,300));
        //handles.push_back(Point(250,300));
        double area;
        Piecewise<D2<SBasis> > pw = path_b[0].toPwSb();
        Geom::centroid(pw, centre, area);
        std::cout << "monk area = " << area << std::endl;
    }
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
