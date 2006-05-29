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
    
    Poly operator+(const Poly& p) {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::max(size(), p.size());
        
        for(unsigned i = 0; i < min_size; i++) {
            result.coeff.push_back(coeff[i] + p.coeff[i]);
        }
        for(unsigned i = size(); i < p.size(); i++)
            result.coeff.push_back(coeff[i]);
        for(unsigned i = p.size(); i < size(); i++)
            result.coeff.push_back(p.coeff[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator-(const Poly& p) {
        Poly result;
        const unsigned out_size = std::max(size(), p.size());
        const unsigned min_size = std::max(size(), p.size());
        
        for(unsigned i = 0; i < min_size; i++) {
            result.coeff.push_back(coeff[i] - p.coeff[i]);
        }
        for(unsigned i = size(); i < p.size(); i++)
            result.coeff.push_back(coeff[i]);
        for(unsigned i = p.size(); i < size(); i++)
            result.coeff.push_back(-p.coeff[i]);
        assert(result.size() == out_size);
        return result;
    }
    Poly operator*(const double p) {
        Poly result;
        const unsigned out_size = size();
        
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
    Poly operator*(const Poly& p);

    double eval(double x);
    double operator()(double t) { return eval(t);}
};

Poly integral(Poly const & p);
Poly derivative(Poly const & p);
Poly divide_out_root(Poly const & p, double x);

std::vector<std::complex<double> > solve(const Poly & p);

inline std::ostream &operator<< (std::ostream &out_file, const Poly &in_poly) {
    if(in_poly.size() == 0)
        out_file << "0";
    else {
        for(int i = (int)in_poly.coeff.size()-1; i >= 0; --i) {
            out_file << "x^" << i << "*" << in_poly.coeff[i];
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
