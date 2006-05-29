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

std::vector<std::complex<double> > solve(Poly const & p) {
    gsl_poly_complex_workspace * w 
        = gsl_poly_complex_workspace_alloc (p.size());
       
    gsl_complex_packed_ptr z = new double[p.degree()*2];
    double* a = new double[p.size()];
    for(int i = 0; i < p.size(); i++)
        a[i] = p[i];
    std::vector<std::complex<double> > roots;
    roots.resize(p.degree());
    
    gsl_poly_complex_solve (a, p.size(), w, z);
    delete[]a;
     
    gsl_poly_complex_workspace_free (w);
     
    for (int i = 0; i < p.degree(); i++) {
        roots.push_back(std::complex<double> (z[2*i] ,z[2*i+1]));
        printf ("z%d = %+.18f %+.18f\n", 
                i, z[2*i], z[2*i+1]);
    }    
    delete[] z;
    return roots;
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
