#include "path.h"
#include "svg-path-parser.h"

#include <iostream>
#include <cstdlib>

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

double rand_d() { return rand() % 100 / 100.0; }
void draw_bounds(cairo_t *cr, vector<Path> ps) {
    for(unsigned i = 0; i < ps.size(); i++) {
        for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++) {
            Rect bounds = it->boundsFast();
            cairo_set_source_rgba(cr, rand_d(), rand_d(), rand_d(), .5);
            draw_rect(cr, bounds.min(), bounds.max());
            cairo_stroke(cr);
        }
    }
}

void mark_verts(cairo_t *cr, vector<Path> ps) {
    for(unsigned i = 0; i < ps.size(); i++)
        for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++)
            draw_cross(cr, it->initialPoint());
}

int winding(vector<Path> ps, Point p) {
    int wind = 0;
    for(unsigned i = 0; i < ps.size(); i++)
        wind += ps[i].winding(p);
    return wind;
}

class WindingTest: public Toy {
    vector<Path> path;
    Piecewise<D2<SBasis> > pw;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_pw_d2(cr, pw);
        cairo_stroke(cr);
        
        srand(0); 
        draw_bounds(cr, path); mark_verts(cr, path);
        
        std::streambuf* cout_buffer = std::cout.rdbuf();
        std::cout.rdbuf(notify->rdbuf());
        *notify << "winding:" << winding(path, handles[0]) << "\n";
        std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    WindingTest () {
        path = read_svgd("winding.svgd");
        pw = paths_to_pw(path);
        handles.push_back(Point(300,300));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new WindingTest());
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
