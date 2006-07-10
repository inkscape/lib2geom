#include <iostream>
#include <math.h>
#include <cassert>
#include "sbasis-poly.h"

int main() {
    SBasis P0(BezOrd(0.5, -1)), P1(BezOrd(3, 1));
    BezOrd one(1,1);

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
    std::cout << "derivative\n";
    std::cout << derivative(test)
              << "   ==   "
              << sbasis_to_poly(derivative(test_sb))
              << std::endl;
    
    std::cout << "integral\n";
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
    
    std::cout << "sqrt\n";
    std::cout << test
              << "\n   ==   \n"
              << sbasis_to_poly(sqrt(multiply(test_sb,test_sb),10))
              << std::endl;
    SBasis radicand = sqrt(test_sb,10);
    std::cout << sbasis_to_poly(truncate(multiply(radicand, radicand),5))
              << "\n   ==   \n"
              << test
              << std::endl;
    
    std::cout << "division\n";
    std::cout << test
              << "\n   ==   \n"
              << sbasis_to_poly(divide(multiply(test_sb,test_sb),test_sb, 20))
              << std::endl;
    std::cout << divide(test_sb, radicand,5)
              << "\n   ==   \n"
              << truncate(radicand,6)
              << std::endl;
    
    std::cout << "composition\n";
    std::cout << (compose(test,sbasis_to_poly(BezOrd(0.5,1))))
              << "\n   ==   \n"
              << sbasis_to_poly(compose(test_sb,BezOrd(0.5,1)))
              << std::endl;
    std::cout << poly_to_sbasis(compose(test,test))
              << "\n   ==   \n"
              << compose(test_sb,test_sb)
              << std::endl;

    std::cout << "inverse\n";
    SBasis A = sqrt(BezOrd(1,4), 5) - one;
    Poly P;
    P.coeff.push_back(1./3);
    P.coeff.push_back(-2./3);
    P.coeff.push_back(0);

    std::cout << sbasis_to_poly(inverse(A,5))
              << "\n   ==   \n"
              << P
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
