#include "basic-intersection.h"
#include "d2.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"

using std::vector;
using namespace Geom;

cairo_t *g_cr = 0;
const double eps = 0.1;

class PairShuttle: public Toy {
unsigned A_bez_ord;
unsigned B_bez_ord;
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    D2<SBasis> A = handles_to_sbasis(handles.begin(), A_bez_ord-1);
    cairo_md_sb(cr, A);
    
    D2<SBasis> B = handles_to_sbasis(handles.begin()+A_bez_ord, B_bez_ord-1);
    cairo_md_sb(cr, B);
    vector<double> Asects, Bsects;
    g_cr = cr;
    //if(0) pair_intersect(Asects, Bsects, A, 0, 1, 
    //               B, 0, 1);
    Point P = handles.back();
    bool dom = false;
    cairo_move_to(cr, P);
    for(int i = 0 ; i < 100; i++) {
        D2<SBasis> Dom = A;
        if(dom)
            Dom = B;
        D2<SBasis> dst = Dom - P;
        SBasis dst2 = derivative(dot(dst, dst));
        std::vector<double> rts = roots(dst2);
        if(rts.empty()) break;
        Point closest = Dom(rts[0]);
        for(unsigned i = 1; i < rts.size(); i++) {
            //draw_cross(cr, A(rts[i]));
            if(L2(P - closest) > L2(P - Dom(rts[i])))
                closest = Dom(rts[i]);
        }
        P = closest;
        cairo_line_to(cr, P);
        dom = !dom;
    }
    cairo_stroke(cr);
    
    Toy::draw(cr, notify, width, height, save);
}
public:
    PairShuttle (unsigned A_bez_ord, unsigned B_bez_ord) :
        A_bez_ord(A_bez_ord), B_bez_ord(B_bez_ord) {
    for(unsigned i = 0; i < A_bez_ord + B_bez_ord+1; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {
unsigned A_bez_ord=10;
unsigned B_bez_ord=3;
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    init(argc, argv, new PairShuttle(A_bez_ord, B_bez_ord));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
