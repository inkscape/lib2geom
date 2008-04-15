#include "piecewise.h"
#include "sbasis.h"
#include "sbasis-math.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"

#include <vector>

using namespace Geom;
using namespace std;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = p[i];
        cairo_md_sb(cr, B);
    }
}

void cairo_horiz(cairo_t *cr, double y, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, p[i], y);
        cairo_rel_line_to(cr, 0, 10);
    }
}

void cairo_vert(cairo_t *cr, double x, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, x, p[i]);
        cairo_rel_line_to(cr, 10, 0);
    }
}

Piecewise<SBasis> log(Interval in) {
    Piecewise<SBasis> I = integral(Geom::reciprocal(Linear(in[0], in[1])));
    return I + Piecewise<SBasis> (-I.segs[0][0] + log(in[0]));
}

Piecewise<SBasis> xlogx(Interval in) {
    Piecewise<SBasis> I = integral(log(in) + Piecewise<SBasis>(1));
    return I + Piecewise<SBasis> (-I.segs[0][0] + in[0]*log(in[0]));
}

class PwToy: public Toy {

    unsigned segs, handles_per_curve, curves;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_set_line_width (cr, 1);
        
        Piecewise<SBasis> pws;
        
        //pws = Geom::cos(Linear(0,100)) + 3;
        //pws = Geom::sqrt(Linear(0,100));
        //pws = log(Interval(1,8));
        //Piecewise<SBasis> l(Linear(-100,100));
        //Piecewise<SBasis> one(Linear(1,1));
        //pws = Geom::reciprocal(l*l + one)*l + l;
        pws = xlogx(Interval(0.5,3));
        //pws = Geom::reciprocal(pws);
        //pws = -integral(Geom::reciprocal(Linear(1,2)))*Piecewise<SBasis>(Linear(1,2));
        
        pws = -pws*width/4 + width/2;
        pws.scaleDomain(width/2);
        pws.offsetDomain(width/4);

        cairo_pw(cr, pws);

        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0., 0., .5, 1.);
        cairo_horiz(cr, 500, pws.cuts);
        cairo_stroke(cr);

        *notify << "total pieces: " << pws.size();

        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
    int should_draw_bounds() { return 2; }
    public:
    PwToy () {
        segs = 3;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new PwToy());
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
