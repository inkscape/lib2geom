#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "solver.h"
#include "nearestpoint.cpp"
#include "sbasis-poly.h"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "choose.h"
#include "convex-cover.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

#define ZROOTS_TEST 0
#if ZROOTS_TEST
#include "zroots.c"
#endif

using std::vector;
using std::complex;
using namespace Geom;

//#define HAVE_GSL

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
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            solutions.resize(6);
            FindRoots(&trans[0], 5, &solutions[0], 0);
            iterations++;
        }
        *notify << "original time = " << timer_precision*units/iterations << std::endl;
#if 0
        std::cout << "original: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
#endif
        
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
        ply = Poly(3*width/4) - ply;
#ifdef HAVE_GSL    
        vector<complex<double> > complex_solutions;
        complex_solutions = solve(ply);
#if 0
        std::cout << "gsl: ";
        std::copy(complex_solutions.begin(), complex_solutions.end(), std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
        std::cout << std::endl;
#endif
#endif
        
#if ZROOTS_TEST
        fcomplex a[ply.size()];
        for(int i = 0; i < ply.size(); i++) {
            a[i] = ply[i];
        }
        //copy(a, a+ply.size(), ply.begin());
        fcomplex rts[ply.size()];
        
        zroots(a, ply.size(), rts, true);
        for(int i = 0; i < ply.size(); i++) {
            if(! a[i].imag())
                solutions[i] = a[i].real();
        }
#endif

        // Base loop to remove overhead
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            iterations++;
        }
        double overhead = timer_precision*units/iterations;

#ifdef HAVE_GSL    
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solve(ply);
            iterations++;
        }
        *notify << "gsl poly " << ", time = " << timer_precision*units/iterations-overhead << std::endl;
      #endif
  
    #if ZROOTS_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            zroots(a, ply.size(), rts, false);
            iterations++;
        }
    
        *notify << "zroots poly " << ", time = " << timer_precision*units/iterations-overhead << std::endl;
    #endif    
    
    #if LAGUERRE_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            Laguerre(ply);
            iterations++;
        }
        complex_solutions = Laguerre(ply);
    
        *notify << "Laguerre poly " << ", time = " << timer_precision*units/iterations-overhead << std::endl;
    #endif    
    
    #define SBASIS_SUBDIV_TEST 0
    #if SBASIS_SUBDIV_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            roots( -test_sb[1] + BezOrd(3*width/4));
            iterations++;
        }
    
        *notify << "sbasis subdivision " << ", time = " << timer_precision*units/iterations-overhead << std::endl;
    #endif    
    
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solutions.resize(0);
            find_parametric_bezier_roots(&trans[0], 5, solutions, 0);
            iterations++;
        }
        *notify << "solver parametric time = " << timer_precision*units/iterations-overhead << std::endl;
    
        double ys[trans.size()];
        for(int i = 0; i < trans.size(); i++) {
            ys[i] = trans[i][1];
            double x = double(i)/(trans.size()-1);
            x = (1-x)*width/4 + x*width*3/4;
            draw_handle(cr, Geom::Point(x, 3*width/4 + ys[i]));
        }
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solutions.resize(0);
            find_bernstein_roots(ys, 5, solutions, 0);
            iterations++;
        }
        *notify << "solver 1d subdivision slns" << solutions.size() << ", time = " << timer_precision*units/iterations-overhead << std::endl;
        solutions = roots( -test_sb[1] + BezOrd(3*width/4));
#if 0
        std::cout << "sbasis sub: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
#endif        
        for(unsigned i = 0; i < solutions.size(); i++) {
            double x = test_sb[0](solutions[i]);
            draw_cross(cr, Geom::Point(x, 3*width/4));
            
        }
    /*
        for(unsigned i = 0; i < complex_solutions.size(); i++) {
            if(complex_solutions[i].imag() == 0) {
                double x = test_sb[0](complex_solutions[i].real());
                draw_handle(cr, Geom::Point(x, 3*width/4));
            }
        }*/

        *notify << "found " << solutions.size() << "solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
            
        multidim_sbasis<2> B = bezier_to_sbasis<2, 5>(handles.begin());
        Geom::PathSetBuilder pb;
        subpath_from_sbasis(pb, B, 0.1);
        Geom::PathSet p = pb.peek();//*Geom::translate(1,1);
        cairo_PathSet(cr, p);
        
        B[0] = BezOrd(width/4, 3*width/4);
        cairo_md_sb(cr, B);
        Toy::draw(cr, notify, width, height, save);
    }
    RootFinderComparer() : timer_precision(0.1), units(1e6) // microseconds
    {
        for(int i = 0; i < 6; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }
};

int main(int argc, char **argv) {
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
