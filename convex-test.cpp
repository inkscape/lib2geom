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
#include <iterator>
#include "s-basis.h"
#include "interactive-bits.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "path.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"
#include "path-builder.h"
#include "translate.h"
#include "translate-ops.h"
#include "s-basis-2d.h"
#include "path-builder.h"
#include "convex-cover.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

class ConvexTest: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    for(int i = 1; i < 4; i+=2) {
        cairo_move_to(cr, 0, i*width/4);
        cairo_line_to(cr, width, i*width/4);
        cairo_move_to(cr, i*width/4, 0);
        cairo_line_to(cr, i*width/4, width);
    }

    if(handles.empty()) {
	    for(int i = 0; i < 100; i++){
		    Geom::Point c(width/2, width/2);
		    handles.push_back(Geom::Point(uniform()*(uniform()-0.5)*width/2,
						  uniform()*(uniform()-0.5)*height/2) + c);
	    }
    }
    
    clock_t end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    unsigned iterations = 0;
    while(end_t > clock()) {
	Geom::ConvexHull ch(handles);
        iterations++;
    }
    *notify << "constructor time = " << 1000*0.1/iterations << std::endl;
    Geom::ConvexHull ch(handles);

    std::vector<Geom::Point> h;
    h.push_back(handles[0]);
    h.push_back(handles[1]);

    end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    iterations = 0;
    while(end_t > clock()) {
        Geom::ConvexHull ch(handles);
        for(int i = 2; i < 100; i++)
            ch.merge(handles[i]);
        iterations++;
    }
    *notify << "merge time = " << 1000*0.1/iterations << std::endl;

    //ch.merge(old_mouse_point);

    //assert(ch.is_clockwise());
    if(ch.contains_point(old_mouse_point))
        *notify << "mouse in convex" << std::endl;
    cairo_move_to(cr, ch.boundary.back());
    for(int i = 0; i < ch.boundary.size(); i++) {
        cairo_line_to(cr, ch.boundary[i]);
    }
}
};

int main(int argc, char **argv) {
    numbers = false;
    init(argc, argv, "convex-test", new ConvexTest());

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
