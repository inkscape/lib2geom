/** @file
 * @brief Unit tests for Point, IntPoint and related functions.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014-2015 Authors
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

#include <gtest/gtest.h>
#include <2geom/point.h>

namespace Geom {

TEST(PointTest, Normalize) {
    Point a(1e-18, 0);
    Point b = a;
    a.normalize();

    EXPECT_EQ(a, Point(1, 0));
    EXPECT_EQ(b.normalized(), a);
    EXPECT_NE(b, a);
}

TEST(PointTest, ScalarOps) {
    Point a(1,2);
    EXPECT_EQ(a * 2, Point(2, 4));
    EXPECT_EQ(2 * a, Point(2, 4));
    EXPECT_EQ(a / 2, Point(0.5, 1));

    Point b = a;
    a *= 2;
    a /= 2;
    EXPECT_EQ(a, b);
}

TEST(PointTest, Rounding) {
    Point a(-0.7, 0.7);
    IntPoint aceil(0, 1), afloor(-1, 0), around(-1, 1);
    EXPECT_TRUE(a.ceil() == aceil);
    EXPECT_TRUE(a.floor() == afloor);
    EXPECT_TRUE(a.round() == around);
}

} // end namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
