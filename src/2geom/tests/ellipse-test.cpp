#include "testing.h"
#include <iostream>

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
