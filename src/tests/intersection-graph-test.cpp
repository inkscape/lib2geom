/** @file
 * @brief Unit tests for PathIntersectionGraph, aka Boolean operations.
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

#include <2geom/intersection-graph.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>
#include <glib.h>

using namespace std;
using namespace Geom;

Path string_to_path(const char* s) {
    PathVector pv = parse_svg_path(s);
    assert(pv.size() == 1u);
    return pv[0];
}

enum Operation {
    UNION,
    INTERSECTION,
    XOR,
    A_MINUS_B,
    B_MINUS_A
};

class IntersectionGraphTest : public ::testing::Test {
protected:
    IntersectionGraphTest() {
        rectangle = string_to_path("M 0,0 L 5,0 5,8 0,8 Z");
        bigrect = string_to_path("M -3,-4 L 7,-4 7,12 -3,12 Z");
        bigh = string_to_path("M 2,-3 L 3,-2 1,2 3,4 4,2 6,3 2,11 0,10 2,5 1,4 -1,6 -2,5 Z");
        smallrect = string_to_path("M 7,4 L 9,4 9,7 7,7 Z");
        g_random_set_seed(2345);
    }

    void checkRandomPoints(PathVector const &a, PathVector const &b, PathVector const &result,
                           Operation op, unsigned npts = 5000)
    {
        Rect bounds = *(a.boundsFast() | b.boundsFast());
        for (unsigned i = 0; i < npts; ++i) {
            Point p;
            p[X] = g_random_double_range(bounds[X].min(), bounds[X].max());
            p[Y] = g_random_double_range(bounds[Y].min(), bounds[Y].max());
            bool in_a = a.winding(p) % 2;
            bool in_b = b.winding(p) % 2;
            bool in_res = result.winding(p) % 2;

            switch (op) {
            case UNION:
                EXPECT_EQ(in_res, in_a || in_b);
                break;
            case INTERSECTION:
                EXPECT_EQ(in_res, in_a && in_b);
                break;
            case XOR:
                EXPECT_EQ(in_res, in_a ^ in_b);
                break;
            case A_MINUS_B:
                EXPECT_EQ(in_res, in_a && !in_b);
                break;
            case B_MINUS_A:
                EXPECT_EQ(in_res, !in_a && in_b);
                break;
            }
        }
    }

    Path rectangle, bigrect, bigh, smallrect;
};

TEST_F(IntersectionGraphTest, Union) {
    PathIntersectionGraph graph(rectangle, bigh);
    //std::cout << graph << std::endl;
    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1u);
    EXPECT_EQ(r.curveCount(), 19u);

    checkRandomPoints(rectangle, bigh, r, UNION);

    /*SVGPathWriter wr;
    wr.feed(r);
    std::cout << wr.str() << std::endl;*/
}

TEST_F(IntersectionGraphTest, DisjointUnion) {
    PathIntersectionGraph graph(rectangle, smallrect);

    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 2u);
    checkRandomPoints(rectangle, smallrect, r, UNION);
}

TEST_F(IntersectionGraphTest, CoverUnion) {
    PathIntersectionGraph graph(bigrect, bigh);
    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1u);
    EXPECT_EQ(r, bigrect);
}

TEST_F(IntersectionGraphTest, Subtraction) {
    PathIntersectionGraph graph(rectangle, bigh);
    PathVector a = graph.getAminusB();
    EXPECT_EQ(a.size(), 4u);
    EXPECT_EQ(a.curveCount(), 17u);
    checkRandomPoints(rectangle, bigh, a, A_MINUS_B);

    PathVector b = graph.getBminusA();
    EXPECT_EQ(b.size(), 4u);
    EXPECT_EQ(b.curveCount(), 15u);
    checkRandomPoints(rectangle, bigh, b, B_MINUS_A);

    PathVector x = graph.getXOR();
    EXPECT_EQ(x.size(), 8u);
    EXPECT_EQ(x.curveCount(), 32u);
    checkRandomPoints(rectangle, bigh, x, XOR);
}

TEST_F(IntersectionGraphTest, PointOnEdge) {
    PathVector a = string_to_path("M 0,0 L 10,0 10,10 0,10 z");
    PathVector b = string_to_path("M -5,2 L 0,2 5,5 0,8 -5,8 z");

    PathIntersectionGraph graph(a, b);
    PathVector u = graph.getUnion();
    //std::cout << u << std::endl;
    EXPECT_EQ(u.size(), 1u);
    EXPECT_EQ(u.curveCount(), 8u);
    checkRandomPoints(a, b, u, UNION);

    PathVector i = graph.getIntersection();
    //std::cout << i << std::endl;
    EXPECT_EQ(i.size(), 1u);
    EXPECT_EQ(i.curveCount(), 3u);
    checkRandomPoints(a, b, i, INTERSECTION);

    PathVector s1 = graph.getAminusB();
    //std::cout << s1 << std::endl;
    EXPECT_EQ(s1.size(), 1u);
    EXPECT_EQ(s1.curveCount(), 7u);
    checkRandomPoints(a, b, s1, A_MINUS_B);

    PathVector s2 = graph.getBminusA();
    //std::cout << s2 << std::endl;
    EXPECT_EQ(s2.size(), 1u);
    EXPECT_EQ(s2.curveCount(), 4u);
    checkRandomPoints(a, b, s2, B_MINUS_A);

    PathVector x = graph.getXOR();
    //std::cout << x << std::endl;
    EXPECT_EQ(x.size(), 2u);
    EXPECT_EQ(x.curveCount(), 11u);
    checkRandomPoints(a, b, x, XOR);
}

TEST_F(IntersectionGraphTest, RhombusInSquare) {
    PathVector square = string_to_path("M 0,0 L 10,0 10,10 0,10 z");
    PathVector rhombus = string_to_path("M 5,0 L 10,5 5,10 0,5 z");

    PathIntersectionGraph graph(square, rhombus);
    //std::cout << graph << std::endl;
    PathVector u = graph.getUnion();
    EXPECT_EQ(u.size(), 1u);
    EXPECT_EQ(u.curveCount(), 4u);
    checkRandomPoints(square, rhombus, u, UNION);

    PathVector i = graph.getIntersection();
    EXPECT_EQ(i.size(), 1u);
    EXPECT_EQ(i.curveCount(), 4u);
    checkRandomPoints(square, rhombus, i, INTERSECTION);

    PathVector s1 = graph.getAminusB();
    EXPECT_EQ(s1.size(), 2u);
    EXPECT_EQ(s1.curveCount(), 8u);
    checkRandomPoints(square, rhombus, s1, A_MINUS_B);

    PathVector s2 = graph.getBminusA();
    EXPECT_EQ(s2.size(), 0u);
    EXPECT_EQ(s2.curveCount(), 0u);
    checkRandomPoints(square, rhombus, s2, B_MINUS_A);
}

// this test is disabled, since we cannot handle overlapping segments for now.
#if 0
TEST_F(IntersectionGraphTest, EqualUnionAndIntersection) {
    PathVector shape = string_to_path("M 0,0 L 2,1 -1,2 -1,3 0,3 z");
    PathIntersectionGraph graph(shape, shape);
    std::cout << graph << std::endl;
    PathVector a = graph.getUnion();
    std::cout << shape << std::endl;
    std::cout << a << std::endl;
    checkRandomPoints(shape, shape, a, UNION);

    PathIntersectionGraph graph2(bigh, bigh);
    PathVector b = graph2.getIntersection();
    checkRandomPoints(bigh, bigh, b, INTERSECTION);
    std::cout << b <<std::endl;
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
