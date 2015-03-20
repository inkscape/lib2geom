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

// Constructs a linear Bezier with root at t
Bezier linear_root(double t) {
    return Bezier(0-t, 1-t);
}

// Constructs a Bezier with roots at the locations in x
Bezier array_roots(vector<double> x) {
    Bezier b(1);
    for(unsigned i = 0; i < x.size(); i++) {
        b = multiply(b, linear_root(x[i]));
    }
    return b;
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
    expect_array((const double[]){0, 0.5, 0.5}, wiggle.roots());
    
    /*Bezier bigun(Bezier::Order(30));
    for(unsigned i = 0; i < bigun.size(); i++) {
        bigun.setCoeff(i,rand()-0.5);
    }
    cout << bigun.roots() << endl;*/

    // The results of our rootfinding are at the moment fairly inaccurate.
    double eps = 5e-4;

    vector<vector<double> > tests;
    tests.push_back(vector_from_array((const double[]){0}));
    tests.push_back(vector_from_array((const double[]){1}));
    tests.push_back(vector_from_array((const double[]){0, 0}));
    tests.push_back(vector_from_array((const double[]){0.5}));
    tests.push_back(vector_from_array((const double[]){0.5, 0.5}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.25,0.75}));
    tests.push_back(vector_from_array((const double[]){0.5,0.5}));
    tests.push_back(vector_from_array((const double[]){0, 0.2, 0.6, 0.6, 1}));
    tests.push_back(vector_from_array((const double[]){.1,.2,.3,.4,.5,.6}));
    tests.push_back(vector_from_array((const double[]){0.25,0.25,0.25,0.75,0.75,0.75}));
    
    for(unsigned test_i = 0; test_i < tests.size(); test_i++) {
        Bezier b = array_roots(tests[test_i]);
        std::cout << tests[test_i] << ": " << b << std::endl;
        std::cout << b.roots() << std::endl;
        vector_equal(tests[test_i], b.roots(), eps);
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

#if 0
struct IntersectionTestcase {
    D2<Bezier> a;
    D2<Bezier> b;
    std::vector<Point> x;
    std::vector<Coord> ta;
    std::vector<Coord> tb;
};

TEST_F(BezierTest, Intersection) {
    // test cases taken from:
    // D. Lasser, Calculating the self-intersections of Bezier curves

    Coord a1x[] = {-3.3, -3.3, 0, 3.3, 3.3};
    Coord a1y[] = {1.3, -0.7, 2.3 -0.7, 1.3};
    Coord b1x[] = {-4.0, -4.0, 0, 4.0, 4.0};
    Coord b1y[] = {-0.35, 3.0, -2.6, 3.0, -0.35};
    Coord p1x[] = {-3.12109, -1.67341, 1.67341, 3.12109};
    Coord p1y[] = {0.76362, 0.60298, 0.60298, 0.76362};
    Coord p1ta[] = {0.09834, 0.32366, 0.67634, 0.90166};
    Coord p1tb[] = {0.20604, 0.35662, 0.64338, 0.79396};

    Coord a2x[] = {0, 0, 3, 3};
    Coord a2y[] = {0, 14, -9, 5};
    Coord b2x[] = {-1, 13, -10, 4};
    Coord b2y[] = {4, 4, 1, 1};
    Coord p2x[] = {0.00809, 0.02596, 0.17250, 0.97778, 1.50000, 2.02221, 2.82750, 2.97404, 2.99191};
    Coord p2y[] = {1.17249, 1.97778, 3.99191, 3.97404, 2.50000, 1.02596, 1.00809, 3.02221, 3.82750};
    Coord p2ta[] = {0.03029, 0.05471, 0.14570, 0.38175, 0.50000, 0.61825, 0.85430, 0.94529, 0.96971};
    Coord p2tb[] = {0.85430, 0.61825, 0.03029, 0.05471, 0.50000, 0.94529, 0.96971, 0.38175, 0.14570};

    Coord a3x[] = {-5, -5, -3, 0, 3, 5, 5};
    Coord a3y[] = {0, 3.555, -1, 4.17, -1, 3.555, 0};
    Coord b3x[] = {-6, -6, -3, 0, 3, 6, 6};
    Coord b3y[] = {3, -0.555, 4, -1.17, 4, -0.555, 3};
    Coord p3x[] = {-3.64353, -2.92393, -0.77325, 0.77325, 2.92393, 3.64353};
    Coord p3y[] = {1.49822, 1.50086, 1.49989, 1.49989, 1.50086, 1.49822};
    Coord p3ta[] = {0.23120, 0.29330, 0.44827, 0.55173, 0.70670, 0.76880};
    Coord p3tb[] = {0.27305, 0.32148, 0.45409, 0.54591, 0.67852, 0.72695};

    Coord a4x[] = {-4, -10, -2, -2, 2, 2, 10, 4};
    Coord a4y[] = {0, 6, 6, 0, 0, 6, 6, 0};
    Coord b4x[] = {-8, 0, 8};
    Coord b4y[] = {1, 6, 1};
    Coord p4x[] = {-5.69310, -2.68113, 2.68113, 5.69310};
    Coord p4y[] = {2.23393, 3.21920, 3.21920, 2.23393};
    Coord p4ta[] = {0.14418, 0.33243, 0.66757, 0.85582};
    Coord p4tb[] = {0.06613, 0.35152, 0.64848, 0.93387};

    Coord a5x[] = {-1.5, -1.5, -10, -10, 0, 10, 10, 1.5, 1.5};
    Coord a5y[] = {0, -8, -8, 9, 9, 9, -8, -8, 0};
    Coord b5x[] = {-3, -12, 0, 12, 3};
    Coord b5y[] = {-5, 8, 2.062507, 8, -5};
    Coord p5x[] = {-3.60359, -5.44653, 0, 5.44653, 3.60359};
    Coord p5y[] = {-4.10631, -0.76332, 4.14844, -0.76332, -4.10631};
    Coord p5ta[] = {0.01787, 0.10171, 0.5, 0.89829, 0.98213};
    Coord p5tb[] = {0.12443, 0.28110, 0.5, 0.71890, 0.87557};

    Coord a6x[] = {5, 14, 10, -12, -12, -2};
    Coord a6y[] = {1, 6, -6, -6, 2, 2};
    Coord b6x[] = {0, 2, -10.5, -10.5, 3.5, 3, 8, 6};
    Coord b6y[] = {0, -8, -8, 9, 9, -4.129807, -4.129807, 3};
    Coord p6x[] = {6.29966, 5.87601, 0.04246, -4.67397, -3.57214};
    Coord p6y[] = {1.63288, -0.86192, -2.38219, -2.17973, 1.91463};
    Coord p6ta[] = {0.03184, 0.33990, 0.49353, 0.62148, 0.96618};
    Coord p6tb[] = {0.96977, 0.85797, 0.05087, 0.28232, 0.46102};
}
#endif

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
