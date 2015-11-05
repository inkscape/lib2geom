/** @file
 * @brief Unit tests for the binomial coefficient function.
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
#include <2geom/choose.h>
#include <glib.h>

using namespace Geom;

TEST(ChooseTest, PascalsTriangle) {
    // check whether the values match Pascal's triangle
    for (unsigned i = 0; i < 500; ++i) {
        int n = g_random_int_range(3, 100);
        int k = g_random_int_range(1, n-1);

        double a = choose<double>(n, k);
        double b = choose<double>(n-1, k);
        double c = choose<double>(n-1, k-1);

        EXPECT_EQ(a, b + c);
    }
}

TEST(ChooseTest, Values) {
    // test some well-known values
    EXPECT_EQ(choose<double>(0, 0), 1);
    EXPECT_EQ(choose<double>(1, 0), 1);
    EXPECT_EQ(choose<double>(1, 1), 1);
    EXPECT_EQ(choose<double>(127, 127), 1);
    EXPECT_EQ(choose<double>(92, 0), 1);
    EXPECT_EQ(choose<double>(2, 1), 2);

    // number of possible flops in Texas Hold 'Em Poker
    EXPECT_EQ(choose<double>(50,  3), 19600.);
    EXPECT_EQ(choose<double>(50, 47), 19600.);
    // number of possible hands in bridge
    EXPECT_EQ(choose<double>(52, 13), 635013559600.);
    EXPECT_EQ(choose<double>(52, 39), 635013559600.);
    // number of possible Lotto results
    EXPECT_EQ(choose<double>(49,  6), 13983816.);
    EXPECT_EQ(choose<double>(49, 43), 13983816.);
}
