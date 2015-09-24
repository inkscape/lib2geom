#include <2geom/polynomial.h>
#include <vector>
#include <iterator>

#include <2geom/sbasis.h>
#include <2geom/sbasis-poly.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/solver.h>
#include <time.h>

using namespace std;
using namespace Geom;

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

int main() {
    Poly a, b, r;
    double timer_precision = 0.01;
    double units = 1e6; // us
    
    a = Poly::linear(1, -0.3)*Poly::linear(1, -0.25)*Poly::linear(1, -0.2);
    
    std::cout << a <<std::endl;
    SBasis B = poly_to_sbasis(a);
    std::cout << B << std::endl;
    Bezier bez;
    sbasis_to_bezier(bez, B);
    cout << bez << endl;
    //copy(bez.begin(), bez.end(), ostream_iterator<double>(cout, ", "));
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
    cout << "err from exact\n";
    for(unsigned i = 0; i < trials.size(); i++) {
        SBasis B = Linear(1.,1);
        sort(trials[i].begin(), trials[i].end());
        for(unsigned j = 0; j < trials[i].size(); j++) {
            B = B*linear(1, -trials[i][j]);
        }
        double left_time;
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            roots(B);
            iterations++;
        }
        left_time = timer_precision*units/iterations;
        vector<double> rt = roots(B);
        double err = 0;
        for(unsigned k = 0; k < rt.size(); k++) {
            double r = rt[k];
            double best = fabs(r - trials[i][0]);
            for(unsigned j = 1; j < trials[i].size(); j++) {
                if(fabs(r - trials[i][j]) < best)
                    best = fabs(r - trials[i][j]);
            }
            err += best;
        }
        if(err > 1e-8){
            for(unsigned j = 0; j < trials[i].size(); j++) {
                cout << trials[i][j] << ", ";
            }
            cout << endl;
        }
        cout << " e: " << err << std::endl;
        ave_left += left_time;
    }
    cout << "average time = " << ave_left/trials.size() << std::endl;
    
    for(int i = 10; i >= 0; i--) {
        vector<double> rt = roots(Linear(i,-1));
        for(unsigned j = 0; j < rt.size(); j++) {
            cout << rt[j] << ", ";
        }
        cout << endl;
    }

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
