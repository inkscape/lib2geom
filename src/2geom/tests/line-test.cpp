#include "testing.h"
#include <iostream>

#include <2geom/line.h>

using namespace Geom;

TEST(LineTest, AngleBisector) {
    Point o(0,0), a(1,1), b(3,0), c(-4, 0);
    Point d(0.5231, 0.75223);

    // normal
    Line ab1 = make_angle_bisector_line(a + d, o + d, b + d);
    Line ab2 = make_angle_bisector_line(a - d, o - d, b - d);
    EXPECT_FLOAT_EQ(ab1.angle(), Angle::from_degrees(22.5));
    EXPECT_FLOAT_EQ(ab2.angle(), Angle::from_degrees(22.5));

    // half angle
    Line bc1 = make_angle_bisector_line(b + d, o + d, c + d);
    Line bc2 = make_angle_bisector_line(b - d, o - d, c - d);
    EXPECT_FLOAT_EQ(bc1.angle(), Angle::from_degrees(90));
    EXPECT_FLOAT_EQ(bc2.angle(), Angle::from_degrees(90));

    // zero angle
    Line aa1 = make_angle_bisector_line(a + d, o + d, a + d);
    Line aa2 = make_angle_bisector_line(a - d, o - d, a - d);
    EXPECT_FLOAT_EQ(aa1.angle(), Angle::from_degrees(45));
    EXPECT_FLOAT_EQ(aa2.angle(), Angle::from_degrees(45));
}

TEST(LineTest, Equality) {
    Line a(Point(0,0), Point(2,2));
    Line b(Point(2,2), Point(5,5));

    EXPECT_EQ(a, a);
    EXPECT_EQ(b, b);
    EXPECT_EQ(a, b);
}
