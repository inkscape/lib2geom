#include "testing.h"
#include <iostream>
#include <glib.h>

#include <2geom/angle.h>
#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <memory>

using namespace Geom;

TEST(EllipseTest, Arcs) {
    Ellipse e(Point(5,10), Point(5, 10), 0);

    std::auto_ptr<EllipticalArc> arc1(e.arc(Point(5,0), Point(0,0), Point(0,10)));

    EXPECT_EQ(arc1->initialPoint(), Point(5,0));
    EXPECT_EQ(arc1->finalPoint(), Point(0,10));
    EXPECT_EQ(arc1->boundsExact(), Rect::from_xywh(0,0,5,10));
    EXPECT_EQ(arc1->center(), e.center());
    EXPECT_EQ(arc1->largeArc(), false);
    EXPECT_EQ(arc1->sweep(), false);

    std::auto_ptr<EllipticalArc> arc1r(e.arc(Point(0,10), Point(0,0), Point(5,0)));

    EXPECT_EQ(arc1r->boundsExact(), arc1->boundsExact());
    EXPECT_EQ(arc1r->sweep(), true);
    EXPECT_EQ(arc1r->largeArc(), false);

    std::auto_ptr<EllipticalArc> arc2(e.arc(Point(5,0), Point(10,20), Point(0,10)));

    EXPECT_EQ(arc2->boundsExact(), Rect::from_xywh(0,0,10,20));
    EXPECT_EQ(arc2->largeArc(), true);
    EXPECT_EQ(arc2->sweep(), true);

    std::auto_ptr<EllipticalArc> arc2r(e.arc(Point(0,10), Point(10,20), Point(5,0)));

    EXPECT_EQ(arc2r->boundsExact(), arc2->boundsExact());
    EXPECT_EQ(arc2r->largeArc(), true);
    EXPECT_EQ(arc2r->sweep(), false);
}

TEST(EllipseTest, AreNear) {
    Ellipse e1(Point(5.000001,10), Point(5,10), Angle::from_degrees(45));
    Ellipse e2(Point(5.000000,10), Point(5,10), Angle::from_degrees(225));
    Ellipse e3(Point(4.999999,10), Point(10,5), Angle::from_degrees(135));
    Ellipse e4(Point(5.000001,10), Point(10,5), Angle::from_degrees(315));

    EXPECT_TRUE(are_near(e1, e2, 1e-5));
    EXPECT_TRUE(are_near(e1, e3, 1e-5));
    EXPECT_TRUE(are_near(e1, e4, 1e-5));

    Ellipse c1(Point(20.000001,35.000001), Point(5.000001,4.999999), Angle::from_degrees(180.00001));
    Ellipse c2(Point(19.999999,34.999999), Point(4.999999,5.000001), Angle::from_degrees(179.99999));
    //std::cout << c1 << "\n" << c2 << std::endl;
    EXPECT_TRUE(are_near(c1, c2, 2e-5));

    EXPECT_FALSE(are_near(c1, e1, 1e-5));
    EXPECT_FALSE(are_near(c2, e1, 1e-5));
    EXPECT_FALSE(are_near(c1, e2, 1e-5));
    EXPECT_FALSE(are_near(c2, e2, 1e-5));
    EXPECT_FALSE(are_near(c1, e3, 1e-5));
    EXPECT_FALSE(are_near(c2, e3, 1e-5));
    EXPECT_FALSE(are_near(c1, e4, 1e-5));
    EXPECT_FALSE(are_near(c2, e4, 1e-5));
}

TEST(EllipseTest, Transformations) {
    Ellipse e(Point(5,10), Point(5,10), Angle::from_degrees(45));

    Ellipse er = e * Rotate::around(Point(5,10), Angle::from_degrees(45));
    Ellipse ercmp(Point(5,10), Point(5,10), Angle::from_degrees(90));
    //std::cout << e << "\n" << er << "\n" << ercmp << std::endl;
    EXPECT_TRUE(are_near(er, ercmp, 1e-12));

    Ellipse eflip = e * Affine(Scale(-1,1));
    Ellipse eflipcmp(Point(-5, 10), Point(5,10), Angle::from_degrees(135));
    EXPECT_TRUE(are_near(eflip, eflipcmp, 1e-12));
}

TEST(EllipseTest, TimeAt) {
    Ellipse e(Point(4, 17), Point(22, 34), 2);

    for (unsigned i = 0; i < 100; ++i) {
        Coord t = g_random_double_range(0, 2*M_PI);
        Point p = e.pointAt(t);
        Coord t2 = e.timeAt(p);
        EXPECT_FLOAT_EQ(t, t2);
    }
}

TEST(EllipseTest, LineIntersection) {
    Ellipse e(Point(0, 0), Point(3, 2), 0);
    Line l(Point(0, -2), Point(1, 0));

    std::vector<ShapeIntersection> xs = e.intersect(l);

    ASSERT_EQ(xs.size(), 2);
    EXPECT_FLOAT_EQ(xs[0].point()[X], 0);
    EXPECT_FLOAT_EQ(xs[0].point()[Y], -2);
    EXPECT_FLOAT_EQ(xs[1].point()[X], 9./5);
    EXPECT_FLOAT_EQ(xs[1].point()[Y], 8./5);

    // due to numeric imprecision when evaluating Ellipse,
    // the points may deviate by around 2e-16
    Point ep = e.pointAt(xs[0].first);
    Point lp = l.pointAt(xs[0].second);
    EXPECT_NEAR(ep[X], lp[X], 1e-15);
    EXPECT_NEAR(ep[Y], lp[Y], 1e-15);
}

TEST(EllipseTest, EllipseIntersection) {
    Ellipse e1(Point(5, 5), Point(15, 10), 0.1);
    Ellipse e2(Point(5, 15), Point(7, 10), -0.1);

    e1.intersect(e2);
}
