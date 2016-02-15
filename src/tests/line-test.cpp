/** @file
 * @brief Unit tests for Line and related functions
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
#include <iostream>
#include <glib.h>

#include <2geom/line.h>
#include <2geom/affine.h>

using namespace Geom;

TEST(LineTest, VectorAndVersor) {
    Line a(Point(10, 10), Point(-10, 20));
    Line b(Point(10, 10), Point(15, 15));
    EXPECT_EQ(a.vector(), Point(-20, 10));
    EXPECT_EQ(b.vector(), Point(5, 5));
    EXPECT_EQ(a.versor(), a.vector().normalized());
    EXPECT_EQ(b.versor(), b.vector().normalized());
}

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

TEST(LineTest, RotationToZero) {
    Line a(Point(-5,23), Point(15,27));
    Affine mx = a.rotationToZero(X);
    Affine my = a.rotationToZero(Y);

    for (unsigned i = 0; i <= 12; ++i) {
        double t = -1 + 0.25 * i;
        Point p = a.pointAt(t);
        Point rx = p * mx;
        Point ry = p * my;
        //std::cout << rx[X] << " " << ry[Y] << std::endl;
        // unfortunately this is precise only to about 1e-14
        EXPECT_NEAR(rx[X], 0, 1e-14);
        EXPECT_NEAR(ry[Y], 0, 1e-14);
    }
}

TEST(LineTest, Coefficients) {
    std::vector<Line> lines;
    lines.push_back(Line(Point(1e9,1e9), Point(1,1)));
    //the case below will never work without normalizing the line
    //lines.push_back(Line(Point(1e5,1e5), Point(1e-15,0)));
    lines.push_back(Line(Point(1e5,1e5), Point(1e5,-1e5)));
    lines.push_back(Line(Point(-3,10), Point(3,10)));
    lines.push_back(Line(Point(250,333), Point(-72,121)));

    for (unsigned i = 0; i < lines.size(); ++i) {
        Coord a, b, c, A, B, C;
        lines[i].coefficients(a, b, c);
        /*std::cout << format_coord_nice(a) << " "
                  << format_coord_nice(b) << " "
                  << format_coord_nice(c) << std::endl;*/
        Line k(a, b, c);
        //std::cout << k.initialPoint() << " " << k.finalPoint() << std::endl;
        k.coefficients(A, B, C);
        /*std::cout << format_coord_nice(A) << " "
                  << format_coord_nice(B) << " "
                  << format_coord_nice(C) << std::endl;*/
        EXPECT_DOUBLE_EQ(a, A);
        EXPECT_DOUBLE_EQ(b, B);
        EXPECT_DOUBLE_EQ(c, C);

        for (unsigned j = 0; j <= 10; ++j) {
            double t = j / 10.;
            Point p = lines[i].pointAt(t);
            /*std::cout << t << " " << p << " "
                      << A*p[X] + B*p[Y] + C << " "
                      << A*(p[X]-1) + B*(p[Y]+1) + C << std::endl;*/
            EXPECT_near(A*p[X] + B*p[Y] + C, 0., 2e-11);
            EXPECT_not_near(A*(p[X]-1) + B*(p[Y]+1) + C, 0., 1e-6);
        }
    }
}

TEST(LineTest, Intersection) {
    Line a(Point(0,3), Point(1,2));
    Line b(Point(0,-3), Point(1,-2));
    LineSegment lsa(Point(0,3), Point(1,2));
    LineSegment lsb(Point(0,-3), Point(1,-2));
    LineSegment lsc(Point(3,1), Point(3, -1));

    std::vector<ShapeIntersection> r1, r2, r3;

    r1 = a.intersect(b);
    ASSERT_EQ(r1.size(), 1u);
    EXPECT_EQ(r1[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, b, r1, 1e-15);

    r2 = a.intersect(lsc);
    ASSERT_EQ(r2.size(), 1u);
    EXPECT_EQ(r2[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, lsc, r2, 1e-15);

    r3 = b.intersect(lsc);
    ASSERT_EQ(r3.size(), 1u);
    EXPECT_EQ(r3[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, lsc, r3, 1e-15);

    EXPECT_TRUE(lsa.intersect(lsb).empty());
    EXPECT_TRUE(lsa.intersect(lsc).empty());
    EXPECT_TRUE(lsb.intersect(lsc).empty());
    EXPECT_TRUE(a.intersect(lsb).empty());
    EXPECT_TRUE(b.intersect(lsa).empty());
}
