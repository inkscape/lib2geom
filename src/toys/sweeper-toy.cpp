#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>


#include <cstdlib>
#include <set>
#include <vector>
#include <algorithm>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>

using namespace Geom;
using namespace std;

#include "sweeper.cpp"

double exp_rescale(double x){ return pow(10, x);}
std::string exp_formatter(double x){ return default_formatter(exp_rescale(x));}


class SweeperToy: public Toy {
    int nb_paths;
    int nb_curves_per_path;
    int degree;

    std::vector<PointSetHandle> paths_handles;
    std::vector<Slider> sliders;
    Sweeper sweeper;

    void drawTile( cairo_t *cr, unsigned idx , unsigned line_width=1){
        if (idx>=sweeper.tiles_data.size()) return;
        Rect box;
        box = sweeper.tiles_data[idx].fbox;
        box[X].expandBy(1);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0., .5);
        cairo_set_line_width (cr, line_width);
        cairo_stroke(cr);
        box = sweeper.tiles_data[idx].tbox;
        box[Y].expandBy(1);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 0., 0., 1., .5);
        cairo_set_line_width (cr, line_width);
        cairo_stroke(cr);

        Sweeper::Tile tile = sweeper.tiles_data[idx];
        D2<SBasis> p = sweeper.paths[tile.path][tile.curve].toSBasis();
        Interval dom = Interval(tile.f,tile.t);
        cairo_set_source_rgba (cr, 0., 1., .5, .8);
        p = portion(p, dom);
        cairo_d2_sb(cr, p);
        cairo_set_line_width (cr, line_width);
        cairo_stroke(cr);
    }

    void drawTiles( cairo_t *cr ){
        for (unsigned i=0; i<sweeper.tiles_data.size(); i++){
            drawTile( cr, i );
        }

//         for (unsigned i=0; i<sweeper.vtxboxes.size(); i++){
//             cairo_rectangle(cr, sweeper.vtxboxes[i]);
//             cairo_set_source_rgba (cr, 0., 0., 0, 1);
//             cairo_set_line_width (cr, 1);
//             cairo_stroke(cr);
//         }
    }

    void enlightTile( cairo_t *cr, unsigned idx){
        drawTile(cr, idx, 4);
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);

        PathVector paths;
        for (int i = 0; i < nb_paths; i++){
            paths_handles[i].pts.back()=paths_handles[i].pts.front();
            paths.push_back(Path(paths_handles[i].pts[0]));
            for (unsigned j = 0; j+degree < paths_handles[i].size(); j+=degree){
                D2<SBasis> c = handles_to_sbasis(paths_handles[i].pts.begin()+j, degree);
                paths[i].append(c);
            }
            paths[i].close();
        }
        
        //cairo_path(cr, paths);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);

        double tol = exp_rescale(sliders[3].value());
    	Rect tolbytol( Point(50,110), Point(50,110) );
    	tolbytol.expandBy( tol );
        cairo_rectangle(cr, tolbytol);
    	cairo_stroke(cr);

        sweeper = Sweeper(paths,X, tol);
       unsigned idx = (unsigned)(sliders[0].value()*(sweeper.tiles_data.size()-1));
       drawTiles(cr);
       enlightTile(cr, idx);

        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    public:
    SweeperToy(int paths, int curves_in_path, int degree) :
        nb_paths(paths), nb_curves_per_path(curves_in_path), degree(degree) {
        for (int i = 0; i < nb_paths; i++){
            paths_handles.emplace_back();
        }
        for(int i = 0; i < nb_paths; i++){
            for(int j = 0; j < (nb_curves_per_path*degree)+1; j++){
                paths_handles[i].push_back(uniform()*400, 100+ uniform()*300);
            }
            handles.push_back(&paths_handles[i]);
        }
        sliders.emplace_back(0.0, 1, 0, 0.0, "intersection chooser");
        sliders.emplace_back(0.0, 1, 0, 0.0, "ray chooser");
        sliders.emplace_back(0.0, 1, 0, 0.0, "area chooser");
        sliders.emplace_back(-5.0, 2, 0, 0.0, "tolerance chooser");
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
        handles.push_back(&(sliders[3]));
        sliders[0].geometry(Point(50, 20), 250);
        sliders[1].geometry(Point(50, 50), 250);
        sliders[2].geometry(Point(50, 80), 250);
        sliders[3].geometry(Point(50, 110), 250);
		sliders[3].formatter(&exp_formatter);
    }

    void first_time(int /*argc*/, char** /*argv*/) override {

    }
};

int main(int argc, char **argv) {
    unsigned paths=10;
    unsigned curves_in_path=3;
    unsigned degree=1;
    if(argc > 3)
        sscanf(argv[3], "%d", &degree);
    if(argc > 2)
        sscanf(argv[2], "%d", &curves_in_path);
    if(argc > 1)
        sscanf(argv[1], "%d", &paths);
    init(argc, argv, new SweeperToy(paths, curves_in_path, degree));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
