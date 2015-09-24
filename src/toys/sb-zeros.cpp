#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using namespace Geom;

#define SIZE 4

class SBZeros: public Toy {
    PointSetHandle pB1, pB2;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        D2<SBasis> B1 = pB1.asBezier();
        D2<SBasis> B2 = pB2.asBezier();
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));
        B.concat(Piecewise<D2<SBasis> >(B2));
        std::vector<Point> e;
        std::vector<Piecewise<D2<SBasis> > > s;
        s.push_back(derivative(B));
        for(int j = 0; j < 5; j++) s.push_back(derivative(s.back()));
        for(int j = 0; j <= 5; j++) {
            for(unsigned d = 0; d < 2; d++) {
                std::vector<double> r = roots(make_cuts_independent(s[j])[d]);
                for(unsigned k = 0; k < r.size(); k++) e.push_back(B.valueAt(r[k]));
            }
        }
        for(unsigned i = 0; i < e.size(); i++) draw_cross(cr, e[i]);
        
        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_pw_d2_sb(cr, B);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    SBZeros () {
        for(unsigned i = 0; i < SIZE; i++)
            pB1.push_back(150+uniform()*300,150+uniform()*300);
        for(unsigned i = 0; i < SIZE; i++)
            pB2.push_back(150+uniform()*300,150+uniform()*300);
        handles.push_back(&pB1);
        handles.push_back(&pB2);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SBZeros());
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
