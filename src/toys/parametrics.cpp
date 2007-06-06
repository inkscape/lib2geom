#include "d2.h"
#include "s-basis.h"
#include "s-basis-2d.h"
#include "bezier-to-sbasis.h"
#include "sb-geometric.h"
#include "path.h"
#include "svg-path-parser.h"

#include "path-cairo.h"
#include "toy-framework.h"

#include <vector>
#include <iostream>
using std::vector;
using namespace Geom;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double space=10){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 2;
    for( double t = M.cuts.front(); t < M.cuts.back(); t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_stroke(cr);
}

static Piecewise<D2<SBasis> > paths_to_pw(vector<Path> paths) {
    Piecewise<D2<SBasis> > ret = paths[0].toPwSb();
    for(unsigned i = 1; i < paths.size(); i++) {
        ret.concat(paths[i].toPwSb());
    }
    return ret;
}

class Parametrics: public Toy {
    Piecewise<D2<SBasis> > cat, alcat;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {    
      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      cairo_pw_d2(cr, cat);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,alcat);
      cairo_stroke(cr);
      *notify << "pieces = " << alcat.size() << ";\n";

      Toy::draw(cr, notify, width, height, save);
    }        

public:
    Parametrics(){
      cat = paths_to_pw(read_svgd("parametrics.svgd"));
      cat *= .3;
      cat += Point(150, 150);
	alcat = arc_length_parametrization(cat);
	handles.push_back(Point(100, 100));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Parametrics);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99:
