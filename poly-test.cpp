#include "poly.h"
#include <vector>
#include <iterator>

#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "sturm.h"

//x^5*1 + x^4*1212.36 + x^3*-2137.83 + x^2*1357.77 + x^1*-366.403 + x^0*42.0846


using namespace std;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

int
main(int argc, char** argv) {
    Poly a, b, r;
    
    a.push_back(1);
    a.push_back(2);
    a.push_back(1);
    
    b.push_back(1);
    b.push_back(1);
    
    std::cout << a <<std::endl;
    std::cout << b <<std::endl;
    std::cout << b*b <<std::endl;
    std::cout << b*b*b <<std::endl;
    
    std::cout << derivative(b*b*b) <<std::endl;
    std::cout << integral(derivative(b*b*b)) <<std::endl;
    
    Poly f, g;
    
    f.push_back(-42);
    f.push_back(0);
    f.push_back(-12);
    f.push_back(1);
    
    g.push_back(-3);
    g.push_back(1);
    
    
    cout << "divide (example from wikipedia): " << f << "/" << g << endl;
    std::cout << divide(f, g, r) << endl;
    cout << r << endl;

    g[0] = -3;
    g[1] = 1;
    g.push_back(1);
    
    cout << "divide (example from wikipedia): " << f << "/" << g << endl;
    std::cout << divide(f, g, r) << endl;
    cout << r << endl;

    // polynomial with a double root at -3
    Poly multiplicity = lin_poly(1, 3)*lin_poly(1, 3)*lin_poly(1,-4)*lin_poly(1,-7)*lin_poly(2,1);
    multiplicity.normalize();
    
    cout << "gcd - we're using a multiple factor poly against its derivative.\n";
    Poly gc = gcd(multiplicity, derivative(multiplicity));
    cout << "gcd(p, p') = " << gc << endl;
    
    
    Poly prod = lin_poly(1, 3)*lin_poly(1,-4)*lin_poly(1,-7)*lin_poly(2,1);
    
    std::cout << prod <<std::endl;
    solve(prod) ;
    //DK(prod);

    sturm st(prod);
    
    for(double t = -3.25; t < 8.25; t+= 0.5) {
        std::cout << st.count_signs(t) << std::endl;
    }
    std::cout << "infty = " << st.count_signs(INFINITY) << std::endl;
    
    Poly dk_trial;
    dk_trial.push_back(-5);
    dk_trial.push_back(3);
    dk_trial.push_back(-3);
    dk_trial.push_back(1);

    Poly p = prod*dk_trial;
    
    vector<complex<double> > prod_root = solve(prod);
    copy(prod_root.begin(), prod_root.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    cout << endl;

    DK(p);
    {
    vector<complex<double> > prod_root = solve(p);
    copy(prod_root.begin(), prod_root.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    cout << endl;
    }
    
    cout << "p = " << p << " = ";

    vector<complex<double> > sol = Laguerre(p);
    copy(sol.begin(), sol.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    
    cout << endl;
    
    //std::cout << prod.eval(4.) << std::endl;
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
