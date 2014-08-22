#include "testing.h"
#include <iostream>

#include <2geom/bezier.h>
#include <2geom/poly.h>
#include <vector>
#include <iterator>
#include <glib.h>

using namespace std;
using namespace Geom;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

bool are_equal(Bezier A, Bezier B) {
    int maxSize = max(A.size(), B.size());
    double t = 0., dt = 1./maxSize;
    
    for(int i = 0; i <= maxSize; i++) {
        EXPECT_FLOAT_EQ(A.valueAt(t), B.valueAt(t));// return false;
        t += dt;
    }
    return true;
}

class BezierTest : public ::testing::Test {
protected:

    BezierTest()
        : zero(fragments[0])
        , unit(fragments[1])
        , hump(fragments[2])
        , wiggle(fragments[3])
    {
        zero = Bezier(0.0,0.0);
        unit = Bezier(0.0,1.0);
        hump = Bezier(0,1,0);
        wiggle = Bezier(0,1,-2,3);
    }

    Bezier fragments[4];
    Bezier &zero, &unit, &hump, &wiggle;
};

TEST_F(BezierTest, Basics) {
  
    //std::cout << unit <<std::endl;
    //std::cout << hump <<std::endl;
    
    EXPECT_TRUE(Bezier(0,0,0,0).isZero());
    EXPECT_TRUE(Bezier(0,1,2,3).isFinite());

    EXPECT_EQ(3u, Bezier(0,2,4,5).order());

    ///cout << " Bezier::Bezier(const Bezier& b);\n";
    //cout << Bezier(wiggle) << " == " << wiggle << endl;
    
    //cout << "explicit Bezier(unsigned ord);\n";
    //cout << Bezier(10) << endl;
    
    //cout << "Bezier(Coord c0, Coord c1);\n";
    //cout << Bezier(0.0,1.0) << endl;
    
    //cout << "Bezier(Coord c0, Coord c1, Coord c2);\n";
    //cout << Bezier(0,1, 2) << endl;

    //cout << "Bezier(Coord c0, Coord c1, Coord c2, Coord c3);\n";
    //cout << Bezier(0,1,2,3) << endl;

    //cout << "unsigned degree();\n";
    EXPECT_EQ(2u, hump.degree());

    //cout << "unsigned size();\n";
    EXPECT_EQ(3u, hump.size());
}

TEST_F(BezierTest, ValueAt) {
    EXPECT_EQ(0.0, wiggle.at0());
    EXPECT_EQ(3.0, wiggle.at1());

    EXPECT_EQ(0.0, wiggle.valueAt(0.5));
    
    EXPECT_EQ(0.0, wiggle(0.5));
    
    //cout << "SBasis toSBasis();\n";
    //cout << unit.toSBasis() << endl;
    //cout << hump.toSBasis() << endl;
    //cout << wiggle.toSBasis() << endl;
}

TEST_F(BezierTest, Casteljau) {
    unsigned N = wiggle.order() + 1;
    std::vector<Coord> left(N), right(N);
    std::vector<Coord> left2(N), right2(N);

    for (unsigned i = 0; i < 10000; ++i) {
        double t = g_random_double_range(0, 1);
        double vok = bernstein_value_at(t, &wiggle[0], wiggle.order());
        double v = casteljau_subdivision<double>(t, &wiggle[0], &left[0], &right[0], wiggle.order());
        EXPECT_EQ(v, vok);
        EXPECT_EQ(left[0], wiggle.at0());
        EXPECT_EQ(left[wiggle.order()], right[0]);
        EXPECT_EQ(right[wiggle.order()], wiggle.at1());

        double vl = casteljau_subdivision<double>(t, &wiggle[0], &left2[0], NULL, wiggle.order());
        double vr = casteljau_subdivision<double>(t, &wiggle[0], NULL, &right2[0], wiggle.order());
        EXPECT_EQ(vl, vok);
        EXPECT_EQ(vr, vok);
        vector_equal(left2, left);
        vector_equal(right2, right);

        double vnone = casteljau_subdivision<double>(t, &wiggle[0], NULL, NULL, wiggle.order());
        EXPECT_EQ(vnone, vok);
    }
}

TEST_F(BezierTest, Portion) {
    for (unsigned i = 0; i < 10000; ++i) {
        double from = g_random_double_range(0, 1);
        double to = g_random_double_range(0, 1);
        for (unsigned i = 0; i < 4; ++i) {
            Bezier &input = fragments[i];
            Bezier result = portion(input, from, to);

            // the endpoints must correspond exactly
            EXPECT_EQ(result.at0(), input.valueAt(from));
            EXPECT_EQ(result.at1(), input.valueAt(to));
        }
    }
}

TEST_F(BezierTest, Subdivide) {
    std::vector<std::pair<Bezier, double> > errors;
    for (unsigned i = 0; i < 10000; ++i) {
        double t = g_random_double_range(0, 1e-6);
        for (unsigned i = 0; i < 4; ++i) {
            Bezier &input = fragments[i];
            std::pair<Bezier, Bezier> result = input.subdivide(t);

            // the endpoints must correspond exactly
            // moreover, the subdivision point must be exactly equal to valueAt(t)
            EXPECT_EQ(result.first.at0(), input.at0());
            EXPECT_EQ(result.first.at1(), result.second.at0());
            EXPECT_EQ(result.second.at0(), input.valueAt(t));
            EXPECT_EQ(result.second.at1(), input.at1());

            // ditto for valueAt
            EXPECT_EQ(result.first.valueAt(0), input.valueAt(0));
            EXPECT_EQ(result.first.valueAt(1), result.second.valueAt(0));
            EXPECT_EQ(result.second.valueAt(0), input.valueAt(t));
            EXPECT_EQ(result.second.valueAt(1), input.valueAt(1));

            if (result.first.at1() != result.second.at0()) {
                errors.push_back(std::pair<Bezier,double>(input, t));
            }
        }
    }
    if (!errors.empty()) {
        std::cout << "Found " << errors.size() << " subdivision errors" << std::endl;
        for (unsigned i = 0; i < errors.size(); ++i) {
            std::cout << "Error #" << i << ":\n"
                          << errors[i].first << "\n"
                          << "t: " << format_coord_nice(errors[i].second) << std::endl;
        }
    }
}

TEST_F(BezierTest, Mutation) {
//Coord &operator[](unsigned ix);
//Coord const &operator[](unsigned ix);
//void setCoeff(unsigned ix double val);
    //cout << "bigun\n";
    Bezier bigun(Bezier::Order(30));
    bigun.setCoeff(5,10.0);
    for(unsigned i = 0; i < bigun.size(); i++) {
        EXPECT_EQ((i == 5) ? 10 : 0, bigun[i]);
    }

    bigun[5] = -3;
    for(unsigned i = 0; i < bigun.size(); i++) {
        EXPECT_EQ((i == 5) ? -3 : 0, bigun[i]);
    }
}

TEST_F(BezierTest, MultiDerivative) {
    vector<double> vnd = wiggle.valueAndDerivatives(0.5, 5);
    expect_array((const double[]){0,0,12,72,0,0}, vnd);
}

TEST_F(BezierTest, DegreeElevation) {
    EXPECT_TRUE(are_equal(wiggle, wiggle));
    Bezier Q = wiggle;
    Bezier P = Q.elevate_degree();
    EXPECT_EQ(P.size(), Q.size()+1);
    //EXPECT_EQ(0, P.forward_difference(1)[0]);
    EXPECT_TRUE(are_equal(Q, P));
    Q = wiggle;
    P = Q.elevate_to_degree(10);
    EXPECT_EQ(10u, P.order());
    EXPECT_TRUE(are_equal(Q, P));
    //EXPECT_EQ(0, P.forward_difference(10)[0]);
    /*Q = wiggle.elevate_degree();
    P = Q.reduce_degree();
    EXPECT_EQ(P.size()+1, Q.size());
    EXPECT_TRUE(are_equal(Q, P));*/
}
//std::pair<Bezier, Bezier > subdivide(Coord t);

Bezier linear_root(double t) {
    return Bezier(0-t, 1-t);
}

Bezier array_roots(vector<double> x) {
    Bezier b(1);
    for(unsigned i = 0; i < x.size(); i++) {
        b = multiply(b, linear_root(x[i]));
    }
    return b;
}

vector<double> find_all_roots(Bezier b) {
    vector<double> rts = b.roots();
    return rts;
}

TEST_F(BezierTest, Deflate) {
    Bezier b = array_roots(vector_from_array((const double[]){0,0.25,0.5}));
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
}

TEST_F(BezierTest, Roots) {
    /*expect_array((const double[]){0.5}, wiggle.roots());
    
    Bezier bigun(Bezier::Order(30));
    for(unsigned i = 0; i < bigun.size(); i++) {
        bigun.setCoeff(i,rand()-0.5);
    }
    cout << bigun.roots() << endl;*/

    vector<vector<double> > tests;
    tests.push_back(vector_from_array((const double[]){0}));
    tests.push_back(vector_from_array((const double[]){0, 0}));
    tests.push_back(vector_from_array((const double[]){0.5}));
    tests.push_back(vector_from_array((const double[]){0.5, 0.5}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.25,0.75}));
    tests.push_back(vector_from_array((const double[]){0.5,0.5}));
    tests.push_back(vector_from_array((const double[]){0, 0.2, 0.6,0.6, 1}));
    tests.push_back(vector_from_array((const double[]){.1,.2,.3,.4,.5,.6}));
    tests.push_back(vector_from_array((const double[]){0.25,0.25,0.25,0.75,0.75,0.75}));
    
    for(unsigned test_i = 0; test_i < tests.size(); test_i++) {
        Bezier b = array_roots(tests[test_i]);
        std::cout << tests[test_i] << ": " << b << std::endl;
        vector_equal(tests[test_i], find_all_roots(b));
    }
}

TEST_F(BezierTest,Operators) {
    /*cout << "scalar operators\n";
    cout << hump + 3 << endl;
    cout << hump - 3 << endl;
    cout << hump*3 << endl;
    cout << hump/3 << endl;*/

    Bezier reverse_wiggle = reverse(wiggle);
    EXPECT_EQ(reverse_wiggle.at0(), wiggle.at1());
    EXPECT_EQ(reverse_wiggle.at1(), wiggle.at0());
    EXPECT_TRUE(are_equal(reverse(reverse_wiggle), wiggle));

    cout << "Bezier portion(const Bezier & a, double from, double to);\n";
    cout << portion(Bezier(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<Bezier > & a) {

    /*cout << "Bezier derivative(const Bezier & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;*/

    EXPECT_TRUE(are_equal(derivative(integral(wiggle)), wiggle));
    //std::cout << derivative(integral(hump)) <<std::endl;
    expect_array((const double []){0.5}, derivative(hump).roots());

    EXPECT_TRUE(bounds_fast(hump)->contains(Interval(0,hump.valueAt(0.5))));

    EXPECT_EQ(Interval(0,hump.valueAt(0.5)), *bounds_exact(hump));

    Interval tight_local_bounds(min(hump.valueAt(0.3),hump.valueAt(0.6)),
             hump.valueAt(0.5));
    EXPECT_TRUE(bounds_local(hump, Interval(0.3, 0.6))->contains(tight_local_bounds));

    Bezier Bs[] = {unit, hump, wiggle};
    for(unsigned i = 0; i < sizeof(Bs)/sizeof(Bezier); i++) {
        Bezier B = Bs[i];
        Bezier product = multiply(B, B);
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
