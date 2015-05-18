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

using namespace std;
using namespace Geom;

Path string_to_path(const char* s) {
    PathVector pv = parse_svg_path(s);
    assert(pv.size() == 1);
    return pv[0];
}

class IntersectionGraphTest : public ::testing::Test {
protected:
    IntersectionGraphTest() {
        rectangle = string_to_path("M 0,0 L 5,0 5,8 0,8 Z");
        bigrect = string_to_path("M -3,-4 L 7,-4 7,12 -3,12 Z");
        bigh = string_to_path("M 2,-3 L 3,-2 1,2 3,4 4,2 6,3 2,11 0,10 2,5 1,4 -1,6 -2,5 Z");
        smallrect = string_to_path("M 7,4 L 9,4 9,7 7,7 Z");
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    Path rectangle, bigrect, bigh, smallrect;
};

TEST_F(IntersectionGraphTest, Union) {
    PathIntersectionGraph graph(rectangle, bigh);

    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1);
    EXPECT_EQ(r.curveCount(), 19);

    /*SVGPathWriter wr;
    wr.feed(r);
    std::cout << wr.str() << std::endl;*/
}

TEST_F(IntersectionGraphTest, DisjointUnion) {
    PathIntersectionGraph graph(rectangle, smallrect);

    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 2);
}

TEST_F(IntersectionGraphTest, CoverUnion) {
    PathIntersectionGraph graph(bigrect, bigh);
    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1);
    EXPECT_EQ(r, bigrect);
}

TEST_F(IntersectionGraphTest, Subtraction) {
    PathIntersectionGraph graph(rectangle, bigh);
    PathVector a = graph.getAminusB();
    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(a.curveCount(), 17);

    PathVector b = graph.getBminusA();
    EXPECT_EQ(b.size(), 4);
    EXPECT_EQ(b.curveCount(), 15);

    PathVector x = graph.getXOR();
    EXPECT_EQ(x.size(), 8);
    EXPECT_EQ(x.curveCount(), 32);
}

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
