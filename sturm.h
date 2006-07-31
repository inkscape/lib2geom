#include "poly.h"

template <typename T> int sgn(T x) { return (x<0)?-1:(x>0)?1:0; }

class sturm : public std::vector<Poly>{
public:
    sturm(Poly const &X) {
        push_back(X);
        push_back(derivative(X));
        Poly Xi = back();
        Poly const &Xim1(X);
        while(!Xi.empty()) {
            Poly r;
            divide(Xi, Xim1, r);
            Xim1(Xi);
            Xi = -r;
            push_back(Xi);
        }
    }
    
    unsigned count_signs(double t) {
        unsigned n_signs = 0;/*  Number of sign-changes */
        
        int old_sign = sgn((*this)[0].eval(t));
        for (int i = 1; i < size(); i++) {
            int sign = sgn((*this)[i].eval(t));
            if (sign != old_sign)
                n_signs++;
            old_sign = sign;
        }
        return n_signs;
    }
};

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
