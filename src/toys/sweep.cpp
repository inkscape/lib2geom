#include "d2.h"
#include "d2.cpp"
#include "sbasis.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

#define SIZE 4

class Sweep: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B1 = handles_to_sbasis<SIZE-1>(handles.begin());
        D2<SBasis> B2 = handles_to_sbasis<SIZE-1>(handles.begin()+SIZE);
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));
        B.concat(Piecewise<D2<SBasis> >(B2));
        std::vector<Point> e;
        std::vector<Piecewise<D2<SBasis> > > s;
        s.push_back(derivative(B));
        for(int j = 0; j < 5; j++) s.push_back(derivative(s.back()));
        for(int j = 0; j <= 5; j++) {
            for(unsigned d = 0; d < 2; d++) {
                std::vector<double> r = roots(make_cuts_independant(s[j])[d]);
                for(int k = 0; k < r.size(); k++) e.push_back(B.valueAt(r[k]));
            }
        }
        for(int i = 0; i < e.size(); i++) draw_cross(cr, e[i]);
        
             cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      //cairo_md_sb(cr, B1);
      cairo_pw_d2(cr, B);
      cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    Sweep () {
        for(int i = 0; i < 2*SIZE; i++)
            handles.push_back(Geom::Point(150+uniform()*300,150+uniform()*300));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sweep());
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
