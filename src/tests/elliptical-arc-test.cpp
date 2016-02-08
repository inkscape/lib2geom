/** @file
 * @brief Unit tests for EllipticalArc.
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
#include <2geom/elliptical-arc.h>
#include <glib.h>

using namespace Geom;

TEST(EllipticalArcTest, PointAt) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, false, true, Point(-40,0));
    EXPECT_near(a.pointAt(0), a.initialPoint(), 1e-14);
    EXPECT_near(a.pointAt(1), a.finalPoint(), 1e-14);
    EXPECT_near(a.pointAt(0.5), Point(-20,10), 1e-14);

    EllipticalArc b(Point(0,0), Point(10,20), 0, false, true, Point(-40,0));
    EXPECT_near(b.pointAt(0), b.initialPoint(), 1e-14);
    EXPECT_near(b.pointAt(1), b.finalPoint(), 1e-14);
    EXPECT_near(b.pointAt(0.5), Point(-20,40), 1e-14);

    EllipticalArc c(Point(200,0), Point(40,20), Angle::from_degrees(90), false, false, Point(200,100));
    EXPECT_near(c.pointAt(0), c.initialPoint(), 1e-13);
    EXPECT_near(c.pointAt(1), c.finalPoint(), 1e-13);
    EXPECT_near(c.pointAt(0.5), Point(175, 50), 1e-13);
}

TEST(EllipticalArc, Transform) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, false, true, Point(-40,0));
    EllipticalArc b(Point(-40,0), Point(10,20), M_PI/2, false, true, Point(0,0));
    EllipticalArc c = a;
    Affine m = Rotate::around(Point(-20,0), M_PI);
    c.transform(m);

    for (unsigned i = 0; i <= 100; ++i) {
        Coord t = i/100.;
        EXPECT_near(c.pointAt(t), b.pointAt(t), 1e-12);
        EXPECT_near(a.pointAt(t)*m, c.pointAt(t), 1e-12);
    }
}

TEST(EllipticalArcTest, Duplicate) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, true, false, Point(-40,0));
    EllipticalArc *b = static_cast<EllipticalArc*>(a.duplicate());
    EXPECT_EQ(a, *b);
    delete b;
}

TEST(EllipticalArcTest, LineSegmentIntersection) {
    std::vector<CurveIntersection> r1;
    EllipticalArc a3(Point(0,0), Point(5,1.5), 0, true, true, Point(0,2));
    LineSegment ls(Point(0,5), Point(7,-3));
    r1 = a3.intersect(ls);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a3, ls, r1, 1e-10);
}

TEST(EllipticalArcTest, ArcIntersection) {
    std::vector<CurveIntersection> r1, r2;

    EllipticalArc a1(Point(0,0), Point(6,3), 0.1, false, false, Point(10,0));
    EllipticalArc a2(Point(0,2), Point(6,3), -0.1, false, true, Point(10,2));
    r1 = a1.intersect(a2);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a1, a2, r1, 1e-10);

    EllipticalArc a3(Point(0,0), Point(5,1.5), 0, true, true, Point(0,2));
    EllipticalArc a4(Point(3,5), Point(5,1.5), M_PI/2, true, true, Point(5,0));
    r2 = a3.intersect(a4);
    EXPECT_EQ(r2.size(), 3u);
    EXPECT_intersections_valid(a3, a4, r2, 1e-10);
}

TEST(EllipticalArcTest, BezierIntersection) {
    std::vector<CurveIntersection> r1, r2;

    EllipticalArc a3(Point(0,0), Point(1.5,5), M_PI/2, true, true, Point(0,2));
    CubicBezier bez1(Point(0,3), Point(7,3), Point(0,-1), Point(7,-1));
    r1 = a3.intersect(bez1);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a3, bez1, r1, 1e-10);

    EllipticalArc a4(Point(3,5), Point(5,1.5), 3*M_PI/2, true, true, Point(5,5));
    CubicBezier bez2(Point(0,5), Point(10,-4), Point(10,5), Point(0,-4));
    r2 = a4.intersect(bez2);
    EXPECT_EQ(r2.size(), 4u);
    EXPECT_intersections_valid(a4, bez2, r2, 1e-10);
}
