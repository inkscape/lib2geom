#include "testing.h"
#include <iostream>

#include <2geom/bezier.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace Geom;

bool are_equal(SBasis A, SBasis B) {
    int maxSize = max(A.size(), B.size());
    double t = 0., dt = 1./maxSize;
    
    for(int i = 0; i <= maxSize; i++) {
        EXPECT_FLOAT_EQ(A.valueAt(t), B.valueAt(t));// return false;
        t += dt;
    }
    return true;
}

namespace {

// The fixture for testing class Foo.
class SBasisTest : public ::testing::Test {
protected:
    friend class Geom::SBasis;
    SBasisTest() {
        // You can do set-up work for each test here.
        zero = SBasis(Bezier(0.0).toSBasis());
        unit = SBasis(Bezier(0.0,1.0).toSBasis());
        hump = SBasis(Bezier(0,1,0).toSBasis());
        wiggle = SBasis(Bezier(0,1,-2,3).toSBasis());
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    SBasis zero, unit, hump, wiggle;

};

TEST_F(SBasisTest, UnitTests) {
    EXPECT_TRUE(Bezier(0,0,0,0).toSBasis().isZero());
    EXPECT_TRUE(Bezier(0,1,2,3).toSBasis().isFinite());

    EXPECT_EQ(3, Bezier(0,2,4,5).toSBasis().size());
    EXPECT_EQ(3, hump.size());
}

TEST_F(SBasisTest, ValueAt) {
    EXPECT_EQ(0.0, wiggle.at0());
    EXPECT_EQ(3.0, wiggle.at1());
    EXPECT_EQ(0.0, wiggle.valueAt(0.5));
    EXPECT_EQ(0.0, wiggle(0.5));
}

TEST_F(SBasisTest, MultiDerivative) {
    vector<double> vnd = wiggle.valueAndDerivatives(0.5, 5);
    expect_array((const double[]){0,0,12,72,0,0}, vnd);
}
  /*
TEST_F(SBasisTest, DegreeElevation) {
    EXPECT_TRUE(are_equal(wiggle, wiggle));
    SBasis Q = wiggle;
    SBasis P = Q.elevate_degree();
    EXPECT_EQ(P.size(), Q.size()+1);
    //EXPECT_EQ(0, P.forward_difference(1)[0]);
    EXPECT_TRUE(are_equal(Q, P));
    Q = wiggle;
    P = Q.elevate_to_degree(10);
    EXPECT_EQ(10, P.order());
    EXPECT_TRUE(are_equal(Q, P));
    //EXPECT_EQ(0, P.forward_difference(10)[0]);
}*/
//std::pair<SBasis, SBasis > subdivide(Coord t);

SBasis linear_root(double t) {
  return SBasis(Linear(0-t, 1-t));
}

SBasis array_roots(vector<double> x) {
    SBasis b(1);
    for(unsigned i = 0; i < x.size(); i++) {
        b = multiply(b, linear_root(x[i]));
    }
    return b;
}

template <typename T, int xn>
vector<T> vector_from_array(const T (&x)[xn]) {
    vector<T> v;
    for(int i = 0; i < xn; i++) {
        v.push_back(x[i]);
    }
    return v;
}

Interval bound_vector(vector<double> v) {
    double low = v[0];
    double high = v[0];
    for(unsigned i = 0; i < v.size(); i++) {
        low = min(v[i], low);
        high = max(v[i], high);
    }
    return Interval(low-1, high-1);
}

void vector_equal(vector<double> a, vector<double> b) {
    EXPECT_EQ(a.size(), b.size());
    if(a.size() != b.size()) return;
    for(unsigned i = 0; i < a.size(); i++) {
        EXPECT_FLOAT_EQ(a[i], b[i]);
    }
}

vector<double> find_all_roots(SBasis b) {
    vector<double> rts = roots(b);
    if(b.at0() == 0) rts.push_back(0);
    if(b.at1() == 0) rts.push_back(1);
    return rts;
}

  /*TEST_F(SBasisTest, Deflate) {
    SBasis b = array_roots(vector_from_array((const double[]){0,0.25,0.5}));
    EXPECT_FLOAT_EQ(0, b.at0());
    b = b.deflate();
    EXPECT_FLOAT_EQ(0, b.valueAt(0.25));
    b = b.subdivide(0.25).second;
    EXPECT_FLOAT_EQ(0, b.at0());
    b = b.deflate();
    const double rootposition = (0.5-0.25) / (1-0.25);
    EXPECT_FLOAT_EQ(0, b.valueAt(rootposition));
    b = b.subdivide(rootposition).second;
    EXPECT_FLOAT_EQ(0, b.at0());
}*/

TEST_F(SBasisTest, Roots) {
    expect_array((const double[]){0.5}, roots(wiggle));
    
    vector<vector<double> > tests;
    tests.push_back(vector_from_array((const double[]){0}));
    tests.push_back(vector_from_array((const double[]){0.5}));
    tests.push_back(vector_from_array((const double[]){0.25,0.75}));
    tests.push_back(vector_from_array((const double[]){0.5,0.5}));
    tests.push_back(vector_from_array((const double[]){0, 0.2, 0.6,0.6, 1}));
    tests.push_back(vector_from_array((const double[]){.1,.2,.3,.4,.5,.6}));
    tests.push_back(vector_from_array((const double[]){0.25,0.25,0.25,0.75,0.75,0.75}));
    
    for(unsigned test_i = 0; test_i < tests.size(); test_i++) {
        SBasis b = array_roots(tests[test_i]);
        vector_equal(tests[test_i], find_all_roots(b));
    }

    vector<Linear> broken;
    broken.push_back(Linear(0, 42350.1));
    broken.push_back(Linear(-71082.3, -67071.5));
    broken.push_back(Linear(1783.41, 796047));
    SBasis b(broken);
    Bezier bz;
    sbasis_to_bezier(bz, b);
    cout << "roots(SBasis(broken))\n";
    for(int i = 0; i < 10; i++) {
      double t = i*0.01 + 0.1;
      cout << b(t) << "," << bz(t) << endl;
    }
    cout << roots(b) << endl;
    EXPECT_EQ(0, bz[0]);
    //bz = bz.deflate();
    cout << bz << endl;
    cout << bz.roots() << endl;
}

TEST_F(SBasisTest,Operators) {
    cout << "scalar operators\n";
    cout << hump + 3 << endl;
    cout << hump - 3 << endl;
    cout << hump*3 << endl;
    cout << hump/3 << endl;

    SBasis reverse_wiggle = reverse(wiggle);
    EXPECT_EQ(reverse_wiggle[0], wiggle[wiggle.size()-1]);
    EXPECT_TRUE(are_equal(reverse(reverse_wiggle), wiggle));

    cout << "SBasis portion(const SBasis & a, double from, double to);\n";
    cout << portion(SBasis(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<SBasis > & a) {

    cout << "SBasis derivative(const SBasis & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;

    EXPECT_TRUE(are_equal(derivative(integral(wiggle)), wiggle));
    std::cout << derivative(integral(hump)) <<std::endl;
    expect_array((const double []){0.5}, roots(derivative(hump)));

    EXPECT_TRUE(bounds_fast(hump)->contains(Interval(0,hump.valueAt(0.5))));

    EXPECT_EQ(Interval(0,hump.valueAt(0.5)), *bounds_exact(hump));

    Interval tight_local_bounds(min(hump.valueAt(0.3),hump.valueAt(0.6)),
             hump.valueAt(0.5));
    EXPECT_TRUE(bounds_local(hump, Interval(0.3, 0.6))->contains(tight_local_bounds));

    SBasis Bs[] = {unit, hump, wiggle};
    for(unsigned i = 0; i < sizeof(Bs)/sizeof(SBasis); i++) {
        SBasis B = Bs[i];
        SBasis product = multiply(B, B);
        for(int i = 0; i <= 16; i++) {
            double t = i/16.0;
            double b = B.valueAt(t);
            EXPECT_FLOAT_EQ(b*b, product.valueAt(t));
        }
    }
}

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
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
