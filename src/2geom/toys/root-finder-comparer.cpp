#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/solver.h>
#include <2geom/sbasis-poly.h>
#include <2geom/nearestpoint.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)
#include <2geom/path.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#define ZROOTS_TEST 0
#if ZROOTS_TEST
#include <2geom/zroots.c>
#endif


namespace Geom{
extern void subdiv_sbasis(SBasis const & s,
                   std::vector<double> & roots, 
                   double left, double right);
};

double eval_bernstein(double* w, double t, unsigned N) {
    double Vtemp[2*N];
    //const int degree = N-1;
    for (unsigned i = 0; i < N; i++)
        Vtemp[i] = w[i];

    /* Triangle computation	*/
    const double omt = (1-t);
    //Left[0] = Vtemp[0];
    //Right[degree] = Vtemp[degree];
    double *prev_row = Vtemp;
    double *row = Vtemp + N;
    for (unsigned i = 1; i < N; i++) {
        for (unsigned j = 0; j < N - i; j++) {
            row[j] = omt*prev_row[j] + t*prev_row[j+1];
        }
        //Left[i] = row[0];
        //Right[degree-i] = row[degree-i];
        std::swap(prev_row, row);
    }
    return prev_row[0];
}

#include <vector>
using std::vector;
using namespace Geom;

#define HAVE_GSL
#ifdef HAVE_GSL
#include <complex>
using std::complex;
#endif

class RootFinderComparer: public Toy {
public:
    double timer_precision;
    double units;
    std::string units_string;
    PointSetHandle psh;
    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        std::vector<Geom::Point> trans;
        trans.resize(psh.size());
        for(unsigned i = 0; i < handles.size(); i++) {
            trans[i] = psh.pts[i] - Geom::Point(0, height/2);
        }
        
        std::vector<double> solutions;
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            solutions.resize(6);
            FindRoots(&trans[0], 5, &solutions[0], 0);
            iterations++;
        }
        *notify << "original time = " << timer_precision*units/iterations 
                << units_string << std::endl;
#if 0
        std::cout << "original: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
#endif
        
        D2<SBasis> test_sb = psh.asBezier();//handles_to_sbasis(handles.begin(),5);
        Interval bs = bounds_exact(test_sb[1]);
        cairo_move_to(cr, test_sb[0](0), bs.min());
        cairo_line_to(cr, test_sb[0](1), bs.min());
        cairo_move_to(cr, test_sb[0](0), bs.max());
        cairo_line_to(cr, test_sb[0](1), bs.max());
        cairo_stroke(cr);
        *notify << "sb bounds = "<<bs.min()<< ", " <<bs.max()<<std::endl;
        Poly ply = sbasis_to_poly(test_sb[1]);
        ply = Poly(height/2) - ply;
        cairo_move_to(cr, 0, height/2);
        cairo_line_to(cr, width, height/2);
        cairo_stroke(cr);
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
        for(unsigned i = 0; i < ply.size(); i++) {
            a[i] = ply[i];
        }
        //copy(a, a+ply.size(), ply.begin());
        fcomplex rts[ply.size()];
        
        zroots(a, ply.size(), rts, true);
        for(unsigned i = 0; i < ply.size(); i++) {
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
        *notify << "gsl poly " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
      #endif
  
    #if ZROOTS_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            zroots(a, ply.size(), rts, false);
            iterations++;
        }
    
        *notify << "zroots poly " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
    #endif    
    
    #if LAGUERRE_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            Laguerre(ply);
            iterations++;
        }
        complex_solutions = Laguerre(ply);
    
        *notify << "Laguerre poly " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
    #endif    
    
    #define SBASIS_SUBDIV_TEST 0
    #if SBASIS_SUBDIV_TEST
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            std::vector<double> rts;
            subdiv_sbasis(-test_sb[1] + Linear(3*width/4),
                          rts, 0, 1);
            iterations++;
        }
    
        *notify << "sbasis subdivision " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
    #endif    
    #if 0
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solutions.resize(0);
            find_parametric_bezier_roots(&trans[0], 5, solutions, 0);
            iterations++;
        }
        *notify << "solver parametric time = " 
                << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
    #endif
        double ys[trans.size()];
        for(unsigned i = 0; i < trans.size(); i++) {
            ys[i] = trans[i][1];
            double x = double(i)/(trans.size()-1);
            x = (1-x)*height/4 + x*height*3/4;
            draw_handle(cr, Geom::Point(x, height/2 + ys[i]));
        }
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solutions.resize(0);
            find_bernstein_roots(ys, 5, solutions, 0, 0, 1, false);
            iterations++;
        }
        *notify << "found sub solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
        *notify << "solver 1d bernstein subdivision slns" << solutions.size() 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            solutions.resize(0);
            find_bernstein_roots(ys, 5, solutions, 0, 0, 1, true);
            iterations++;
        }
        *notify << "solver 1d bernstein secant subdivision slns" << solutions.size() 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
        *notify << "found secant solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
        *notify << "solver 1d bernstein subdivision accuracy:"
                 << std::endl;
        for(unsigned i = 0; i < solutions.size(); i++) {
            *notify << solutions[i] << ":" << eval_bernstein(ys, solutions[i], trans.size()) << ",";
        }
        solutions = roots( -test_sb[1] + Linear(height/2));
#if 0
        std::cout << "sbasis sub: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(std::cout, ",\t"));
        std::cout << std::endl;
#endif        
        for(unsigned i = 0; i < solutions.size(); i++) {
            double x = test_sb[0](solutions[i]);
            draw_cross(cr, Geom::Point(x, height/2));
            
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
            
        D2<SBasis> B = psh.asBezier();//handles_to_sbasis(handles.begin(), 5);
        Geom::Path pb;
        pb.append(B);
        pb.close(false);
        cairo_path(cr, pb);
        
        B[0] = Linear(width/4, 3*width/4);
        cairo_md_sb(cr, B);
        Toy::draw(cr, notify, width, height, save);
    }
    RootFinderComparer() : timer_precision(0.1), units(1e6), units_string("us") // microseconds
    {
        for(unsigned i = 0; i < 6; i++) psh.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(&psh);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new RootFinderComparer());

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
