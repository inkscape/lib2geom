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

    int n = 30;

    if(handles.empty()) {
	    for(int i = 0; i < n; i++){
		    Geom::Point c(width/2, width/2);
		    handles.push_back(Geom::Point(uniform()*(uniform()-0.5)*width/2,
						  uniform()*(uniform()-0.5)*height/2) + c);
	    }
    }
    
    std::vector<Geom::Point> h1, h2;
    for(int i = 0; i < 15; i++) {
        h1.push_back(handles[i]);
        h2.push_back(handles[i + 15]);
    }

    clock_t end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    unsigned iterations = 0;
    while(end_t > clock()) {
	Geom::ConvexHull ch(handles);
        iterations++;
    }
    *notify << "constructor time = " << 1000*0.1/iterations << std::endl;
    Geom::ConvexHull ch1(h1);
    Geom::ConvexHull ch2(h2);

    end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    iterations = 0;
    while(end_t > clock()) {
        graham_merge(ch1, ch2);
        iterations++;
    }
    *notify << "graham merge time = " << 1000*0.1/iterations << std::endl;

    end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    iterations = 0;
    while(end_t > clock()) {
        merge(ch1, ch2);
        iterations++;
    }
    *notify << "merge time = " << 1000*0.1/iterations << std::endl;
    {
    Geom::ConvexHull gm = graham_merge(ch1, ch2);
    Geom::Point offset = Geom::Point(100, 0);

    cairo_set_line_width (cr, 2);
    if(gm.boundary.size() > 0) {
        cairo_move_to(cr, gm.boundary.back() + offset);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        for(int i = 0; i < gm.boundary.size(); i++) {
            cairo_line_to(cr, gm.boundary[i] + offset);
            draw_number(cr, gm.boundary[i] + offset, i);
        }
    }
    cairo_stroke(cr);
    }
    Geom::ConvexHull m = merge(ch1, ch2);
    //ch.merge(old_mouse_point);

    //assert(ch.is_clockwise());
    if(m.contains_point(old_mouse_point))
        *notify << "mouse in convex" << std::endl;

    Geom::Point offset = Geom::Point(0, -200);

    cairo_set_line_width (cr, 2);
    if(m.boundary.size() > 0) {
        cairo_move_to(cr, m.boundary.back() + offset);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        for(int i = 0; i < m.boundary.size(); i++) {
            cairo_line_to(cr, m.boundary[i] + offset);
            draw_number(cr, m.boundary[i] + offset, i);
        }
    }
    cairo_stroke(cr);
    cairo_set_line_width (cr, 1);

    cairo_set_source_rgba(cr, 0., 0., 1., 0.5);
    std::vector<Geom::Point> bs = bridge_points(ch1, ch2);
    for(int i = 0; i < bs.size(); i+=2) {
        cairo_move_to(cr, bs[i]);
        cairo_line_to(cr, bs[i + 1]);
        draw_number(cr, (bs[i] + bs[i + 1]) / 2, i / 2);
    }
    cairo_stroke(cr);

    cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
    cairo_move_to(cr, ch1.boundary.back());
    for(int i = 0; i < ch1.boundary.size(); i++) {
        cairo_line_to(cr, ch1.boundary[i]);
    }
    cairo_stroke(cr);

    cairo_move_to(cr, ch2.boundary.back());
    cairo_set_source_rgba (cr, 0., 1., 0, 0.8);
    for(int i = 0; i < ch2.boundary.size(); i++) {
        cairo_line_to(cr, ch2.boundary[i]);
    }
    cairo_stroke(cr);
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
