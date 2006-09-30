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

template <typename T>
void shift(T &a, T &b, T const &c) {
    a = b;
    b = c;
}
template <typename T>
void shift(T &a, T &b, T &c, T const &d) {
    a = b;
    b = c;
    c = d;
}

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
        Geom::ArrangementBuilder pb;
        subpath_from_sbasis(pb, B, 0.1);
        Geom::Arrangement p = pb.peek();
        cairo_arrangement(cr, p);
        cairo_stroke(cr);
        
        multidim_sbasis<2> m;
        multidim_sbasis<2> dB = derivative(B);
        multidim_sbasis<2> ddB = derivative(dB);
        multidim_sbasis<2> dddB = derivative(ddB);
        
        Geom::Point pt = point_at(B, 0);
        Geom::Point tang = point_at(dB, 0);
        Geom::Point dtang = point_at(ddB, 0);
        Geom::Point ddtang = point_at(dddB, 0);
        double t = 1;
        for(int dim = 0; dim < 2; dim++) {
            m[dim] = BezOrd(pt[dim],pt[dim]+tang[dim]);
            m[dim] += (1./2)*BezOrd(0, 1)*BezOrd(0, 1*dtang[dim]);
            m[dim] += (1./6)*BezOrd(0, 1)*BezOrd(0, 1)*BezOrd(0, ddtang[dim]);
        }
        
        double lo = 0, hi = 1;
        double eps = 5;
        while(hi - lo > 0.0001) {
            double mid = (hi + lo)/2;
            //double Bmid = (Bhi + Blo)/2;
            
            m = truncate(compose(B, BezOrd(0, mid)), 2);
            // perform golden section search
            double best_f = 0, best_x = 1;
            for(int n = 2; n < 4; n++) {
            Geom::Point m_pt = point_at(m, double(n)/6);
            double x0 = 0, x3 = 1.; // just a guess!
            const double R = 0.61803399;
            const double C = 1 - R;
            double x1 = C*x0 + R*x3;
            double x2 = C*x1 + R*x3;
            double f1 = L2(point_at(B, x1) - m_pt);
            double f2 = L2(point_at(B, x2) - m_pt);
            while(fabs(x3 - x0) > 1e-3*(fabs(x1) + fabs(x2))) {
                if(f2 < f1) {
                    shift(x0, x1, x2, R*x1 + C*x3);
                    shift(f1, f2, L2(point_at(B, x2) - m_pt));
                } else {
                    shift(x3, x2, x1, R*x2 + C*x0);
                    shift(f2, f1, L2(point_at(B, x1) - m_pt));

                }
                std::cout << x0 << "," 
                          << x1 << ","
                          << x2 << ","
                          << x3 << ","
                          << std::endl;
            }
            if(f2 < f1) {
                f1 = f2;
                x1 = x2;
            }
            if(f1 > best_f) {
                best_f = f1;
                best_x = x1;
            }
            }
            std::cout << mid << ":" << best_x << "->" << best_f << std::endl;
            //draw_cross(cr, point_at(B, x1));
            
            if(best_f > eps) {
                hi = mid;
            } else {
                lo = mid;
            }
        }
        std::cout << std::endl;
        //draw_cross(cr, point_at(B, hi));
        draw_circ(cr, point_at(m, hi));
        {
            Geom::ArrangementBuilder pb;
            subpath_from_sbasis(pb, m, 0.1);
            Geom::Arrangement p = pb.peek();
            cairo_arrangement(cr, p);
        }
        
        /*m = truncate(compose(B, BezOrd(0, hi*2)), 2);
        {
            Geom::ArrangementBuilder pb;
            subpath_from_sbasis(pb, m, 0.1);
            Geom::Arrangement p = pb.peek();
            cairo_arrangement(cr, p);
            }*/
       
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
