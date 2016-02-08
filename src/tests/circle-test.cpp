/** @file
 * @brief Unit tests for Circle and related functions.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include "testing.h"
#include <2geom/circle.h>
#include <2geom/line.h>

using namespace Geom;

TEST(CircleTest, Equality) {
    Circle a(4, 5, 6);
    Circle b(Point(4, 5), 6);
    Circle c(4.00000001, 5, 6);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(b, c);
}

TEST(CircleTest, Nearness) {
    Circle a(4, 5, 6);
    Circle b(4.000007, 5, 6);
    Circle c(4, 5, 6.000007);
    Circle d(4.000007, 5, 6.000007);
    Circle e(4, 5, 7);

    EXPECT_TRUE(are_near(a, b, 1e-5));
    EXPECT_TRUE(are_near(a, c, 1e-5));
    EXPECT_TRUE(are_near(c, d, 1e-5));
    EXPECT_FALSE(are_near(a, d, 1e-5));
    EXPECT_FALSE(are_near(a, e, 1e-2));
    EXPECT_FALSE(are_near(b, e, 1e-2));
    EXPECT_FALSE(are_near(c, e, 1e-2));
}

TEST(CircleTest, UnitCircleTransform) {
    Circle c(17, 23, 22);

    Point q = c.pointAt(M_PI/2);
    Point p = Point(0, 1) * c.unitCircleTransform();
    Point r = q * c.inverseUnitCircleTransform();

    EXPECT_FLOAT_EQ(p[X], q[X]);
    EXPECT_FLOAT_EQ(p[Y], q[Y]);
    EXPECT_FLOAT_EQ(r[X], 0);
    EXPECT_FLOAT_EQ(r[Y], 1);
}

TEST(CircleTest, Coefficients) {
    Circle circ(5, 12, 87), circ2;

    Coord a, b, c, d;
    circ.coefficients(a, b, c, d);
    circ2.setCoefficients(a, b, c, d);

    EXPECT_TRUE(are_near(circ, circ2, 1e-15));

    for (unsigned i = 0; i < 100; ++i) {
        Coord t = -5 + 0.111 * i;
        Point p = circ.pointAt(t);
        Coord eqres = a * p[X]*p[X] + a*p[Y]*p[Y] + b*p[X] + c*p[Y] + d;
        EXPECT_NEAR(eqres, 0, 1e-11);
    }
}

TEST(CircleTest, CircleIntersection) {
    Circle a(5, 5, 5), b(15, 5, 5), c(10, 10, 6), d(-5, 5, 2);
    std::vector<ShapeIntersection> r1, r2, r3;

    r1 = a.intersect(b);
    ASSERT_EQ(r1.size(), 1u);
    EXPECT_EQ(r1[0].point(), Point(10,5));
    EXPECT_intersections_valid(a, b, r1, 1e-15);

    r2 = a.intersect(c);
    EXPECT_EQ(r2.size(), 2u);
    EXPECT_intersections_valid(a, c, r2, 1e-15);

    r3 = b.intersect(c);
    EXPECT_EQ(r3.size(), 2u);
    EXPECT_intersections_valid(b, c, r3, 3e-15);

    EXPECT_TRUE(a.intersect(d).empty());
    EXPECT_TRUE(b.intersect(d).empty());
    EXPECT_TRUE(c.intersect(d).empty());
}

TEST(CircleTest, LineIntersection) {
    Circle c(5, 5, 10);
    Line l1(Point(-5, -20), Point(-5, 20));
    Line l2(Point(0, 0), Point(10, 2.3));
    Line l3(Point(20, -20), Point(0, -20));

    EXPECT_TRUE(c.intersects(l1));
    EXPECT_TRUE(c.intersects(l2));
    EXPECT_FALSE(c.intersects(l3));

    std::vector<ShapeIntersection> r1, r2, r3;

    r1 = c.intersect(l1);
    ASSERT_EQ(r1.size(), 1u);
    EXPECT_EQ(r1[0].point(), Point(-5, 5));
    EXPECT_intersections_valid(c, l1, r1, 1e-15);

    r2 = c.intersect(l2);
    EXPECT_EQ(r2.size(), 2u);
    EXPECT_intersections_valid(c, l2, r2, 1e-14);

    r3 = c.intersect(l3);
    EXPECT_TRUE(r3.empty());
}
