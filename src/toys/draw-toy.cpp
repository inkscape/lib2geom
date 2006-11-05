#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

//Not sure how much of this is required, but hey
#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include <iterator>
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
#include "nearestpoint.cpp"
#include "sbasis-poly.h"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "choose.h"
#include "convex-cover.h"

#include "toy-framework.cpp"

/*2geom crit:
L2 = pythagorean distance. Its ok to call it L2 internally, but have a distance alias.  Maybe there is one.

path convenience constructors - eg, spline, poly, etc.

Perhaps we should split the docs into useage/complexity levels.  eg, "basic", "advanced".  Not sure if there is a good way to doxy this.
(L0, L1, L2 would be put in the advanced section)
*/

class MyToy: public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::ArrangementBuilder builder;
        if(handles.size() > 0) {
            builder.start_subpath(handles[0]);
            for(int i = 1; i < handles.size(); i++) {
                builder.push_line(handles[i]);
            }
        }
        cairo_arrangement(cr, builder.peek());
    }
    void mouse_pressed(GdkEventButton* e) {
        Geom::Point mouse(e->x, e->y);
        int close_i = 0;
        float close_d = 1000;
        for(int i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < close_d) {
                close_d = Geom::L2(mouse - handles[i]);
                close_i = i;
            }
        }
        if(close_d < 10) {if(e->button==3) handles.erase(handles.begin() += close_i);}
                     else if(e->button==1) handles.push_back(mouse);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "draw-toy", new MyToy());
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
