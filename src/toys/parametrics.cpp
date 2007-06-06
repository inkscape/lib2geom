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

static void draw_axis(cairo_t *cr, Piecewise<D2<SBasis> > const &M, unsigned ax, Point offset, double st=0) {
    Piecewise<D2<SBasis> > pw(M);
    if(st != 0) {
	if(st>pw.cuts.back()) return;
        pw = portion(M, st, pw.cuts.back());
        pw.offsetDomain(-st);
    }
    for(unsigned i = 0; i < pw.size(); i++) {
        if(!ax)
            cairo_md_sb(cr, D2<SBasis>(pw[i][0], Linear(pw.cuts[i],pw.cuts[i+1])*5) + offset);
        else
            cairo_md_sb(cr, D2<SBasis>(Linear(pw.cuts[i],pw.cuts[i+1])*5, pw[i][1]) + offset);
    }
}

class Parametrics: public Toy {
    Piecewise<D2<SBasis> > cat, alcat;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {

      double t = handles[0][0] / 5.;
  
      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      cairo_pw_d2(cr, portion(cat, 0, t));
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
      draw_axis(cr, cat, 0, Point(0, 450), t);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      draw_axis(cr, cat, 1, Point(450, 0), t);
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0., 0., 0., 1);
      draw_line_seg(cr, cat(t), Point(cat(t)[0], 450));
      draw_line_seg(cr, cat(t), Point(450, cat(t)[1]));
      cairo_stroke(cr);

      *notify << cat.segN(t) << " / " << cat.size();

      /*cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,alcat);
      cairo_stroke(cr);
      *notify << "pieces = " << alcat.size() << ";\n";
*/
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
