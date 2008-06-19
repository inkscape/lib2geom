#include "d2.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"

#include "path-cairo.h"
#include "toy-framework-2.h"

#include <vector>
using std::vector;
using namespace Geom;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double space=10){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 2;
    for( double t = M.cuts.front(); t < M.cuts.back(); t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_pw_d2(cr, M);
    cairo_stroke(cr);
}

#define SIZE 4

class LengthTester: public Toy {
public:
    PointSetHandle b1_handle;
    PointSetHandle b2_handle;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
    
        D2<SBasis> B1 = b1_handle.asBezier();
        D2<SBasis> B2 = b2_handle.asBezier();
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));
        B.concat(Piecewise<D2<SBasis> >(B2));
      
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

        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    LengthTester(){
        for(int i = 0; i < SIZE; i++) {
            b1_handle.push_back(150+uniform()*300,150+uniform()*300);
            b2_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
        b1_handle.pts[0] = Geom::Point(150,150);
        b1_handle.pts[1] = Geom::Point(150,150);
        b1_handle.pts[2] = Geom::Point(150,450);
        b1_handle.pts[3] = Geom::Point(450,150);
        handles.push_back(&b1_handle);
        handles.push_back(&b2_handle);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new LengthTester);
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
