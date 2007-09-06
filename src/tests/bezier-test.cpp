#include "bezier.h"
#include "poly.h"
#include <vector>
#include <iterator>

#include "poly-dk-solve.h"
//#include "poly-laguerre-solve.h"
//#include "poly-laguerre-solve.cpp"
#include "sturm.h"

//x^5*1 + x^4*1212.36 + x^3*-2137.83 + x^2*1357.77 + x^1*-366.403 + x^0*42.0846


using namespace std;


using namespace Geom;

inline std::ostream &operator<< (std::ostream &out_file, const Interval &bo) {
    out_file << "[" << bo[0] << ", " << bo[1] << "]";
    return out_file;
}

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

// Bezier::Bezier(const Bezier& b);
    cout << Bezier(hump) << endl;
    
// explicit Bezier(unsigned ord);
    cout << Bezier(10) << endl;
    
// Bezier(Coord c0, Coord c1);
    cout << Bezier(0.0,1.0) << endl;
    
//Bezier(Coord c0, Coord c1, Coord c2);
    cout << Bezier(0,1, 2) << endl;

//Bezier(Coord c0, Coord c1, Coord c2, Coord c3);
    cout << Bezier(0,1,2,3) << endl;

//unsigned degree();
    cout << hump.degree() << " == 2\n";

//unsigned size();
    cout << hump.size() << " == 3\n";

// Coord at0();
// Coord at1();
    cout << wiggle.at0() << "..." << wiggle.at1() << endl;

// Coord valueAt(double t);
    cout << wiggle.valueAt(0.5) << endl;
    
//Coord operator()(double t);
    cout << wiggle(0.5) << endl;
    
//SBasis toSBasis();
    cout << unit.toSBasis() << endl;
    cout << hump.toSBasis() << endl;
    cout << wiggle.toSBasis() << endl;

//Only mutator
//Coord &operator[](unsigned ix);
//Coord const &operator[](unsigned ix);
//void setPoint(unsigned ix, double val);
    Bezier bigun(10);
    bigun.setPoint(5,10.0);
    cout << bigun << endl;
    
    for(int i = 0; i < bigun.size(); i++)
        bigun[i] = double(rand() % 20) - 10;
    cout <<  bigun << endl;
    
//std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs);
//std::pair<Bezier, Bezier > subdivide(Coord t);

    cout << "std::vector<double> roots();\n";
    cout << bigun.roots() << endl;
    
/*
Bezier operator+(const Bezier & a, double v);
Bezier operator-(const Bezier & a, double v);
Bezier operator*(const Bezier & a, double v);
Bezier operator/(const Bezier & a, double v)k
*/
    cout << hump + 3 << endl;
    cout << hump - 3 << endl;
    cout << hump*3 << endl;
    cout << hump/3 << endl;

// Bezier reverse(const Bezier & a)
    cout << reverse(Bezier(0,1,2,3)) << endl;

    cout << "Bezier portion(const Bezier & a, double from, double to);\n";
    cout << portion(Bezier(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<Bezier > & a) {

    cout << "Bezier derivative(const Bezier & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;
    std::cout << derivative(integral(hump)) <<std::endl;

// Interval bounds_fast(Bezier const & b) {
    std::cout << bounds_fast(hump) << endl;

// Interval bounds_exact(Bezier const & b) {
    cout << bounds_exact(hump) << " == [0, " << hump.valueAt(0.5) << "]\n";

// Interval bounds_local(Bezier const & b, Interval i) {
    cout << bounds_local(hump, Interval(0.3, 0.6)) << endl;
    
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
