#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "s-basis-2d.h"
#include "d2.h"

#include "path-cairo.h"

#include <map>
#include <iterator>
#include "translate.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

static void dot_plot(cairo_t *cr, D2<Piecewise<SBasis> > const &M, double space=10){
    double t = M[0].cuts.front();
    while (t < M[0].cuts.back()){
        draw_handle(cr, M(t));
        t += space;
    }
    vector<double> cuts;
    vector<D2<SBasis> > sections = sectionize(M, cuts); 
    for (int i=0; i < sections.size(); i++){
        cairo_md_sb(cr, sections[i]);
    }
    cairo_stroke(cr);
}

#define SIZE 5

class LengthTester: public Toy {

    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
    
      D2<SBasis> B = bezier_to_sbasis<SIZE-1>(handles.begin());
      D2<Piecewise<SBasis> >d2B;
      d2B[0]=Piecewise<SBasis>(B[0]);
      d2B[1]=Piecewise<SBasis>(B[1]);

      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      cairo_md_sb(cr, B);
      cairo_stroke(cr);

      D2<Piecewise<SBasis> > U = arc_length_parametrization(B,3,1e-7);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,U);
      cairo_stroke(cr);

      Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    LengthTester(){
        if(handles.empty()) {
            for(int i = 0; i < SIZE; i++)
                handles.push_back(Geom::Point(150+uniform()*300,150+uniform()*300));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new LengthTester);
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
