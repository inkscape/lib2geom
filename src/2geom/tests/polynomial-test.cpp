#include "testing.h"
#include <iostream>
#include <glib.h>

#include <2geom/polynomial.h>

using namespace Geom;

TEST(PolynomialTest, SolveQuadratic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2);
        Coord c = a * x1 * x2;

        std::vector<Coord> result = solve_quadratic(a, b, c);

        EXPECT_EQ(result.size(), 2);
        if (x1 < x2) {
            EXPECT_FLOAT_EQ(result[0], x1);
            EXPECT_FLOAT_EQ(result[1], x2);
        } else {
            EXPECT_FLOAT_EQ(result[0], x2);
            EXPECT_FLOAT_EQ(result[1], x1);
        }
    }
}

TEST(PolynomialTest, SolveCubic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);
        Coord x3 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2 + x3);
        Coord c = a * (x1*x2 + x2*x3 + x1*x3);
        Coord d = -a * x1 * x2 * x3;

        std::vector<Coord> result = solve_cubic(a, b, c, d);
        std::vector<Coord> x(3); x[0] = x1; x[1] = x2; x[2] = x3;
        std::sort(x.begin(), x.end());

        EXPECT_EQ(result.size(), 3);
        EXPECT_FLOAT_EQ(result[0], x[0]);
        EXPECT_FLOAT_EQ(result[1], x[1]);
        EXPECT_FLOAT_EQ(result[2], x[2]);
    }
}
