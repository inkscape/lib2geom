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

#include "toy-framework.cpp"

using std::string;
using std::vector;
using std::complex;

extern unsigned total_steps, total_subs;

class RootFinderComparer: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        std::vector<Geom::Point> trans;
        trans.resize(handles.size());
        for(unsigned i = 0; i < handles.size(); i++) {
            trans[i] = handles[i] - Geom::Point(0, 3*height/4);
        }
        
        std::vector<double> solutions;
        clock_t end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            total_steps = 0;
            total_subs = 0;
            solutions.resize(6);
            FindRoots(&trans[0], 5, &solutions[0], 0);
            iterations++;
        }
        *notify << "original time = " << 1./iterations << std::endl;
        std::cout << "original: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
        
        multidim_sbasis<2> test_sb = bezier_to_sbasis<2, 5>(handles.begin());
        double lo, hi;
        bounds(test_sb[1], lo, hi);
        cairo_move_to(cr, test_sb[0](0), lo);
        cairo_line_to(cr, test_sb[0](1), lo);
        cairo_move_to(cr, test_sb[0](0), hi);
        cairo_line_to(cr, test_sb[0](1), hi);
        cairo_stroke(cr);
        *notify << "sb bounds = "<<lo << ", " <<hi<<std::endl;
        Poly ply = sbasis_to_poly(test_sb[1]);
        ply = Poly(3*height/4) - ply;
    
        vector<complex<double> > complex_solutions;
        complex_solutions = solve(ply);
        std::cout << "gsl: ";
        std::copy(complex_solutions.begin(), complex_solutions.end(), std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
        std::cout << std::endl;

        /*complex_solutions = Laguerre(ply);
        std::cout << "laguerre: ";
        std::copy(complex_solutions.begin(), complex_solutions.end(), std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
        std::cout << std::endl;*/
    
        /*complex_solutions = DK(ply);
        std::cout << "dk: ";
        std::copy(complex_solutions.begin(), complex_solutions.end(), std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
        std::cout << std::endl;*/
    
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solve(ply);
            iterations++;
        }
        *notify << "gsl poly " << ", time = " << 1./iterations << std::endl;
        
    #if LAGUERRE_TEST
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            Laguerre(ply);
            iterations++;
        }
        complex_solutions = Laguerre(ply);
    
        *notify << "Laguerre poly " << ", time = " << 1./iterations << std::endl;
    #endif    
    
    #define SBASIS_SUBDIV_TEST 1
    #if SBASIS_SUBDIV_TEST
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            roots( -test_sb[1] + BezOrd(3*height/4));
            iterations++;
        }
    
        *notify << "sbasis subdivision " << ", time = " << 1./iterations << std::endl;
    #endif    
    
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        /*while(end_t > clock()) {
            total_steps = 0;
            total_subs = 0;
            solutions.resize(0);
            find_parametric_bezier_roots(&trans[0], 5, solutions, 0);
            iterations++;
        }
        *notify << "subdivision " << total_steps << ", " << total_subs <<  ", time = " << 1./iterations << std::endl;*/
        solutions = roots( -test_sb[1] + BezOrd(3*height/4));
        std::cout << "sbasis sub: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
        
        for(unsigned i = 0; i < solutions.size(); i++) {
            double x = test_sb[0](solutions[i]);
            draw_cross(cr, Geom::Point(x, 3*height/4));
            
        }
    /*
        for(unsigned i = 0; i < complex_solutions.size(); i++) {
            if(complex_solutions[i].imag() == 0) {
                double x = test_sb[0](complex_solutions[i].real());
                draw_handle(cr, Geom::Point(x, 3*height/4));
            }
        }*/

        *notify << "found " << solutions.size() << "solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
            
        multidim_sbasis<2> B = bezier_to_sbasis<2, 5>(handles.begin());
        Geom::PathBuilder pb;
        subpath_from_sbasis(pb, B, 0.1);
        Geom::Path p = pb.peek();//*Geom::translate(1,1);
        cairo_path(cr, p);
    }
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
