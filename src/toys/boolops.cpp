#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

#include <cstdlib>

using namespace Geom;

void cairo_shape(cairo_t *cr, Shapes s) {
    for(unsigned i = 0; i < s.size(); i++) {
        cairo_path(cr, s[i].getOuter());
    }
}

void mark_crossings(cairo_t *cr, Path const &a, Path const &b) {
    Crossings c = crossings(a, b);
    for(Crossings::iterator i = c.begin(); i != c.end(); i++) {
        draw_cross(cr, a.pointAt(i->ta));
        draw_cross(cr, Point(i->ta * 10, i->ta * 10));
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
    vector<Path> path_a, path_b;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        //Paths none;
        //Shape a(path_a.front(), none), b(path_b.front(), none);
        Path a(path_a.front()), b(path_b.front() * Matrix(1, 0, 0, 1, handles[0][X], handles[0][Y]));
        cairo_path(cr, a);
        cairo_path(cr, b);
        cairo_stroke(cr);
        
        Path port = a.portion(handles[1][X] / 100., handles[2][X] / 100.);
        cairo_set_source_rgba(cr, 0., 1., 0., 1.);
        cairo_path(cr, port);
        cairo_stroke(cr);
        
        mark_crossings(cr, a, b);
        draw_bounds(cr, a);
        draw_bounds(cr, b);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        Shapes res = path_union(a, b);
        cairo_set_source_rgba(cr, 1., 0., 0., 1);
        cairo_shape(cr, res);
        cairo_stroke(cr);
        //std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    BoolOps () {
        path_a = read_svgd("winding.svgd");
        path_b = read_svgd("monk.svgd");
        handles.push_back(Point(300,300));
        handles.push_back(Point(200,300));
        handles.push_back(Point(250,300));
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
