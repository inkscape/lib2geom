#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "s-basis-2d.h"
#include "d2.h"

#include "path-cairo.h"

#include <map>
#include <iterator>

#include "toy-framework.h"

using std::vector;
using namespace Geom;

static void dot_plot(cairo_t *cr, D2<Piecewise<SBasis> > const &M, double space=10){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    double dt=space;
    double t = M[0].cuts.front();
    while (t < M[0].cuts.back()){
        draw_handle(cr, M(t));
        t += dt;
    }
    cairo_d2_pw(cr, M);
    cairo_stroke(cr);
}

#define SIZE 4

class LengthTester: public Toy {

    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
    
      D2<SBasis> B1 = handles_to_sbasis<SIZE-1>(handles.begin());
      D2<SBasis> B2 = handles_to_sbasis<SIZE-1>(handles.begin()+SIZE);
      D2<Piecewise<SBasis> >B;
      for ( int dim=0; dim<2; dim++ ){
          B[dim].cuts.push_back(0);
          B[dim].push(B1[dim],1);
          B[dim].push(B2[dim],2);
      }
      
      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      //cairo_md_sb(cr, B1);
      cairo_d2_pw(cr, B);
      cairo_stroke(cr);

//       D2<Piecewise<SBasis> > uniform_B1 = arc_length_parametrization(B1,3,1e-7);
//       cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
//       dot_plot(cr,uniform_B1);
//       cairo_stroke(cr);

      D2<Piecewise<SBasis> > uniform_B = arc_length_parametrization(B,3,1e-7);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,uniform_B);
      cairo_stroke(cr);

      Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    LengthTester(){
        if(handles.empty()) {
            for(int i = 0; i < 2*SIZE; i++)
                handles.push_back(Geom::Point(150+uniform()*300,150+uniform()*300));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, argv[0], new LengthTester);
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
