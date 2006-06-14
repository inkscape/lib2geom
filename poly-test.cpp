#include "poly.h"
#include <vector>
#include <iterator>

//x^5*1 + x^4*1212.36 + x^3*-2137.83 + x^2*1357.77 + x^1*-366.403 + x^0*42.0846


using namespace std;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.coeff.push_back(b);
    p.coeff.push_back(a);
    return p;
}

complex<double> eval(Poly const & p, complex<double> x) {
    complex<double> result = 0;
    complex<double> xx = 1;
    
    for(int i = 0; i < p.size(); i++) {
        result += p.coeff[i]*xx;
        xx *= x;
    }
    return result;
}

void DK(Poly const & ply) {
    vector<complex<double> > roots;
    vector<complex<double> > new_roots;
    const int N = ply.degree();
    
    complex<double> b(0.4, 0.9);
    complex<double> p = 1;
    for(int i = 0; i < N; i++) {
        roots.push_back(p);
        p *= b;
    }
    assert(roots.size() == ply.degree());
    
    copy(roots.begin(), roots.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    cout << endl;
    
    new_roots.resize(N);
    for(int i = 0; i < 35; i++) {
        for(int r_i = 0; r_i < N; r_i++) {
            complex<double> denom = 1;
            complex<double> R = roots[r_i];
            for(int d_i = 0; d_i < N; d_i++) {
                if(r_i != d_i)
                    denom *= R-roots[d_i];
            }
            //cout << denom <<endl;
            roots[r_i] = R - eval(ply, R)/denom;
        }
        //copy(new_roots.begin(), new_roots.end(), roots.begin());
        copy(roots.begin(), roots.end(), ostream_iterator<complex<double> >(cout, ",\t"));
        cout << endl;
    }
}

int
main(int argc, char** argv) {
    Poly a, b;
    
    a.coeff.push_back(1);
    a.coeff.push_back(2);
    a.coeff.push_back(1);
    
    b.coeff.push_back(1);
    b.coeff.push_back(1);
    
    std::cout << a <<std::endl;
    std::cout << b <<std::endl;
    std::cout << b*b <<std::endl;
    std::cout << b*b*b <<std::endl;
    
    std::cout << derivative(b*b*b) <<std::endl;
    std::cout << integral(derivative(b*b*b)) <<std::endl;
    Poly prod = lin_poly(1, 3)*lin_poly(1,-4)*lin_poly(1,-7)*lin_poly(2,1);
    
    std::cout << prod <<std::endl;
    solve(prod) ;
    DK(prod);
    
    Poly dk_trial;
    dk_trial.coeff.push_back(-5);
    dk_trial.coeff.push_back(3);
    dk_trial.coeff.push_back(-3);
    dk_trial.coeff.push_back(1);
    vector<complex<double> > prod_root = solve(prod);
    copy(prod_root.begin(), prod_root.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    cout << endl;

    DK(dk_trial);
    {
    vector<complex<double> > prod_root = solve(dk_trial);
    copy(prod_root.begin(), prod_root.end(), ostream_iterator<complex<double> >(cout, ",\t"));
    cout << endl;
    }
    
    std::cout << prod.eval(4) << std::endl;
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
