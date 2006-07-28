#include "poly.h"

Poly Poly::operator*(const Poly& p) const {
    Poly result;
    const unsigned out_size = degree() +  p.degree()+1;
    const unsigned min_size = std::max(size(), p.size());
    
    result.coeff.resize(out_size);
    
    for(unsigned i = 0; i < size(); i++) {
        for(unsigned j = 0; j < p.size(); j++) {
            result.coeff[i+j] += coeff[i] * p.coeff[j];
        }
    }
    return result;
}

#include <gsl/gsl_poly.h>

double Poly::eval(double x) const {
    return gsl_poly_eval(&coeff[0], size(), x);
}

void Poly::normalize() {
    while(coeff.back() == 0)
        coeff.pop_back();
}

void Poly::monicify() {
    normalize();
    
    double scale = 1./coeff.back(); // unitize
    
    for(unsigned i = 0; i < size(); i++) {
        coeff[i] *= scale;
    }
}


std::vector<std::complex<double> > solve(Poly const & pp) {
    Poly p(pp);
    p.normalize();
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (p.size());
       
    gsl_complex_packed_ptr z = new double[p.degree()*2];
    double* a = new double[p.size()];
    for(int i = 0; i < p.size(); i++)
        a[i] = p[i];
    std::vector<std::complex<double> > roots;
    //roots.resize(p.degree());
    
    gsl_poly_complex_solve (a, p.size(), w, z);
    delete[]a;
     
    gsl_poly_complex_workspace_free (w);
     
    for (int i = 0; i < p.degree(); i++) {
        roots.push_back(std::complex<double> (z[2*i] ,z[2*i+1]));
        //printf ("z%d = %+.18f %+.18f\n", i, z[2*i], z[2*i+1]);
    }    
    delete[] z;
    return roots;
}

std::vector<double > solve_reals(Poly const & p) {
    std::vector<std::complex<double> > roots = solve(p);
    std::vector<double> real_roots;
    
    for(int i = 0; i < roots.size(); i++) {
        if(roots[i].imag() == 0) // should be more lenient perhaps
            real_roots.push_back(roots[i].real());
    }
    return real_roots;
}

double polish_root(Poly const & p, double guess, double tol) {
    Poly dp = derivative(p);
    
    double fn = p(guess);
    while(fabs(fn) > tol) {
        guess -= fn/dp(guess);
        fn = p(guess);
    }
    return guess;
}

Poly integral(Poly const & p) {
    Poly result;
    
    result.coeff.reserve(p.size()+1);
    result.coeff.push_back(0); // arbitrary const
    for(int i = 0; i < p.size(); i++) {
        result.coeff.push_back(p[i]/(i+1));
    }
    return result;

}

Poly derivative(Poly const & p) {
    Poly result;
    
    result.coeff.reserve(p.size()-1);
    for(int i = 1; i < p.size(); i++) {
        result.coeff.push_back(i*p[i]);
    }
    return result;
}

Poly compose(Poly const & a, Poly const & b) {
    Poly result;
    
    for(int i = a.size()-1; i >=0; i--) {
        result = Poly(a.coeff[i]) + result * b;
    }
    return result;
    
}

/* This version is backwards - dividing taylor terms
Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    
    const unsigned k = a.size();
    r.coeff.resize(k, 0);
    c.coeff.resize(k, 0);

    for(unsigned i = 0; i < k; i++) {
        double ci = r[i]/b[0];
        c.coeff[i] += ci;
        Poly bb = ci*b;
        std::cout << ci <<"*" << b << ", r= " << r << std::endl;
        r -= bb.shifted(i);
    }
    
    return c;
}
*/

// probably wrong
Poly divide(Poly const &a, Poly const &b, Poly &r) {
    Poly c;
    r = a; // remainder
    
    const unsigned k = a.size();
    r.coeff.resize(k, 0);
    c.coeff.resize(k-1, 0);

    for(int i = k-1; i >= b.size()-1; i--) {
        double ci = r[i]/b[0];
        c.coeff[i-1] += ci;
        Poly bb = ci*b;
        std::cout << ci <<"*" << b.shifted(i-1) << ", r= " << r << std::endl;
        r -= bb.shifted(i-1);
        r.coeff[i] = 0;
    }
    
    return c;
}




/*Poly divide_out_root(Poly const & p, double x) {
    assert(1);
    }*/


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
