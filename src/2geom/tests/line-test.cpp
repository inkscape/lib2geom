#include "testing.h"
#include <iostream>

#include <2geom/line.h>
#include <2geom/affine.h>

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

TEST(LineTest, Reflection) {
    Line a(Point(10, 0), Point(15,5));
    Point pa(10,5), ra(15,0);

    Line b(Point(1,-2), Point(2,0));
    Point pb(5,1), rb(1,3);
    Affine reflecta = a.reflection(), reflectb = b.reflection();

    Point testra = pa * reflecta;
    Point testrb = pb * reflectb;
    EXPECT_FLOAT_EQ(testra[X], ra[X]);
    EXPECT_FLOAT_EQ(testra[Y], ra[Y]);
    EXPECT_FLOAT_EQ(testrb[X], rb[X]);
    EXPECT_FLOAT_EQ(testrb[Y], rb[Y]);
}
