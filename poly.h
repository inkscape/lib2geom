#ifndef SEEN_POLY_H
#define SEEN_POLY_H
#include <assert.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <complex>

class Poly{
public:
    std::vector<double> coeff; // sum x^i*coeff[i]
    
    unsigned size() const { return coeff.size();}
    unsigned degree() const { return size()-1;}

    double operator[](const int i) const { return coeff[i];}
    
    Poly operator+(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        //result.coeff.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.coeff.push_back(coeff[i] + p.coeff[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.coeff.push_back(coeff[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.coeff.push_back(p.coeff[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-(const Poly& p) const {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::min(size(), p.size());
        result.coeff.reserve(out_size);
        
        for(unsigned i = 0; i < min_size; i++) {
            result.coeff.push_back(coeff[i] - p.coeff[i]);
        }
        for(unsigned i = min_size; i < size(); i++)
            result.coeff.push_back(coeff[i]);
        for(unsigned i = min_size; i < p.size(); i++)
            result.coeff.push_back(-p.coeff[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-(const double k) const {
        Poly result;
        const unsigned out_size = size();
        result.coeff.reserve(out_size);
        
        for(unsigned i = 0; i < out_size; i++) {
            result.coeff.push_back(coeff[i]);
        }
        result.coeff[0] -= k;
        return result;
    }
    Poly operator*(const double p) const {
        Poly result;
        const unsigned out_size = size();
        result.coeff.reserve(out_size);
        
        for(unsigned i = 0; i < out_size; i++) {
            result.coeff.push_back(coeff[i]*p);
        }
        assert(result.size() == out_size);
        return result;
    }
// equivalent to multiply by x^terms, discard negative terms
    Poly shifted(int terms) { 
        Poly result;
        
        const unsigned out_size = std::max(0u, size()+terms);
        result.coeff.reserve(out_size);
        
        if(terms < 0) {
            for(unsigned i = 0; i < out_size; i++) {
                result.coeff.push_back(coeff[i-terms]);
            }
        } else {
            for(unsigned i = 0; i < terms; i++) {
                result.coeff.push_back(0.0);
            }
            for(unsigned i = 0; i < size(); i++) {
                result.coeff.push_back(coeff[i]);
            }
        }
        
        assert(result.size() == out_size);
        return result;
    }
    Poly operator*(const Poly& p) const;

    double eval(double x) const;
    double operator()(double t) const { return eval(t);}
    
    void normalize();
    
    void monicify();
    Poly() {}
    Poly(const Poly& p) : coeff(p.coeff) {}
    Poly(const double a) {coeff.push_back(a);}
    
};

inline Poly operator*(double a, Poly const & b) { return b * a;}

Poly integral(Poly const & p);
Poly derivative(Poly const & p);
Poly divide_out_root(Poly const & p, double x);
Poly compose(Poly const & a, Poly const & b);

/*** solve(Poly p)
 * find all p.degree() roots of p.
 * This function can take a long time with suitably crafted polynomials, but in practice it should be fast.  Should we provide special forms for degree() <= 4?
 */
std::vector<std::complex<double> > solve(const Poly & p);

/*** solve_reals(Poly p)
 * find all real solutions to Poly p.
 * currently we just use solve and pick out the suitably real looking values, there may be a better algorithm.
 */
std::vector<double> solve_reals(const Poly & p);
double polish_root(Poly const & p, double guess, double tol);

inline std::ostream &operator<< (std::ostream &out_file, const Poly &in_poly) {
    if(in_poly.size() == 0)
        out_file << "0";
    else {
        for(int i = (int)in_poly.coeff.size()-1; i >= 0; --i) {
            out_file << "(" << in_poly.coeff[i] << ")*x^" << i;
            if(i)
                out_file << " + ";
        }
    }
    return out_file;
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
#endif
