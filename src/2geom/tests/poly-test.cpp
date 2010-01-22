#include <2geom/poly.h>
#include <vector>
#include <iterator>


using namespace std;
using Geom::Poly;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

int main() {
    Poly a, b, r;
    
    a.push_back(1);
    a.push_back(2);
    a.push_back(1);
    
    b.push_back(1);
    b.push_back(1);
    
    std::cout << a <<std::endl;
    std::cout << b <<std::endl;
    std::cout << b*b <<std::endl;
    std::cout << b*b*b <<std::endl;
    
    std::cout << derivative(b*b*b) <<std::endl;
    std::cout << integral(derivative(b*b*b)) <<std::endl;
    
    Poly f, g;
    
    f.push_back(-42);
    f.push_back(0);
    f.push_back(-12);
    f.push_back(1);
    
    g.push_back(-3);
    g.push_back(1);
    
    
    cout << "divide (example from wikipedia): " << f << "/" << g << endl;
    std::cout << divide(f, g, r) << endl;
    cout << r << endl;

    g[0] = -3;
    g[1] = 1;
    g.push_back(1);
    
    cout << "divide (example from wikipedia): " << f << "/" << g << endl;
    std::cout << divide(f, g, r) << endl;
    cout << r << endl;

    // polynomial with a double root at -3
    Poly multiplicity = lin_poly(1, 3)*lin_poly(1, 3)*lin_poly(1,-4)*lin_poly(1,-7)*lin_poly(2,1);
    multiplicity.normalize();
    
    cout << "gcd - we're using a multiple factor poly against its derivative.\n";
    Poly gc = gcd(multiplicity, derivative(multiplicity));
    cout << "gcd(p, p') = " << gc << endl;
    
    
    Poly prod = lin_poly(1, 3)*lin_poly(1,-4)*lin_poly(1,-7)*lin_poly(2,1);
    
    std::cout << prod <<std::endl;

    return 0;
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
