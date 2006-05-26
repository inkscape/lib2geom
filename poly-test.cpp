#include "poly.h"

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.coeff.push_back(b);
    p.coeff.push_back(a);
    return p;
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
