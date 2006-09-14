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
#include "path.h"
#include "path-builder.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

unsigned total_pieces;

void draw_md_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::PathBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_path(cr, pb.peek());
}


class Clothoid: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);
        
        cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
        double a0 = ((handles[0][0]-width/4)*2)/width;
        double a1 = ((handles[1][0]-width/4)*2)/width;
        *notify << "[" << a0 << ", " << a1 << "]";
        
        multidim_sbasis<2> B;
        BezOrd bo = BezOrd(a0*6,a1*6);
        SBasis t2 = BezOrd(0,1);
        t2 = t2*t2;
        B[0] = compose(cos(bo,6), t2);
        B[1] = compose(sin(bo,6), t2);
        for(int dim = 0; dim < 2; dim++) {
            B[dim] = integral(B[dim]);
            B[dim] -= B[dim](0);
            B[dim] = 200 + 300*B[dim];
        }
        draw_md_sb(cr, B);
    }
};

int main(int argc, char **argv) {
    handles.push_back(Geom::Point(100, 400));
    handles.push_back(Geom::Point(400, 400));
    
    init(argc, argv, "clothoid", new Clothoid());
    
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
