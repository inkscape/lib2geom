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
#include "path-builder.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

void draw_md_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::PathBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_path(cr, pb.peek());
}

class Gear: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        Geom::Point centre(width/2, height/2);
        double angle = atan2(handles[0] - centre);
        double base_radius = L2(handles[0] - centre);
        double r = L2(handles[1] - centre);
        handles[1] = base_radius*unit_vector(handles[1] - centre) + centre;
        double angle1 = atan2(handles[1] - centre);
        for(int i = 0; i < handles.size(); i++) {
            draw_circ(cr, handles[i]);
        }
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        double dominant_dim = std::max(width,height);
        double minor_dim = std::min(width,height);

        // draw cross hairs
        for(int i = 1; i < 2; i++) {
            cairo_move_to(cr, centre[0]-minor_dim/4, centre[1]);
            cairo_line_to(cr, centre[0]+minor_dim/4, centre[1]);
            cairo_move_to(cr, centre[0], centre[1]-minor_dim/4);
            cairo_line_to(cr, centre[0], centre[1]+minor_dim/4);
        }
        cairo_stroke(cr);
        
        // arc
        multidim_sbasis<2> B;
        BezOrd bo = BezOrd(0,angle);
        
        // involute
        multidim_sbasis<2> I;
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        I = B - BezOrd(0,1) * derivative(B);
        B = base_radius*B + centre;
        draw_md_sb(cr, B);
        I = base_radius*I + centre;
        draw_md_sb(cr, I);
        
        // line
        multidim_sbasis<2> A;
        handles[2] = point_at(I, angle1/angle);
        for(int dim = 0; dim < 2; dim++)
            A[dim] = BezOrd(handles[1][dim], handles[2][dim]);
        draw_md_sb(cr, A);
        
        // draw base radius
        /*cairo_new_sub_path(cr);
        double base_radius = L2(handles[0] - centre);
        cairo_arc(cr, centre[0], centre[1], base_radius, 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);*/
        cairo_stroke(cr);
        
        *notify << "base radius = " << base_radius;
    }
};

int main(int argc, char **argv) {
    for(int i = 0; i < 3; i++)
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    
    screen_lines = false;

    init(argc, argv, "gear", new Gear());

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
