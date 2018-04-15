/**
 * efficient and precise flattening of curves.
 * incomplete rewrite (njh)
 */
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

class PreciseFlat: public Toy {
    PointSetHandle hand;
void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
    cairo_set_line_width (cr, 0.5);
    
    D2<SBasis> B = hand.asBezier();
    D2<SBasis> dB = derivative(B);
    D2<SBasis> ddB = derivative(dB);
    cairo_set_source_rgb(cr, 0,0,0);
    cairo_d2_sb(cr, B);
    cairo_stroke(cr);
    
    // draw the longest chord that is no worse than tol from the curve.
    
    Geom::Point st = unit_vector(dB(0));
    double s3 = fabs(dot(hand.pts[2] - hand.pts[0], rot90(st)));
    
    SBasis inflect = dot(dB, rot90(ddB));
    std::vector<double> rts = roots(inflect);
    double f = 3;
    for(double rt : rts) {
        draw_handle(cr, B(rt));
        
        double tp = rt;
        Geom::Point st = unit_vector(dB(tp));
        Geom::Point O = B(tp);
        double s4 = fabs(dot(hand.pts[3] - O, rot90(st)));
        double tf = pow(f/s4, 1./3);
        Geom::Point t1p = B(tp + tf*(1-tp));
        Geom::Point t1m = B(tp - tf*(1-tp));
        cairo_move_to(cr, t1m);
        cairo_line_to(cr, t1p);
        cairo_stroke(cr);
        //std::cout << tp << ", " << t1m << ", " << t1p << std::endl;
    }
    
    cairo_move_to(cr, B(0));
    double t0 = 2*sqrt(f/(3*s3));
    //std::cout << t0 << std::endl;
    cairo_line_to(cr, B(t0));
    cairo_stroke(cr);
    Toy::draw(cr, notify, width, height, save,timer_stream);
}

public:
PreciseFlat () {
    for(unsigned i = 0; i < 4; i++)
        hand.pts.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(&hand);
}

};

int main(int argc, char **argv) {
    init(argc, argv, new PreciseFlat());

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
