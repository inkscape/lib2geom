#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/solver.h>
#include <2geom/sbasis-poly.h>
#include <2geom/orphan-code/nearestpoint.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)
#include <2geom/path.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#define ZROOTS_TEST 0
#if ZROOTS_TEST
#include <2geom/zroots.c>
#endif

using std::swap;

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
        swap(prev_row, row);
    }
    return prev_row[0];
}

#include <vector>
using std::vector;
using namespace Geom;

#ifdef HAVE_GSL
#include <complex>
using std::complex;
#endif

class RootFinderComparer: public Toy {
public:
    PointSetHandle psh;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        std::vector<Geom::Point> trans;
        trans.resize(psh.size());
        for(unsigned i = 0; i < handles.size(); i++) {
            trans[i] = psh.pts[i] - Geom::Point(0, height/2);
        }
        
        Timer tm;
        
        
        std::vector<double> solutions;
        solutions.resize(6);
        
        tm.ask_for_timeslice();
        tm.start();
        FindRoots(&trans[0], 5, &solutions[0], 0);
        Timer::Time als_time = tm.lap();
        *notify << "original time = " << als_time << std::endl;
        
        D2<SBasis> test_sb = psh.asBezier();//handles_to_sbasis(handles.begin(),5);
        Interval bs = *bounds_exact(test_sb[1]);
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
#if 1
        *notify << "gsl: ";
        std::copy(complex_solutions.begin(), complex_solutions.end(), std::ostream_iterator<std::complex<double> >(*notify, ",\t"));
        *notify << std::endl;
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

#ifdef HAVE_GSL    
        
        tm.ask_for_timeslice();
        tm.start();
        solve(ply);
        als_time = tm.lap();
        *timer_stream << "gsl poly = " << als_time << std::endl;
#endif
  
    #if ZROOTS_TEST
        tm.ask_for_timeslice();
        tm.start();
        zroots(a, ply.size(), rts, false);
        als_time = tm.lap();
        *timer_stream << "zroots poly = " << als_time << std::endl;
    #endif    
    
    #if LAGUERRE_TEST
        tm.ask_for_timeslice();
        tm.start();
        Laguerre(ply);
        als_time = tm.lap();
        *timer_stream << "Laguerre poly = " << als_time << std::endl;
        complex_solutions = Laguerre(ply);
    
    #endif    
    
    #define SBASIS_SUBDIV_TEST 0
    #if SBASIS_SUBDIV_TEST
        tm.ask_for_timeslice();
        tm.start();
        subdiv_sbasis(-test_sb[1] + Linear(3*width/4),
                      rts, 0, 1);
        als_time = tm.lap();
        *timer_stream << "sbasis subdivision = " << als_time << std::endl;
    #endif    
    #if 0
        tm.ask_for_timeslice();
        tm.start();
        solutions.resize(0);
        find_parametric_bezier_roots(&trans[0], 5, solutions, 0);
        als_time = tm.lap();
        *timer_stream << "solver parametric = " << als_time << std::endl;
    #endif
        double ys[trans.size()];
        for(unsigned i = 0; i < trans.size(); i++) {
            ys[i] = trans[i][1];
            double x = double(i)/(trans.size()-1);
            x = (1-x)*height/4 + x*height*3/4;
            draw_handle(cr, Geom::Point(x, height/2 + ys[i]));
        }
        
        solutions.resize(0);
        tm.ask_for_timeslice();
        tm.start();
        find_bernstein_roots(ys, 5, solutions, 0, 0, 1, false);
        als_time = tm.lap();
        *notify << "found sub solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
        *notify << "solver 1d bernstein subdivision n_slns = " << solutions.size() 
                << ", time = " << als_time << std::endl;

        solutions.resize(0);
        tm.ask_for_timeslice();
        tm.start();
        find_bernstein_roots(ys, 5, solutions, 0, 0, 1, true);
        als_time = tm.lap();

        *notify << "solver 1d bernstein secant subdivision slns" << solutions.size() 
                << ", time = " << als_time << std::endl;
        *notify << "found secant solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
        *notify << "solver 1d bernstein subdivision accuracy:"
                 << std::endl;
        for(double solution : solutions) {
            *notify << solution << ":" << eval_bernstein(ys, solution, trans.size()) << ",";
        }
        tm.ask_for_timeslice();
        tm.start();
        solutions = roots( -test_sb[1] + Linear(height/2));
        als_time = tm.lap();
#if 1
        *notify << "sbasis roots: ";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double>(*notify, ",\t"));
        *notify << "\n time = " << als_time << std::endl;
#endif        
        for(double solution : solutions) {
            double x = test_sb[0](solution);
            draw_cross(cr, Geom::Point(x, height/2));
            
        }

        *notify << "found " << solutions.size() << "solutions at:\n";
        std::copy(solutions.begin(), solutions.end(), std::ostream_iterator<double >(*notify, ","));
            
        D2<SBasis> B = psh.asBezier();//handles_to_sbasis(handles.begin(), 5);
        Geom::Path pb;
        pb.append(B);
        pb.close(false);
        cairo_path(cr, pb);
        
        B[0] = Linear(width/4, 3*width/4);
        cairo_d2_sb(cr, B);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    RootFinderComparer(unsigned degree)
    {
        for(unsigned i = 0; i < degree; i++) psh.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(&psh);
    }
};

int main(int argc, char **argv) {
    unsigned bez_ord = 6;
    if(argc > 1)
        sscanf(argv[1], "%d", &bez_ord);
    init(argc, argv, new RootFinderComparer(bez_ord));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
