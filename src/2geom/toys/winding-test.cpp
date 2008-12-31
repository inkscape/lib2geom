#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>

#include <iostream>
#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/ord.h>
using namespace Geom;

void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

void draw_bounds(cairo_t *cr, vector<Path> ps) {
    srand(0); 
    vector<Rect> bnds;
    for(unsigned i = 0; i < ps.size(); i++) {
        for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++) {
            Rect bounds = *(it->boundsFast());
            bnds.push_back(bounds);
            cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
            //draw_rect(cr, bounds.min(), bounds.max());
            cairo_stroke(cr);
        }
    }
    {
        std::vector<std::vector<unsigned> > res = sweep_bounds(bnds);
        cairo_set_line_width(cr,0.5);
        cairo_save(cr);
        cairo_set_source_rgb(cr, 1, 0, 0);
        for(unsigned i = 0; i < res.size(); i++) {
            for(unsigned j = 0; j < res[i].size(); j++) {
                draw_line_seg(cr, bnds[i].midpoint(), bnds[res[i][j]].midpoint());
                cairo_stroke(cr);
            }
        }
        cairo_restore(cr);
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
        wind += winding(ps[i],p);
    return wind;
}

class WindingTest: public Toy {
    vector<Path> path;
    PointHandle test_pt_handle;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_path(cr, path);
        cairo_stroke(cr);
        mark_verts(cr, path);
        draw_bounds(cr, path);
        
        //draw_bounds(cr, path); mark_verts(cr, path);
        
        std::streambuf* cout_buffer = std::cout.rdbuf();
        std::cout.rdbuf(notify->rdbuf());
        *notify << "\nwinding:" << winding(path, test_pt_handle.pos) << "\n";
        std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    WindingTest () : test_pt_handle(300,300) {}
    void first_time(int argc, char** argv) {
        const char *path_name="winding.svgd";
        if(argc > 1)
            path_name = argv[1];
        path = read_svgd(path_name);
        
        handles.push_back(&test_pt_handle);
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
