#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "s-basis.h"
#include "interactive-bits.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"
#include "path-builder.h"
#include "translate.h"
#include "translate-ops.h"
#include "solver.h"
#include <iterator>
#include "nearestpoint.cpp"
#include "sbasis-poly.h"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "choose.h"
#include "convex-cover.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

SBasis cheb(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(BezOrd(1,1));
        basis.push_back(BezOrd(0,1));
    }
    for(int i = basis.size(); i <= n; i++) {
        basis.push_back(BezOrd(0,2)*basis[i-1] - basis[i-2]);
    }
    
    return basis[n];
}

SBasis cheb01(unsigned n) {
    static std::vector<SBasis> basis;
    if(basis.empty()) {
        basis.push_back(BezOrd(1,1));
        basis.push_back(BezOrd(-1,1));
    }
    for(int i = basis.size(); i <= n; i++) {
        basis.push_back(2*(BezOrd(0,2)*basis[i-1] - basis[i-2]));
    }
    
    return basis[n];
}

class Sb1d: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        multidim_sbasis<2> B;
        B[0] = BezOrd(width/4, 3*width/4);
        for(unsigned i = 0;  i < 10; i++) {
            //B[1] = cheb(i);
            B[1] = compose(cheb(i), BezOrd(-1,1));
            //B[1] = cheb01(i);
            *notify << "cheb(" << i << ") = "
                    << B[1]
//sbasis_to_poly(B[1]) 
                    << std::endl;
            Geom::PathSetBuilder pb;
            B[1] = SBasis(BezOrd(2*width/4)) - (width/4)*B[1];
            subpath_from_sbasis(pb, B, 0.001);
            Geom::PathSet p = pb.peek();
            cairo_PathSet(cr, p);
            
            cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
            cairo_stroke(cr);
        }
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
