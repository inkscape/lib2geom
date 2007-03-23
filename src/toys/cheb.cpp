#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"
#include "solver.h"
#include "nearestpoint.cpp"
#include "sbasis-poly.h"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "choose.h"
#include "convex-cover.h"

#include "path2.h"
#include "path-cairo.h"

#include <iterator>
#include "translate.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

SBasis cheb(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(Linear(1,1));
        basis.push_back(Linear(0,1));
    }
    for(int i = basis.size(); i <= n; i++) {
        basis.push_back(Linear(0,2)*basis[i-1] - basis[i-2]);
    }
    
    return basis[n];
}

SBasis cheb01(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(Linear(1,1));
        basis.push_back(Linear(-1,1));
    }
    for(int i = basis.size(); i <= n; i++) {
        basis.push_back(2*(Linear(0,2)*basis[i-1] - basis[i-2]));
    }
    
    return basis[n];
}

class Sb1d: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        D2<SBasis> B;
        B[0] = Linear(width/4, 3*width/4);
        for(unsigned i = 0;  i < 10; i++) {
            //B[1] = cheb(i);
            B[1] = compose(cheb(i), Linear(-1,1));
            //B[1] = cheb01(i);
            *notify << "cheb(" << i << ") = "
                    << B[1]
//sbasis_to_poly(B[1]) 
                    << std::endl;
            Geom::Path2::Path pb;
            B[1] = SBasis(Linear(2*width/4)) - (width/4)*B[1];
            pb.append(B);
            cairo_path(cr, pb);
            
            cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
            cairo_stroke(cr);
        }
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "chebyshev polynomials", new Sb1d());
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
