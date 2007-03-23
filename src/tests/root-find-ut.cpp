#include "poly.h"
#include <vector>
#include <iterator>

#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "sturm.h"
#include "s-basis.h"
#include "sbasis-poly.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include <time.h>

using namespace std;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

Linear linear(double ax, double b) {
    return Linear(b, ax+b);
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

std::vector<double> roots_buggy(SBasis const & s) {
    std::vector<double> b = sbasis_to_bezier(s), r;
    
    find_bernstein_roots_buggy(&b[0], b.size()-1, r, 0, 0., 1.);
    return r;
}

int
main(int argc, char** argv) {
    Poly a, b, r;
    double timer_precision = 0.1;
    double units = 1e6; // us
    
    a = Poly::linear(1, -0.3)*Poly::linear(1, -0.25)*Poly::linear(1, -0.2);
    
    std::cout << a <<std::endl;
    SBasis B = poly_to_sbasis(a);
    std::cout << B << std::endl;
    std::vector<double> bez = sbasis_to_bezier(B);
    copy(bez.begin(), bez.end(), ostream_iterator<double>(cout, ", "));
    cout << endl;
    cout << endl;
    cout << endl;
    
    std::vector<std::vector<double> > trials;
    
    // evenly spaced roots
    for(int N = 2; N <= 5; N++)
    {
        std::vector<double> r;
        for(int i = 0; i < N; i++)
            r.push_back(double(i)/(N-1));
        trials.push_back(r);
    }
    // sort of evenish
    for(int N = 0; N <= 5; N++)
    {
        std::vector<double> r;
        for(int i = 0; i < N; i++)
            r.push_back(double(i+0.5)/(2*N));
        trials.push_back(r);
    }
    // one at 0.1
    for(int N = 0; N <= 5; N++)
    {
        std::vector<double> r;
        for(int i = 0; i < N; i++)
            r.push_back(i+0.1);
        trials.push_back(r);
    }
    for(int N = 0; N <= 6; N++)
    {
        std::vector<double> r;
        for(int i = 0; i < N; i++)
            r.push_back(i*0.8+0.1);
        trials.push_back(r);
    }
    for(int N = 0; N <= 20; N++)
    {
        std::vector<double> r;
        for(int i = 0; i < N/2; i++) {
            r.push_back(0.1);
            r.push_back(0.9);
        }
        trials.push_back(r);
    }
    for(int i = 0; i <= 20; i++)
    {
        std::vector<double> r;
        for(int i = 0; i < 4; i++) {
            r.push_back(uniform()*5 - 2.5);
            r.push_back(0.9);
        }
        trials.push_back(r);
    }
    double ave_left = 0;
    double ave_right = 0;
    double ave_rel = 0;
    cout << "split at first choice \t split at 0.5 \t\t err from exact\n";
    for(int i = 0; i < trials.size(); i++) {
        SBasis B = Linear(1.,1);
        for(int j = 0; j < trials[i].size(); j++) {
            B = B*linear(1, -trials[i][j]);
        }
        int N = B.size()*2;
        if(B.back()[0] == B.back()[1])
            N--;
        //std::cout << sbasis_to_poly(B) <<std::endl;
        //std::cout << B << std::endl;
        /*std::vector<double> bez = sbasis_to_bezier(B);
          for(int i = 0; i < N; i++) 
          printf("%g %g\n", (i/10.), bez[i]);
          cout << endl;*/
        double left_time, right_time;
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            roots(B);
            iterations++;
        }
        left_time = timer_precision*units/iterations;
        cout << left_time;
        {
	    clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
	    unsigned iterations = 0;
	    while(end_t > clock()) {
                roots_buggy(B);
                iterations++;
	    }
    
            right_time = timer_precision*units/iterations;
	    cout <<"\t\t\t" <<  right_time;
        }
        std::vector<double> rt = roots(B);
        /*cout <<" roots = ";
          copy(rt.begin(), rt.end(), ostream_iterator<double>(cout, ", "));
          cout << endl;*/
	    
        double err = 0;
        for(int k = 0; k < rt.size(); k++) {
            double r = rt[k];
            double best = fabs(r - trials[i][0]);
            for(int j = 1; j < trials[i].size(); j++) {
                if(fabs(r - trials[i][j]) < best)
                    best = fabs(r - trials[i][j]);
            }
            err += best;
        }
        cout << "\t\t e: " << err << std::endl;
        ave_left += left_time;
        ave_right += right_time;
        ave_rel += left_time / right_time;
    }
    cout << "average time left = " << ave_left/trials.size() << std::endl;
    cout << "average time right = " << ave_right/trials.size() << std::endl;
    cout << "average relative time increase left/right= " << ave_rel/trials.size() << std::endl;
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
