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
    double dt=space;
    double t = M.cuts.front();
    while (t < M.cuts.back()){
        draw_handle(cr, M(t));
        t += dt;
    }
    cairo_pw_d2(cr, M);
    cairo_stroke(cr);
}

class Parametrics: public Toy {

    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
      vector<Path> ps = read_svgd("parametrics.svgd");
      std::cerr << "woo";
      std::cerr << ps.size();
      //Piecewise<D2<SBasis> > B = ps[0].toMdSb();
     /* for(unsigned i = 1; i < ps.size(); i++) {
          B.concat(ps[i].toMdSb());
      }*/
     /*
      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      //cairo_md_sb(cr, B1);
      cairo_pw_d2(cr, B);
      cairo_stroke(cr);

      Piecewise<D2<SBasis> > uniform_B = arc_length_parametrization(B);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,uniform_B);
      cairo_stroke(cr);
      *notify << "pieces = " << uniform_B.size() << ";\n";
*/
      Toy::draw(cr, notify, width, height, save);
    }        
/*
public:
    Parametrics(){
    }*/
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
