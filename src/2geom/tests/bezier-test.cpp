#include <iostream>

#include "bezier.h"
#include "poly.h"
#include <vector>
#include <iterator>

using namespace std;


using namespace Geom;

// streams out a vector
template <class T>
std::ostream&
operator<< (std::ostream &out, const std::vector<T,
             std::allocator<T> > &v)
{
    typedef std::ostream_iterator<T, char,
    std::char_traits<char> > Iter;

    std::copy (v.begin (), v.end (), Iter (out, " "));

    return out;
}

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

int
main(int argc, char** argv) {
    Bezier unit(0.0,1.0), hump(0,1,0), wiggle(0,1,-2,3);
  
    std::cout << unit <<std::endl;
    std::cout << hump <<std::endl;
    
// bool isZero();
    assert(Bezier(0,0,0,0).isZero());
// bool isFinite();
    assert(Bezier(0,1,2,3).isFinite());

/* Todo:

Coord subdivideArr(Coord t, Coord const *v, Coord *left, Coord *right, unsigned order);
*/
    cout << Bezier(0,2,4,5).order() << endl;

    cout << " Bezier::Bezier(const Bezier& b);\n";
    cout << Bezier(wiggle) << " == " << wiggle << endl;
    
    cout << "explicit Bezier(unsigned ord);\n";
    cout << Bezier(10) << endl;
    
    cout << "Bezier(Coord c0, Coord c1);\n";
    cout << Bezier(0.0,1.0) << endl;
    
    cout << "Bezier(Coord c0, Coord c1, Coord c2);\n";
    cout << Bezier(0,1, 2) << endl;

    cout << "Bezier(Coord c0, Coord c1, Coord c2, Coord c3);\n";
    cout << Bezier(0,1,2,3) << endl;

    cout << "unsigned degree();\n";
    cout << hump.degree() << " == 2\n";

    cout << "unsigned size();\n";
    cout << hump.size() << " == 3\n";

    cout << "Coord at0();  Coord at1();\n";
    cout << wiggle.at0() << "..." << wiggle.at1() << endl;

    cout << "Coord valueAt(double t);\n";
    cout << wiggle.valueAt(0.5) << endl;
    
    cout << "Coord operator()(double t);\n";
    cout << wiggle(0.5) << endl;
    
    cout << "SBasis toSBasis();\n";
    cout << unit.toSBasis() << endl;
    cout << hump.toSBasis() << endl;
    cout << wiggle.toSBasis() << endl;

//Only mutator
//Coord &operator[](unsigned ix);
//Coord const &operator[](unsigned ix);
//void setPoint(unsigned ix double val);
    cout << "bigun\n";
    Bezier bigun(Bezier::Order(30));
    bigun.setPoint(5,10.0);
    cout << bigun << endl;
    
    for(unsigned i = 0; i < bigun.size(); i++)
        bigun[i] = double(rand() % 20) - 10;
    cout <<  bigun << endl;
    
    cout << "std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs);\n";
    //wiggle = Bezier(1,-3,4,5);
    cout << "value at 0.5 = " << wiggle.valueAt(0.5) << endl;
    cout << wiggle.valueAndDerivatives(0.5, 5) << endl;
    Bezier w = wiggle;
    while(w.size()) {
        cout << w.valueAt(0.5) << ", ";
        w = derivative(w);
    }
    cout << endl;
    
    
//std::pair<Bezier, Bezier > subdivide(Coord t);

    cout << "std::vector<double> roots();\n";
    cout << wiggle.roots() << endl;
    cout << bigun.roots() << endl;
    
/*
Bezier operator+(const Bezier & a, double v);
Bezier operator-(const Bezier & a, double v);
Bezier operator*(const Bezier & a, double v);
Bezier operator/(const Bezier & a, double v)k
*/
    cout << "scalar operators\n";
    cout << hump + 3 << endl;
    cout << hump - 3 << endl;
    cout << hump*3 << endl;
    cout << hump/3 << endl;

    cout << "Bezier reverse(const Bezier & a);\n";
    cout << reverse(Bezier(0,1,2,3)) << endl;

    cout << "Bezier portion(const Bezier & a, double from, double to);\n";
    cout << portion(Bezier(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<Bezier > & a) {

    cout << "Bezier derivative(const Bezier & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;
    std::cout << derivative(integral(hump)) <<std::endl;
    std::cout << derivative(hump).roots() <<std::endl;

    cout << "Interval bounds_fast(Bezier const & b);\n";
    std::cout << *bounds_fast(hump) << endl;

    cout << "Interval bounds_exact(Bezier const & b);\n";
    cout << *bounds_exact(hump) << " == [0, " << hump.valueAt(0.5) << "]\n";

    cout << "Interval bounds_local(Bezier const & b, Interval i);\n";
    cout << *bounds_local(hump, Interval(0.3, 0.6)) << endl;
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
