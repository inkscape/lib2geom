#include "testing.h"
#include <iostream>

#include <2geom/bezier.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <vector>
#include <iterator>
#include <glib.h>

using namespace std;
using namespace Geom;

bool are_equal(SBasis const &A, SBasis const &B) {
    int maxSize = max(A.size(), B.size());
    double t = 0., dt = 1./maxSize;
    
    for(int i = 0; i <= maxSize; i++) {
        EXPECT_FLOAT_EQ(A.valueAt(t), B.valueAt(t));// return false;
        t += dt;
    }
    return true;
}

class SBasisTest : public ::testing::Test {
protected:
    friend class Geom::SBasis;
    SBasisTest()
        : zero(fragments[0])
        , unit(fragments[1])
        , hump(fragments[2])
        , wiggle(fragments[3])
    {
        zero = SBasis(Bezier(0.0).toSBasis());
        unit = SBasis(Bezier(0.0,1.0).toSBasis());
        hump = SBasis(Bezier(0,1,0).toSBasis());
        wiggle = SBasis(Bezier(0,1,-2,3).toSBasis());
    }

    SBasis fragments[4];
    SBasis &zero, &unit, &hump, &wiggle;
};

TEST_F(SBasisTest, UnitTests) {
    EXPECT_TRUE(Bezier(0,0,0,0).toSBasis().isZero());
    EXPECT_TRUE(Bezier(0,1,2,3).toSBasis().isFinite());

    // note: "size" of sbasis equals half the number of coefficients
    EXPECT_EQ(2u, Bezier(0,2,4,5).toSBasis().size());
    EXPECT_EQ(2u, hump.size());
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
    expect_array((const double[]){0, 0.5, 0.5}, roots(wiggle));
    
    // The results of our rootfinding are at the moment fairly inaccurate.
    double eps = 5e-4;

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
        std::cout << tests[test_i] << ": " << b << std::endl;
        std::cout << roots(b) << std::endl;
        EXPECT_vector_near(tests[test_i], roots(b), eps);
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

TEST_F(SBasisTest, Subdivide) {
    std::vector<std::pair<SBasis, double> > errors;
    for (unsigned i = 0; i < 10000; ++i) {
        double t = g_random_double_range(0, 1e-6);
        for (unsigned i = 0; i < 4; ++i) {
            SBasis &input = fragments[i];
            std::pair<SBasis, SBasis> result;
            result.first = portion(input, 0, t);
            result.second = portion(input, t, 1);

            // the endpoints must correspond exactly
            EXPECT_EQ(result.first.at0(), input.at0());
            EXPECT_EQ(result.first.at1(), result.second.at0());
            EXPECT_EQ(result.second.at1(), input.at1());

            // ditto for valueAt
            EXPECT_EQ(result.first.valueAt(0), input.valueAt(0));
            EXPECT_EQ(result.first.valueAt(1), result.second.valueAt(0));
            EXPECT_EQ(result.second.valueAt(1), input.valueAt(1));

            if (result.first.at1() != result.second.at0()) {
                errors.push_back(std::pair<SBasis,double>(input, t));
            }
        }
    }
    if (!errors.empty()) {
        std::cout << "Found " << errors.size() << " subdivision errors" << std::endl;
        for (unsigned i = 0; i < errors.size(); ++i) {
            std::cout << "Error #" << i << ":\n"
                          << "SBasis: " << errors[i].first << "\n"
                          << "t: " << format_coord_nice(errors[i].second) << std::endl;
        }
    }
}

TEST_F(SBasisTest, Reverse) {
    SBasis reverse_wiggle = reverse(wiggle);
    EXPECT_EQ(reverse_wiggle.at0(), wiggle.at1());
    EXPECT_EQ(reverse_wiggle.at1(), wiggle.at0());
    EXPECT_EQ(reverse_wiggle.valueAt(0.5), wiggle.valueAt(0.5));
    EXPECT_EQ(reverse_wiggle.valueAt(0.25), wiggle.valueAt(0.75));
    EXPECT_TRUE(are_equal(reverse(reverse_wiggle), wiggle));
}

TEST_F(SBasisTest,Operators) {
    //cout << "scalar operators\n";
    //cout << hump + 3 << endl;
    //cout << hump - 3 << endl;
    //cout << hump*3 << endl;
    //cout << hump/3 << endl;

    //cout << "SBasis derivative(const SBasis & a);\n";
    //std::cout << derivative(hump) <<std::endl;
    //std::cout << integral(hump) <<std::endl;

    EXPECT_TRUE(are_equal(derivative(integral(wiggle)), wiggle));
    //std::cout << derivative(integral(hump)) << std::endl;
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
