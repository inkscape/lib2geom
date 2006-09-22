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
#include <iterator>
#include "multidim-sbasis.h"
#include "path-builder.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

BezOrd z0(0.5,1.);

unsigned total_pieces;

void draw_cb(cairo_t *cr, multidim_sbasis<2> const &B) {
    std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
    cairo_move_to(cr, bez[0]);
    cairo_curve_to(cr, bez[1], bez[2], bez[3]);
}

double sinC(double t) { return t - sin(t);}
double cosC(double t) { return 1 - cos(t);}
double tanC(double t) { return sinC(t) / cosC(t);}

class Conic3: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);
        
        vector<Geom::Point> e_a_h;
        Geom::Point a[2] = {handles[0] - handles[1],
                        handles[2] - handles[1]};
        double angle = Geom::angle_between(a[0], a[1]);
        double len = std::max(Geom::L2(a[0]),
                                Geom::L2(a[1]));
        for(int i = 0; i < 2; i++) 
        a[i] = len*unit_vector(a[i]);
        *notify << "angle = " << angle;
        *notify << " sinC = " << sinC(angle);
        *notify << " cosC = " << cosC(angle);
        *notify << " tanC = " << tanC(angle);
        e_a_h = handles;
        //e_a_h.resize(4);
        
        SBasis one = BezOrd(1, 1);
        multidim_sbasis<2> B;
        double alpha = M_PI;
        SBasis C = cos(BezOrd(0, alpha), 10);
        SBasis S = sin(BezOrd(0, alpha), 10);
        SBasis X(BezOrd(0,alpha));
        SBasis sinC = X - S;
        SBasis cosC = one - C;
        //SBasis tanC = divide(sinC, cosC, 10);
        SBasis Z3 = (1./(sinC(1)))*sinC;
        SBasis Z0 = reverse(Z3);
        SBasis Z2 = (1./(cosC(1)))*cosC - Z3;
        SBasis Z1 = reverse(Z2);
        
        SBasis Z[4] = {Z0, Z1, Z2, Z3};
        
        for(unsigned dim  = 0; dim < 2; dim++) {
            B[dim] = BezOrd(0,0);
            for(unsigned i  = 0; i < 4; i++) {
                B[dim] += e_a_h[i][dim]*Z[i];
            }
        }
        {
            Geom::PathBuilder pb;
            subpath_from_sbasis(pb, B, 1);
            cairo_path(cr, pb.peek());
            cairo_path_handles(cr, pb.peek());
        }
    }
};

int main(int argc, char **argv) {
    handles.push_back(Geom::Point(100, 500));
    handles.push_back(Geom::Point(100, 500 - 200*M_PI/2));
    handles.push_back(Geom::Point(500, 500 - 200*M_PI/2));
    handles.push_back(Geom::Point(500, 500));
    
    init(argc, argv, "conic-3", new Conic3());

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
