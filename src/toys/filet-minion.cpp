#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/path-intersection.h>
#include <2geom/nearest-time.h>
#include <2geom/circle.h>

#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>
using namespace Geom;
using namespace std;

class IntersectDataTester: public Toy {
    int nb_paths;
    int nb_curves_per_path;
    int degree;

    std::vector<PointSetHandle> paths_handles;
    std::vector<Slider> sliders;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
	std::vector<D2<SBasis> > pieces;
        PathVector paths;
        for (int i = 0; i < nb_paths; i++){
            paths.push_back(Path(paths_handles[i].pts[0]));
            for (unsigned j = 0; j+degree < paths_handles[i].size(); j+=degree){
                D2<SBasis> c = handles_to_sbasis(paths_handles[i].pts.begin()+j, degree);
                paths[i].append(c);
		pieces.push_back(c);
            }
        }
        
        cairo_path(cr, paths);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
        double r = sliders[0].value();
	
        D2<SBasis> B = pieces[0];
        Piecewise<D2<SBasis> > offset_curve0 = Piecewise<D2<SBasis> >(pieces[0])+rot90(unitVector(derivative(pieces[0])))*(-r);
        Piecewise<D2<SBasis> > offset_curve1 = Piecewise<D2<SBasis> >(pieces[1])+rot90(unitVector(derivative(pieces[1])))*(-r);

        //cairo_pw_d2_sb(cr, offset_curve0);
        //cairo_pw_d2_sb(cr, offset_curve1);
        //cairo_stroke(cr);
        
        
        Path p0 = path_from_piecewise(offset_curve0, 0.1)[0];
        Path p1 = path_from_piecewise(offset_curve1, 0.1)[0];
        Crossings cs = crossings(p0, p1);
        
        
            for(auto & c : cs) {
                *notify << c.ta << ", " << c.tb << '\n';
                Point cp =p0(c.ta);
                //draw_circ(cr, cp);
                //cairo_stroke(cr);
                double p0pt = nearest_time(cp, pieces[0]);
                double p1pt = nearest_time(cp, pieces[1]);
                Circle circ(cp[0], cp[1], r);
                //cairo_arc(cr, circ.center(X), circ.center(Y), circ.ray(), 0, 2*M_PI);
                
                std::unique_ptr<EllipticalArc> eap(
                    circ.arc(pieces[0](p0pt), pieces[0](1), pieces[1](p1pt)) );
                D2<SBasis> easb = eap->toSBasis();
                cairo_d2_sb(cr, easb);
                cairo_stroke(cr);
            }        
        
	Point ends[2];
        if (0)
	for(int endi = 0; endi < 2; endi++) {
	  D2<SBasis> dist = pieces[endi]-pieces[0].at1();
	  *notify << dist << "\n";
	  vector<double> locs = roots(dot(dist,dist) - SBasis(r*r));
	  for(double loc : locs) {
	    //draw_circ(cr, pieces[endi](locs[i]));
	    *notify  << loc << ' ';
	  }
	  if(locs.size()) {
	    std::sort(locs.begin(), locs.end());
	    if (endi)
	      ends[endi] = pieces[endi](locs[0]);
	    else
	      ends[endi] = pieces[endi](locs.back());
	    draw_circ(cr, ends[endi]);
	  }
	}

        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    public:
    IntersectDataTester(int paths, int curves_in_path, int degree) :
        nb_paths(paths), nb_curves_per_path(curves_in_path), degree(degree) {
        for (int i = 0; i < nb_paths; i++){
            paths_handles.push_back(PointSetHandle());
        }
        for(int i = 0; i < nb_paths; i++){
            for(int j = 0; j < (nb_curves_per_path*degree)+1; j++){
                paths_handles[i].push_back(uniform()*400, 100+ uniform()*300);
            }
            handles.push_back(&paths_handles[i]);
        }
        sliders.push_back(Slider(0.0, 100.0, 1, 30.0, "min radius"));
        sliders.push_back(Slider(0.0, 100.0, 1, 0.0, "ray chooser"));
        sliders.push_back(Slider(0.0, 100.0, 1, 0.0, "area chooser"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
        sliders[0].geometry(Point(50, 20), 250);
        sliders[1].geometry(Point(50, 50), 250);
        sliders[2].geometry(Point(50, 80), 250);
    }

    void first_time(int /*argc*/, char** /*argv*/) override {

    }
};

int main(int argc, char **argv) {
    unsigned paths=1;
    unsigned curves_in_path=2;
    unsigned degree=3;
    if(argc > 3)
        sscanf(argv[3], "%d", &degree);
    if(argc > 2)
        sscanf(argv[2], "%d", &curves_in_path);
    if(argc > 1)
        sscanf(argv[1], "%d", &paths);
    init(argc, argv, new IntersectDataTester(paths, curves_in_path, degree));
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
