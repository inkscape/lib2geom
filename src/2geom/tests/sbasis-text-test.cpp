#include <iostream>
#include <math.h>
#include <cassert>
#include <2geom/sbasis.h>
#include <2geom/sbasis-poly.h>
#include <iterator>
#include <2geom/point.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/solver.h>

using namespace Geom;

Poly roots_to_poly(double *a, unsigned n) {
    Poly r;
    r.push_back(1);
    
    for(unsigned i = 0; i < n; i++) {
        Poly p;
        p.push_back(-a[i]);
        p.push_back(1);
        r = r*p;
    }
    return r;
}

unsigned small_rand() {
    return (rand() & 0xff) + 1;
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

int main() {
    SBasis P0(Linear(0.5, -1)), P1(Linear(3, 1));
    Linear one(1,1);

    std::cout << "round tripping of poly conversions\n";
    std::cout << P0 
              << "=>" << sbasis_to_poly(P0) 
              << "=>" << poly_to_sbasis(sbasis_to_poly(P0)) 
              << std::endl;
    
    std::cout << "derivatives and integrals\n";
    
    Poly test;
    for(int i = 0; i < 4; i++)
        test.push_back(1);
    
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
    std::cout << (compose(test,sbasis_to_poly(Linear(0.5,1))))
              << "\n   ==   \n"
              << sbasis_to_poly(compose(test_sb,Linear(0.5,1)))
              << std::endl;
    std::cout << poly_to_sbasis(compose(test,test))
              << "\n   ==   \n"
              << compose(test_sb,test_sb)
              << std::endl
              << std::endl;
    std::cout << (compose(sbasis_to_poly(Linear(1,2)),sbasis_to_poly(Linear(-1,0))))
              << std::endl;
    std::cout << (compose(SBasis(Linear(1,2)),SBasis(Linear(-1,0))))
              << std::endl;

    std::cout << "inverse of x - 1\n";
    std::cout << sbasis_to_poly(inverse(Linear(-1,0),2))
              << "   ==   y + 1\n";
    std::cout << "f^-1(f(x)) = " 
              << sbasis_to_poly(compose(inverse(Linear(-1,0),2), 
                                        Linear(-1,0))) 
              << std::endl
              << std::endl;
    
    std::cout << "inverse of 3x - 2\n";
    std::cout << sbasis_to_poly(inverse(Linear(-2,1),2))
              << "   ==   (y + 2)/3\n";
    std::cout << "f^-1(f(x)) = " 
              << sbasis_to_poly(compose(inverse(Linear(-2,1),2), 
                                        Linear(-2,1))) 
              << std::endl
              << std::endl;
    
    
    std::cout << "inverse of sqrt(" << sbasis_to_poly(Linear(1,4)) << ") - 1\n";
    SBasis A = sqrt(Linear(1,4), 5) - one;
    Poly P;
    P.push_back(0);
    P.push_back(2./3);
    P.push_back(1./3);
    
    std::cout << "2 term approximation\n";
    std::cout << sbasis_to_poly(inverse(A,2))
              << "\n   ==   \n"
              << P
              << std::endl;
    std::cout << "general approximation\n";
    std::cout << sbasis_to_poly(inverse(A,5))
              << "\n   ==   \n"
              << P
              << std::endl;
    
    {
    std::cout << "inverse of (x^2+2x)/3\n";
    SBasis A = poly_to_sbasis(P);
    SBasis I = inverse(A,10);
    std::cout << sbasis_to_poly(truncate(compose(A, I), 10))
              << "   ==   x\n"
              << std::endl;
    std::cout << sbasis_to_poly(truncate(compose(I, A), 10))
              << "   ==   x\n"
              << std::endl;
    std::cout << sbasis_to_poly(truncate(I - (sqrt(Linear(1,4), 10) - one), 10))
              << std::endl;
    }
#ifdef HAVE_GSL
    for(int i = 0 ; i < 10; i++) {
        Poly P;
        P.push_back(0);
        P.push_back(1);
        for(int j = 0 ; j < 2; j++) {
            P.push_back((uniform()-0.5)/10);
        }
        std::vector<std::complex<double> > prod_root = solve(derivative(P));
        copy(prod_root.begin(), prod_root.end(), 
             std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
        std::cout << std::endl;
        std::cout << "inverse of (" << P << " )\n";
        for(int k = 1; k < 30; k++) {
            SBasis A = poly_to_sbasis(P);
            SBasis I = inverse(A,k);
            SBasis err = compose(A, I) - Linear(0,1); // ideally is 0
            std::cout << truncate(err, k).tailError(0)
                      << std::endl;
            /*std::cout << sbasis_to_poly(err)
                      << "   ==   x\n"
                      << std::endl;*/
        }
    }
#endif
    /*double roots[] = {0.1,0.2,0.6};
    Poly prod = roots_to_poly(roots, sizeof(roots)/sizeof(double));
    std::cout << "real solve\n";
    std::cout << prod 
              << " solves as ";
    std::vector<std::complex<double> > prod_root = solve(prod);
    copy(prod_root.begin(), prod_root.end(), 
         std::ostream_iterator<std::complex<double> >(std::cout, ",\t"));
    std::cout << std::endl;
    
    SBasis prod_sb = poly_to_sbasis(prod);
    std::vector<double> bez = sbasis_to_bezier(prod_sb, prod_sb.size());
    
    copy(bez.begin(), bez.end(), 
         std::ostream_iterator<double>(std::cout, ",\t"));
         std::cout << std::endl;*/
    
    /*std::cout << "crossing count = " 
              << crossing_count(bez, bez.size())
              << std::endl;*/
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
