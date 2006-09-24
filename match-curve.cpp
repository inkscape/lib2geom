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
#include "translate.h"
#include "translate-ops.h"
#include "solver.h"
#include "nearestpoint.cpp"
#include "sbasis-poly.h"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "point-fns.h"

#include "toy-framework.cpp"

#define ZROOTS_TEST 0
#if ZROOTS_TEST
#include "zroots.c"
#endif

using std::string;
using std::vector;
using std::complex;

//#define HAVE_GSL

extern unsigned total_steps, total_subs;

class RootFinderComparer: public Toy {
public:
    double timer_precision;
    double units;
    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        std::vector<Geom::Point> trans;
        trans.resize(handles.size());
        for(unsigned i = 0; i < handles.size(); i++) {
            trans[i] = handles[i] - Geom::Point(0, 3*width/4);
        }
        
        std::vector<double> solutions;
        
        multidim_sbasis<2> test_sb = bezier_to_sbasis<2, 5>(handles.begin());

    
        multidim_sbasis<2> B = bezier_to_sbasis<2, 5>(handles.begin());
        Geom::PathBuilder pb;
        subpath_from_sbasis(pb, B, 0.1);
        Geom::Path p = pb.peek();
        cairo_path(cr, p);
        cairo_stroke(cr);
        
        multidim_sbasis<2> m;
        Geom::Point pt = point_at(B, 0);
        Geom::Point tang = point_at(derivative(B), 0);
        Geom::Point dtang = point_at(derivative(derivative(B)), 0);
        Geom::Point ddtang = point_at(derivative(derivative(derivative(B))), 0);
        double t = 1;
        for(int dim = 0; dim < 2; dim++) {
            m[dim] = BezOrd(pt[dim],pt[dim]+t*tang[dim]);
            m[dim] += (1./2)*BezOrd(0, t)*BezOrd(0, t*dtang[dim]);
            m[dim] += (1./6)*BezOrd(0, t)*BezOrd(0, t)*BezOrd(0, t*ddtang[dim]);
        }
        {
            Geom::PathBuilder pb;
            subpath_from_sbasis(pb, m, 0.1);
            Geom::Path p = pb.peek();
            cairo_path(cr, p);
        }
        cairo_stroke(cr);
    }
    RootFinderComparer() : timer_precision(0.1), units(1e6) // microseconds
 {}
};

int main(int argc, char **argv) {
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));

    init(argc, argv, "root-finder-comparer", new RootFinderComparer());

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
