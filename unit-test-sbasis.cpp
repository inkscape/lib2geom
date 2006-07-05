#include <iostream>
#include <math.h>
#include <cassert>
#include "sbasis-poly.h"

int main() {
    SBasis P0(BezOrd(0.5, -1)), P1(BezOrd(3, 1));
    std::cout << "round tripping of poly conversions\n";
    std::cout << P0 
              << "=>" << sbasis_to_poly(P0) 
              << "=>" << poly_to_sbasis(sbasis_to_poly(P0)) 
              << std::endl;
    
    std::cout << "derivatives and integrals\n";
    
    Poly test;
    for(int i = 0; i < 4; i++)
        test.coeff.push_back(1);
    
    SBasis test_sb = poly_to_sbasis(test);
    std::cout << test << "(" << test.size() << ")"
              << "   ==   "
              << test_sb << "(" << test_sb.size() << ")"
              << std::endl;
    std::cout << derivative(test)
              << "   ==   "
              << sbasis_to_poly(derivative(test_sb))
              << std::endl;
    
    std::cout << integral(test)
              << "   ==   "
              << sbasis_to_poly(integral(test_sb))
              << std::endl;
    
    std::cout << "evaluation\n";
    std::cout << integral(test)(0.3) - integral(test)(0.)
              << "   ==   "
              << integral(test_sb)(0.3) - integral(test_sb)(0.)
              << std::endl;
    
    std::cout << "multiplication\n";
    std::cout << (test*test)
              << "\n   ==   \n"
              << sbasis_to_poly(multiply(test_sb,test_sb))
              << std::endl;
    std::cout << poly_to_sbasis(test*test)
              << "\n   ==   \n"
              << multiply(test_sb,test_sb)
              << std::endl;
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
